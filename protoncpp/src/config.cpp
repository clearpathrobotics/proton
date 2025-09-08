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
#include "yaml-cpp/yaml.h"
#include <iostream>

using namespace proton;

Config::Config() {}

Config::Config(std::string file) {
  std::string yaml_file_name = file.substr(file.find_last_of('/') + 1);
  name_ = yaml_file_name.substr(0, yaml_file_name.find(".yaml"));

  YAML::Node config = YAML::LoadFile(file);

  for (auto node : config[keys::NODES]) {
    auto transport = node[keys::TRANSPORT];
    std::string transport_type = transport[keys::TYPE].as<std::string>();
    if (transport_type == TransportConfig::TYPE_UDP4) {
      nodes_.push_back(NodeConfig(node[keys::NAME].as<std::string>(),
                                  transport[keys::IP].as<std::string>(),
                                  transport[keys::PORT].as<uint32_t>()));
    } else if (transport_type == TransportConfig::TYPE_SERIAL) {
      nodes_.push_back(NodeConfig(node[keys::NAME].as<std::string>(),
                                  transport[keys::DEVICE].as<std::string>()));
    }
  }

  // Get bundle configs
  for (auto bundle : config[keys::BUNDLES]) {
    std::vector<SignalConfig> signal_configs;

    // Get signal configs for this bundle
    for (auto signal : bundle[keys::SIGNALS]) {
      uint32_t length, capacity;

      try {
        length = signal[keys::LENGTH].as<uint32_t>();
      } catch (const YAML::TypedBadConversion<uint32_t> &e) {
        length = 0;
      }

      try {
        capacity = signal[keys::CAPACITY].as<uint32_t>();
      } catch (const YAML::TypedBadConversion<uint32_t> &e) {
        capacity = 0;
      }

      SignalConfig config = {signal[keys::NAME].as<std::string>(),
                             bundle[keys::NAME].as<std::string>(),
                             signal[keys::TYPE].as<std::string>(), length,
                             capacity};

      signal_configs.push_back(config);
    }

    BundleConfig bundle_config = {
        bundle[keys::NAME].as<std::string>(), bundle[keys::ID].as<uint32_t>(),
        bundle[keys::PRODUCER].as<std::string>(),
        bundle[keys::CONSUMER].as<std::string>(), signal_configs};

    bundles_.push_back(bundle_config);
  }
}
