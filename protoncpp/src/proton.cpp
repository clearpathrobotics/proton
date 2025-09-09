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

Node::Node(const std::string config_file, const std::string target)
    : config_(config_file), target_(target), rx_(0), tx_(0), rx_kbps_(0.0), tx_kbps_(0.0) {

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

  if (!connect())
  {
    std::cerr << "Connect error" << std::endl;
  }
}

void Node::startStatsThread()
{
  stats_thread_ = std::thread(&Node::runStatsThread, this);
}

void Node::sendBundle(const std::string &bundle_name) {
  sendBundle(getBundle(bundle_name));
}

void Node::sendBundle(BundleHandle &bundle_handle) {
  if (!connected()) {
    return;
  }

  auto bundle = bundle_handle.getBundlePtr().get();

  auto buf = std::make_unique<uint8_t[]>(bundle->ByteSizeLong());

  if (bundle->SerializeToArray(buf.get(), bundle->ByteSizeLong()))
  {
    tx_ += write(buf.get(), bundle->ByteSizeLong());

    if (target_node_config_.transport.type == transport_types::SERIAL)
    {
      tx_ += SerialTransport::FRAME_OVERHEAD;
    }

    bundle_handle.incrementTxCount();
  }
}

bool Node::registerCallback(const std::string &bundle_name, BundleHandle::BundleCallback callback)
{
  auto& bundle = getBundle(bundle_name);

  if (callback && bundle.getConsumer() == target_)
  {
    bundle.registerCallback(callback);
    return true;
  }

  return false;
}


void Node::spinOnce() {
  if (!connected()) {
    return;
  }

  uint8_t read_buf[PROTON_MAX_MESSAGE_SIZE];

  size_t bytes_read = read(read_buf, PROTON_MAX_MESSAGE_SIZE);

  if (bytes_read > 0) {
    auto& bundle = receiveBundle(read_buf, bytes_read);
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
}

void Node::spin() {
  while (1) {
    spinOnce();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
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
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
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

  std::cout << "---------------------------------" << std::endl;
}
