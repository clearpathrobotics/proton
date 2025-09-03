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


using namespace proton;

Node::Node() {}

Node::Node(const std::string config_file, const std::string target)
    : config_(config_file), target_(target), rx_(0), tx_(0) {
  for (auto b : config_.getBundles()) {
    addBundle(b);
  }

  NodeConfig target_config, peer_config;

  for (auto n : config_.getNodes()) {
    // Target node config
    if (n.getName() == target_) {
      target_config = n;
    }
    // Peer node config
    else {
      peer_config = n;
    }
  }

  auto transport_type = target_config.getTransport().getType();
  if (transport_type == TransportConfig::TYPE_UDP4) {
    auto target_endpoint =
        proton::socket_endpoint(target_config.getTransport().getIP(),
                                target_config.getTransport().getPort());
    auto peer_endpoint =
        proton::socket_endpoint(peer_config.getTransport().getIP(),
                                peer_config.getTransport().getPort());
    setTransport(
        std::make_unique<Udp4Transport>(target_endpoint, peer_endpoint));
  }

  if (!connect())
  {
    std::cerr << "Connect error" << std::endl;
  }
}

void Node::sendBundle(const std::string &bundle_name) {
  if (!connected()) {
    return;
  }

  auto bundle = getBundle(bundle_name).bundle.get();

  auto buf = std::make_unique<uint8_t[]>(bundle->ByteSizeLong());

  if (bundle->SerializeToArray(buf.get(), bundle->ByteSizeLong()))
  {
    tx_ += write(buf.get(), bundle->ByteSizeLong());
  }
}

void Node::spinOnce() {
  if (!connected()) {
    return;
  }

  uint8_t read_buf[1024];

  size_t bytes_read = read(read_buf, 1024);
  if (bytes_read > 0) {
    auto bundle = receiveBundle(read_buf, bytes_read);
    rx_ += bytes_read;
    // std::cout << "Received 0x" << std::hex << bundle.id << std::endl;
  }
}

void Node::spin() {
  while (1) {
    spinOnce();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}
