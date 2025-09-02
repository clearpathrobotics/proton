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
    : config_(config_file), target_(target) {
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
}

void Node::spinOnce() {
  uint8_t read_buf[1024];
  if (!connected()) {
    if (!connect()) {
      std::cerr << "Connect error" << std::endl;
    }
  } else {
    size_t bytes_read = read(read_buf, 1024);
    if (bytes_read > 0)
    {
      auto bundle = receiveBundle(read_buf, bytes_read);
      //std::cout << "Received 0x" << std::hex << bundle.id << std::endl;
    }
  }
}
