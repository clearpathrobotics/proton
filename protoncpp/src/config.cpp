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

namespace YAML {

template<>
struct convert<proton::SignalConfig> {
  static bool decode(const Node& node, proton::SignalConfig& rhs) {
    if(!node.IsDefined() || node.IsNull()) {
      return false;
    }

    rhs.name = node[proton::keys::NAME].as<std::string>();
    rhs.type_string = node[proton::keys::TYPE].as<std::string>();

    auto length_key = node[proton::keys::LENGTH];
    if (length_key.IsDefined())
    {
      rhs.length = length_key.as<uint32_t>();
    }

    auto capacity_key = node[proton::keys::CAPACITY];
    if (capacity_key.IsDefined())
    {
      rhs.capacity = capacity_key.as<uint32_t>();
    }

    auto value_key = node[proton::keys::VALUE];
    if (value_key.IsDefined())
    {
      rhs.value = value_key;

      if (value_key.IsScalar() && rhs.type_string == proton::value_types::STRING)
      {
        rhs.capacity = value_key.size();
      }
      else if (value_key.IsSequence())
      {
        if (rhs.type_string == proton::value_types::BYTES)
        {
          rhs.capacity = value_key.size();
        }
        else
        {
          rhs.length = value_key.size();
        }
      }
    }

    return true;
  }
};

template<>
struct convert<proton::BundleConfig> {
  static bool decode(const Node& node, proton::BundleConfig& rhs) {
    if(!node.IsDefined() || node.IsNull()) {
      return false;
    }

    rhs.name = node[proton::keys::NAME].as<std::string>();
    rhs.id = node[proton::keys::ID].as<uint32_t>();
    rhs.producer = node[proton::keys::PRODUCER].as<std::string>();
    rhs.consumer = node[proton::keys::CONSUMER].as<std::string>();

    YAML::Node signals = node[proton::keys::SIGNALS];

    if (signals.IsDefined() && !signals.IsNull())
    {
      // Get signal configs for this bundle
      for (auto signal : signals) {
        rhs.signals.push_back(signal.as<proton::SignalConfig>());
      }
    }

    return true;
  }
};

template<>
struct convert<proton::TransportConfig> {
  static bool decode(const Node& node, proton::TransportConfig& rhs) {
    if(!node.IsDefined() || node.IsNull()) {
      return false;
    }

    rhs.type = node[proton::keys::TYPE].as<std::string>();

    auto ip_node = node[proton::keys::IP];
    auto port_node = node[proton::keys::PORT];
    auto device_node = node[proton::keys::DEVICE];

    if (rhs.type == proton::transport_types::UDP4)
    {
      rhs.ip = ip_node.as<std::string>();
      rhs.port = port_node.as<uint32_t>();
    }
    else if (rhs.type == proton::transport_types::SERIAL)
    {
      rhs.device = device_node.as<std::string>();
    }
    else
    {
      return false;
    }

    return true;
  }
};

template<>
struct convert<proton::NodeConfig> {
  static bool decode(const Node& node, proton::NodeConfig& rhs) {
    if(!node.IsDefined() || node.IsNull()) {
      return false;
    }

    rhs.name = node[proton::keys::NAME].as<std::string>();
    rhs.transport = node[proton::keys::TRANSPORT].as<proton::TransportConfig>();

    return true;
  }
};

}

using namespace proton;

Config::Config() {}

Config::Config(std::string file) {
  std::string yaml_file_name = file.substr(file.find_last_of('/') + 1);
  name_ = yaml_file_name.substr(0, yaml_file_name.find(".yaml"));

  yaml_node_ = YAML::LoadFile(file);

  // Get node configs
  for (auto node : yaml_node_[keys::NODES]) {
    nodes_.push_back(node.as<NodeConfig>());
  }

  // Get bundle configs
  for (auto bundle : yaml_node_[keys::BUNDLES]) {
    bundles_.push_back(bundle.as<BundleConfig>());
  }
}
