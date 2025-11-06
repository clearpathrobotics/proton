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

Node::Node(const std::string config_file, const std::string name, bool auto_configure, bool auto_activate)
    : config_(config_file), name_(name), state_(NodeState::UNCONFIGURED), rx_(0), tx_(0), rx_kbps_(0.0), tx_kbps_(0.0)
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
  if (state_ != NodeState::UNCONFIGURED)
  {
    return Status::INVALID_STATE_TRANSITION;
  }

  for (auto b : config_.getBundles()) {
    addBundle(b);
  }

  // Get config for this node
  node_config_ = config_.getNodes().at(name_);

  // Find all peers
  for (auto& [name, peer_config]: config_.getNodes()) {
    if (name != name_) {
      peers_.emplace(
        name,
        Peer(
          node_config_,
          peer_config,
          std::bind(&Node::readCompleteCallback, this, std::placeholders::_1, std::placeholders::_2)
        )
      );
    }
  }

  if (node_config_.heartbeat.enabled)
  {
    // Add a heartbeat bundle to send to each peer
    for (auto& [peer_name, peer] : peers_)
    {
      addHeartbeat(name_, peer_name);
    }
  }

  for (auto& [peer_name, peer] : peers_)
  {
    // Add a heartbeat bundle to receive from each peer
    if (peer.getConfig().heartbeat.enabled)
    {
      addHeartbeat(peer_name, name_);
    }
  }

  std::cout << "Configured" << std::endl;

  state_ = NodeState::INACTIVE;
  return Status::OK;
}

Status Node::activate()
{
  if (state_ != NodeState::INACTIVE)
  {
    return Status::INVALID_STATE_TRANSITION;
  }

  for (auto& [name, peer]: peers_)
  {
    peer.run();
  }

  // Start heartbeat thread
  if (node_config_.heartbeat.enabled)
  {
    heartbeat_thread_ = std::thread(&Node::runHeartbeatThread, this);
  }

  std::cout << "Activated" << std::endl;
  state_ = NodeState::ACTIVE;
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
  auto& peer = peers_.at(bundle_handle.getConsumer());

  if (!peer.connected()) {
    return Status::INVALID_STATE;
  }

  auto bundle = bundle_handle.getBundlePtr().get();

  auto buf = std::make_unique<uint8_t[]>(bundle->ByteSizeLong());

  if (bundle->SerializeToArray(buf.get(), bundle->ByteSizeLong()))
  {
    size_t bytes_written = 0;

    Status status = peer.write(buf.get(), bundle->ByteSizeLong(), bytes_written);

    if (status == Status::OK)
    {
      tx_ += bytes_written;

      if (peer.getConfig().transport.type == transport_types::SERIAL)
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
  if (!node_config_.heartbeat.enabled)
  {
    return Status::ERROR;
  }

  auto& heartbeat = getHeartbeat(name_);
  auto& signal = heartbeat.getSignal("heartbeat");
  // Increment heartbeat
  signal.setValue<uint32_t>(signal.getValue<uint32_t>() + 1);
  return sendBundle(heartbeat);
}

Status Node::registerCallback(const std::string &bundle_name, BundleHandle::BundleCallback callback)
{
  auto& bundle = getBundle(bundle_name);

  if (callback && bundle.getConsumer() == name_)
  {
    bundle.registerCallback(callback);
    return Status::OK;
  }

  return Status::ERROR;
}

Status Node::registerHeartbeatCallback(const std::string &producer, BundleHandle::BundleCallback callback)
{
  auto& bundle = getHeartbeat(producer);

  if (callback && bundle.getConsumer() == name_)
  {
    bundle.registerCallback(callback);
    return Status::OK;
  }

  return Status::ERROR;
}

Status Node::readCompleteCallback(Bundle& bundle, const std::string& producer)
{
  read_queue_.push({bundle, producer});
  return Status::OK;
}

Status Node::waitForBundle()
{
  auto received_bundle = read_queue_.pop();

  auto& handle = setBundle(received_bundle.bundle, received_bundle.producer);
  handle.incrementRxCount();
  rx_ += received_bundle.bundle.ByteSizeLong();

  if (node_config_.transport.type == transport_types::SERIAL)
  {
    rx_ += SerialTransport::FRAME_OVERHEAD;
  }

  auto callback = handle.getCallback();

  if (callback)
  {
    callback(handle);
  }

  return Status::OK;
}

Status Node::spinOnce() {
  if (state_ != NodeState::ACTIVE) {
    return Status::INVALID_STATE;
  }

  return waitForBundle();

  // switch (transport_->state())
  // {
  //   case TransportState::DISCONNECTED:
  //   {
  //     transport_->connect();
  //     break;
  //   }

  //   case TransportState::ERROR:
  //   {
  //     transport_->disconnect();
  //     // Reset heartbeat count
  //     getHeartbeat(name_).getSignal("heartbeat").setValue<uint32_t>(0);
  //     break;
  //   }

  //   case TransportState::CONNECTED:
  //   {
  //     return pollForBundle();
  //   }
  // }
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
    std::this_thread::sleep_for(std::chrono::milliseconds(node_config_.heartbeat.period));
  }
}

void Node::printStats()
{
  std::cout<< u8"\033[2J\033[1;1H";
  std::cout << "-------- Proton CPP Node --------" << std::endl;
  std::cout << "Config: " << config_.getName() << std::endl;
  std::cout << "Node: " << name_ << std::endl;
  std::cout << "Peers: ";
  for (auto& [name, peer]: peers_)
  {
    std::cout << name + " ";
  }
  std::cout << std::endl;
  std::cout << "Rx: " << getRxKbps() << " KB/s " << "Tx: " << getTxKbps() << " KB/s" << std::endl;
  std::cout << "----- Produced Bundles (hz) -----" << std::endl;
  for (auto& [name, handle] : bundles_)
  {
    if (handle.getProducer() == name_)
    {
      std::cout << name << ": " << handle.getTxps() << std::endl;
    }
  }
  std::cout << "----- Consumed Bundles (hz) -----" << std::endl;
  for (auto& [name, handle] : bundles_)
  {
    if (handle.getConsumer() == name_)
    {
      std::cout << name << ": " << handle.getRxps() << std::endl;
    }
  }
  std::cout << "----- Heartbeats (hz) -----" << std::endl;
  for (auto& [name, handle] : heartbeat_bundles_)
  {
    if (handle.getConsumer() == name_)
    {
      std::cout << name << ": " << handle.getRxps() << std::endl;
    }
  }

  std::cout << "---------------------------------" << std::endl;
}

Peer::Peer(const NodeConfig& node_config, const NodeConfig& peer_config, ReadCompleteCallback callback):
 config_(peer_config), callback_(callback)
{
  auto transport_type = peer_config.transport.type;
  if (transport_type == transport_types::UDP4) {
    auto node_endpoint =
        proton::socket_endpoint(node_config.transport.ip,
                                node_config.transport.port);
    auto peer_endpoint =
        proton::socket_endpoint(peer_config.transport.ip,
                                peer_config.transport.port);
    setTransport(
        std::make_unique<Udp4Transport>(node_endpoint, peer_endpoint));
    std::cout << "Created transport " << transport_->state() << std::endl;
  }
  else if (transport_type == transport_types::SERIAL) {
    auto device = proton::serial_device(node_config.transport.device, 0);
    setTransport(std::make_unique<SerialTransport>(device));
  }
}

void Peer::run()
{
  run_thread_ = std::thread(&Peer::spin, this);
}

void Peer::spin()
{
  Status status;
  std::cout << "Peer state " << transport_->state() << std::endl;
  while(1)
  {
    switch(transport_->state())
    {
      case TransportState::DISCONNECTED:
      {
        std::cout << "Connecting" << std::endl;
        status = connect();
        if (status != Status::OK)
        {
          std::cout << "Failed to connect to peer " << config_.name << std::endl;
        }
        break;
      }

      case TransportState::CONNECTED:
      {
        status = pollForBundle();
        if (status != Status::OK)
        {
          std::cout << "Failed to poll for bundle on peer " << config_.name << std::endl;
        }
        break;
      }

      case TransportState::ERR:
      {
        status = disconnect();
        if (status != Status::OK)
        {
          std::cout << "Failed to disconnect from peer " << config_.name << std::endl;
        }
      }
    }
  }
}

Status Peer::pollForBundle()
{
  uint8_t read_buf[PROTON_MAX_MESSAGE_SIZE];
  size_t bytes_read = 0;
  Status status = read(read_buf, PROTON_MAX_MESSAGE_SIZE, bytes_read);

  if (status == Status::OK) {
    if (callback_ == nullptr)
    {
      return Status::NULL_PTR;
    }

    // Parse bundle
    auto bundle = Bundle();
    bundle.ParseFromArray(read_buf, bytes_read);

    return callback_(bundle, config_.name);
  }

  std::cerr << "Read error " << status << std::endl;
  return status;
}