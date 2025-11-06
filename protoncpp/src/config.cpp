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

    auto value_key = node[proton::keys::VALUE];
    // Constant value defined
    if (value_key.IsDefined() && !value_key.IsNull())
    {
      rhs.is_const = true;
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
        else if (rhs.type_string == proton::value_types::LIST_STRING || rhs.type_string == proton::value_types::LIST_BYTES)
        {
          // Set capacity to largest of values
          for (const auto& v : value_key)
          {
            if (v.size() > rhs.capacity)
            {
              rhs.capacity = v.size();
            }
          }

          // Set length to sequence size
          rhs.length = value_key.size();
        }
        else
        {
          rhs.length = value_key.size();
        }
      }
    }
    else
    {
      rhs.is_const = false;

      auto length_key = node[proton::keys::LENGTH];
      if (length_key.IsDefined())
      {
        rhs.length = length_key.as<uint32_t>();
      }
      else if (proton::signal_map::SignalMap.at(rhs.type_string) >= proton::Signal::SignalCase::kListDoubleValue)
      {
        throw std::runtime_error("Signal " + rhs.name + " of type " + rhs.type_string + " must define a length");
      }

      auto capacity_key = node[proton::keys::CAPACITY];
      if (capacity_key.IsDefined())
      {
        rhs.capacity = capacity_key.as<uint32_t>();
      }
      else
      {
        switch(proton::signal_map::SignalMap.at(rhs.type_string))
        {
          case proton::Signal::SignalCase::kStringValue:
          case proton::Signal::SignalCase::kBytesValue:
          case proton::Signal::SignalCase::kListStringValue:
          case proton::Signal::SignalCase::kListBytesValue:
          {
            throw std::runtime_error("Signal " + rhs.name + " of type " + rhs.type_string + " must define a capacity");
          }
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
    rhs.producer = node[proton::keys::PRODUCER].as<std::string>();
    rhs.consumer = node[proton::keys::CONSUMER].as<std::string>();
    rhs.id = node[proton::keys::ID].as<uint32_t>();
    if (rhs.id == 0U)
    {
      throw std::runtime_error("Bundle ID cannot be 0");
    }

    YAML::Node signals = node[proton::keys::SIGNALS];

    if (signals.IsDefined() && !signals.IsNull())
    {
      // Get signal configs for this bundle
      for (const auto& signal : signals) {
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
struct convert<proton::HeartbeatConfig> {
  static bool decode(const Node& node, proton::HeartbeatConfig& rhs) {
    if(!node.IsDefined() || node.IsNull()) {
      return false;
    }

    if (node[proton::keys::ENABLED])
    {
      rhs.enabled = node[proton::keys::ENABLED].as<bool>();
    }
    else
    {
      rhs.enabled = false;
    }

    if (node[proton::keys::PERIOD])
    {
      rhs.period = node[proton::keys::PERIOD].as<uint32_t>();
    }
    else
    {
      rhs.period = 1000;
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
    if (node[proton::keys::HEARTBEAT])
    {
      rhs.heartbeat = node[proton::keys::HEARTBEAT].as<proton::HeartbeatConfig>();
    }
    else
    {
      rhs.heartbeat.enabled = false;
    }

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
    NodeConfig config = node.as<NodeConfig>();
    nodes_.emplace(config.name, config);
  }

  // Get bundle configs
  for (auto bundle : yaml_node_[keys::BUNDLES]) {
    bundles_.push_back(bundle.as<BundleConfig>());
  }
}
