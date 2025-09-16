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

#include "protoncpp/config.hpp"

#include <iostream>

using namespace proton;

Config::Config() {}

Config::Config(std::string file) {
  std::string yaml_file_name = file.substr(file.find_last_of('/') + 1);
  name_ = yaml_file_name.substr(0, yaml_file_name.find(".yaml"));

  yaml_node_ = YAML::LoadFile(file);

  for (auto node : yaml_node_[keys::NODES]) {
    auto transport = node[keys::TRANSPORT];

    std::string type, device, ip;
    uint32_t port;

    type = transport[keys::TYPE].as<std::string>();

    if (type == transport_types::UDP4)
    {
      ip = transport[keys::IP].as<std::string>();
      port = transport[keys::PORT].as<uint32_t>();

      try {
        device = transport[keys::DEVICE].as<std::string>();
      }
      catch (YAML::TypedBadConversion<std::string>& e)
      {}
    }
    else if (type == transport_types::SERIAL)
    {
      device = transport[keys::DEVICE].as<std::string>();

      try {
        ip = transport[keys::IP].as<std::string>();
      }
      catch (YAML::TypedBadConversion<std::string>& e)
      {}

      try {
        port = transport[keys::PORT].as<uint32_t>();
      }
      catch (YAML::TypedBadConversion<uint32_t>& e)
      {}
    }

    TransportConfig transport_config = {
      type,
      device,
      ip,
      port
    };

    NodeConfig node_config = {
      node[keys::NAME].as<std::string>(),
      transport_config
    };

    nodes_.push_back(node_config);
  }

  // Get bundle configs
  for (auto bundle : yaml_node_[keys::BUNDLES]) {
    bundles_.push_back(bundle.as<BundleConfig>());
  }
}
