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

Node::Node(const std::string config_file) : config_(config_file) {
  for (auto b : config_.getBundles()) {
    addBundle(b);
  }

  for (auto n : config_.getNodes()) {
    auto& transport_config = n.getTransport();
    if (n.getTransport().getType() == TransportConfig::TYPE_UDP4)
    {
      auto& udp_config = dynamic_cast<Udp4TransportConfig&>(transport_config);
      transport_ = Udp4Transport(udp_config);
    }
  }
}

