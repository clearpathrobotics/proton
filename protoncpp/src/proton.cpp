/**
 * Software License Agreement (proprietary)
 *
 * @copyright Copyright (c) 2025 Clearpath Robotics, Inc., All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, is not permitted without the express permission of Clearpath
 * Robotics.
 *
 * @author Roni Kreinin (rkreinin@clearpathrobotics.com)
 */

#include "protoncpp/proton.hpp"
#include "protoncpp/config.hpp"
#include <iostream>
#include <chrono>


using namespace proton;

Node::Node() {}

Node::Node(const std::string config_file, const std::string target, bool auto_configure, bool auto_activate)
    : config_(config_file), target_(target), state_(State::UNCONFIGURED), rx_(0), tx_(0), rx_kbps_(0.0), tx_kbps_(0.0)
{
  Status status;

  if (!auto_configure && auto_activate)
  {
    throw std::runtime_error("Auto activate cannot be enabled without auto configure");
  }

  if (auto_configure)
  {
    status = configure();

    if (status != Status::OK)
    {
      throw std::runtime_error("Configuration error: " + std::to_string(status));
    }

    if (auto_activate)
    {
      status = activate();

      if (status != Status::OK)
      {
        throw std::runtime_error("Activation error: " + std::to_string(status));
      }
    }
  }
}

Status Node::configure()
{
  if (state_ != State::UNCONFIGURED)
  {
    return Status::INVALID_STATE_TRANSITION;
  }

  for (auto b : config_.getBundles()) {
    addBundle(b);
  }

  NodeConfig target_config, peer_config;

  for (auto n : config_.getNodes()) {
    // Target node config
    if (n.name == target_) {
      target_config = n;
    }
    // Peer node config
    else {
      peer_config = n;
      peer_ = peer_config.name;
    }
  }

  target_node_config_ = target_config;

  auto transport_type = target_config.transport.type;
  if (transport_type == transport_types::UDP4) {
    auto target_endpoint =
        proton::socket_endpoint(target_config.transport.ip,
                                target_config.transport.port);
    auto peer_endpoint =
        proton::socket_endpoint(peer_config.transport.ip,
                                peer_config.transport.port);
    setTransport(
        std::make_unique<Udp4Transport>(target_endpoint, peer_endpoint));
  }
  else if (transport_type == transport_types::SERIAL) {
    auto device = proton::serial_device(target_config.transport.device, 0);
    setTransport(std::make_unique<SerialTransport>(device));
  }

  if (target_config.heartbeat.enabled)
  {
    addHeartbeat(target_, peer_);
  }

  if (peer_config.heartbeat.enabled)
  {
    addHeartbeat(peer_, target_);
  }

  state_ = State::INACTIVE;
  return Status::OK;
}

Status Node::activate()
{
  if (state_ != State::INACTIVE)
  {
    return Status::INVALID_STATE_TRANSITION;
  }

  Status status = connect();

  if (status != Status::OK)
  {
    return status;
  }

  // Start heartbeat thread
  if (target_node_config_.heartbeat.enabled)
  {
    heartbeat_thread_ = std::thread(&Node::runHeartbeatThread, this);
  }

  state_ = State::ACTIVE;
  return Status::OK;
}

void Node::startStatsThread()
{
  stats_thread_ = std::thread(&Node::runStatsThread, this);
}

Status Node::sendBundle(const std::string &bundle_name) {
  return sendBundle(getBundle(bundle_name));
}

Status Node::sendBundle(BundleHandle &bundle_handle) {
  if (!connected()) {
    return Status::INVALID_STATE;
  }

  auto bundle = bundle_handle.getBundlePtr().get();

  auto buf = std::make_unique<uint8_t[]>(bundle->ByteSizeLong());

  if (bundle->SerializeToArray(buf.get(), bundle->ByteSizeLong()))
  {
    size_t bytes_written = 0;

    Status status = write(buf.get(), bundle->ByteSizeLong(), bytes_written);

    if (status == Status::OK)
    {
      tx_ += bytes_written;

      if (target_node_config_.transport.type == transport_types::SERIAL)
      {
        tx_ += SerialTransport::FRAME_OVERHEAD;
      }

      bundle_handle.incrementTxCount();
    }

    return status;
  }

  return Status::SERIALIZATION_ERROR;
}

Status Node::sendHeartbeat()
{
  auto& heartbeat = getHeartbeat(target_);
  auto& signal = heartbeat.getSignal("heartbeat");
  // Increment heartbeat
  signal.setValue<uint32_t>(signal.getValue<uint32_t>() + 1);
  return sendBundle(heartbeat);
}

Status Node::registerCallback(const std::string &bundle_name, BundleHandle::BundleCallback callback)
{
  auto& bundle = getBundle(bundle_name);

  if (callback && bundle.getConsumer() == target_)
  {
    bundle.registerCallback(callback);
    return Status::OK;
  }

  return Status::ERROR;
}

Status Node::registerHeartbeatCallback(const std::string &producer, BundleHandle::BundleCallback callback)
{
  auto& bundle = getHeartbeat(producer);

  if (callback && bundle.getConsumer() == target_)
  {
    bundle.registerCallback(callback);
    return Status::OK;
  }

  return Status::ERROR;
}

Status Node::pollForBundle()
{
  uint8_t read_buf[PROTON_MAX_MESSAGE_SIZE];
  size_t bytes_read = 0;
  Status status = read(read_buf, PROTON_MAX_MESSAGE_SIZE, bytes_read);

  if (status == Status::OK) {
    auto& bundle = receiveBundle(read_buf, bytes_read, peer_);
    bundle.incrementRxCount();
    rx_ += bytes_read;

    if (target_node_config_.transport.type == transport_types::SERIAL)
    {
      rx_ += SerialTransport::FRAME_OVERHEAD;
    }

    auto callback = bundle.getCallback();
    if (callback)
    {
      callback(bundle);
    }
  }
  else
  {
    std::cerr << "Read error " << status << std::endl;
  }

  return status;
}

Status Node::spinOnce() {
  if (state_ != State::ACTIVE) {
    return Status::INVALID_STATE;
  }

  switch (transport_->state())
  {
    case Transport::State::DISCONNECTED:
    {
      transport_->connect();
      break;
    }

    case Transport::State::ERROR:
    {
      transport_->disconnect();
      // Reset heartbeat count
      getHeartbeat(target_).getSignal("heartbeat").setValue<uint32_t>(0);
      break;
    }

    case Transport::State::CONNECTED:
    {
      return pollForBundle();
    }
  }

  return Status::OK;
}

Status Node::spin() {
  Status status;
  while (1) {
    status = spinOnce();

    if (status != Status::OK)
    {
      return status;
    }
  }
}

void Node::runStatsThread()
{
  while(1)
  {
    rx_kbps_ = static_cast<double>(rx_) / 1000;
    tx_kbps_ = static_cast<double>(tx_) / 1000;
    rx_ = 0;
    tx_ = 0;

    if (bundles_.size() > 0)
    {
      for (auto& [name, handle] : bundles_)
      {
        handle.setRxps(handle.getRxCount());
        handle.setTxps(handle.getTxCount());
        handle.resetRxCount();
        handle.resetTxCount();
      }

      for (auto& [name, handle] : heartbeat_bundles_)
      {
        handle.setRxps(handle.getRxCount());
        handle.setTxps(handle.getTxCount());
        handle.resetRxCount();
        handle.resetTxCount();
      }
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

void Node::runHeartbeatThread()
{
  while(1)
  {
    sendHeartbeat();
    std::this_thread::sleep_for(std::chrono::milliseconds(target_node_config_.heartbeat.period));
  }
}

void Node::printStats()
{
  std::cout<< u8"\033[2J\033[1;1H";
  std::cout << "-------- Proton CPP Node --------" << std::endl;
  std::cout << "Config: " << config_.getName() << std::endl;
  std::cout << "Target: " << target_ << std::endl;
  std::cout << "Rx: " << getRxKbps() << " KB/s " << "Tx: " << getTxKbps() << " KB/s" << std::endl;
  std::cout << "----- Produced Bundles (hz) -----" << std::endl;
  for (auto& [name, handle] : bundles_)
  {
    if (handle.getProducer() == target_)
    {
      std::cout << name << ": " << handle.getTxps() << std::endl;
    }
  }
  std::cout << "----- Consumed Bundles (hz) -----" << std::endl;
  for (auto& [name, handle] : bundles_)
  {
    if (handle.getConsumer() == target_)
    {
      std::cout << name << ": " << handle.getRxps() << std::endl;
    }
  }
  std::cout << "----- Heartbeats (hz) -----" << std::endl;
  for (auto& [name, handle] : heartbeat_bundles_)
  {
    if (handle.getConsumer() == target_)
    {
      std::cout << name << ": " << handle.getRxps() << std::endl;
    }
  }

  std::cout << "---------------------------------" << std::endl;
}
