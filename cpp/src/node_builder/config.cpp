/*
 * Copyright 2026 Rockwell Automation Technologies, Inc., All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @author Roni Kreinin (roni.kreinin@rockwellautomation.com)
 */

#include "proton/proton_config.h"

#if PROTON_NODE_BUILDER

#include "protoncpp/node_builder/config.hpp"

#include <sstream>

namespace YAML
{

template <>
struct convert<proton::node_builder::SignalConfig>
{
  static bool decode(const Node & node, proton::node_builder::SignalConfig & rhs)
  {
    if (!node.IsDefined() || node.IsNull())
    {
      return false;
    }

    rhs.name = node[proton::node_builder::keys::NAME].as<std::string>();
    rhs.type_string = node[proton::node_builder::keys::TYPE].as<std::string>();

    // Check that type string is a valid type
    bool valid_value_type = std::any_of(
      proton::node_builder::value_types::VALUE_TYPES.begin(),
      proton::node_builder::value_types::VALUE_TYPES.end(),
      [&rhs](const std::string_view & val_type) { return rhs.type_string == val_type; });

    if (!valid_value_type)
    {
      throw std::runtime_error("Signal value type " + rhs.type_string + " is not a valid type");
    }

    rhs.id = node[proton::node_builder::keys::ID].as<uint32_t>();

    auto value_key = node[proton::node_builder::keys::VALUE];
    // Default value defined
    rhs.has_default_value = value_key.IsDefined() && !value_key.IsNull();

    // Signal is a capacity type
    bool is_capacity_type = rhs.type_string == proton::node_builder::value_types::BYTES ||
                            rhs.type_string == proton::node_builder::value_types::STRING;

    auto capacity_key = node[proton::node_builder::keys::CAPACITY];
    if (capacity_key.IsDefined())
    {
      rhs.capacity = capacity_key.as<uint32_t>();
    }
    else
    {
      rhs.capacity = 0;
    }

    if (rhs.has_default_value)
    {
      rhs.value = value_key;

      // Determine capacity based on the size of the default value
      if (value_key.IsScalar() && rhs.type_string == proton::node_builder::value_types::STRING)
      {
        std::string value = value_key.as<std::string>();
        size_t string_capacity = value.size() + 1;
        if (rhs.capacity == 0)
        {
          rhs.capacity = string_capacity;
        }
        else if (rhs.capacity < string_capacity)
        {
          std::stringstream ss;
          ss << "Error in signal: " << rhs.name << ": Capacity " << rhs.capacity
             << " is shorter than default value: " << string_capacity;
          throw std::runtime_error(ss.str());
        }
      }
      else if (value_key.IsSequence())
      {
        if (rhs.type_string == proton::node_builder::value_types::BYTES)
        {
          if (rhs.capacity == 0)
          {
            rhs.capacity = value_key.size();
          }
          else if (rhs.capacity < value_key.size())
          {
            std::stringstream ss;
            ss << "Error in signal: " << rhs.name << ": Capacity " << rhs.capacity
               << " is shorter than default value: " << value_key.size();
            throw std::runtime_error(ss.str());
          }
        }
        else
        {
          throw std::runtime_error(
            "Error in signal " + rhs.name +
            ": only bytes type signals can have a sequence as a default value");
        }
      }
    }
    else if (is_capacity_type && rhs.capacity == 0)
    {
      std::stringstream ss;
      ss << "Signal " << rhs.name << " of type " << rhs.type_string << " must define a capacity";
      throw std::runtime_error(ss.str());
    }

    return true;
  }
};

template <>
struct convert<proton::node_builder::BundleConfig>
{
  static bool decode(const Node & node, proton::node_builder::BundleConfig & rhs)
  {
    if (!node.IsDefined() || node.IsNull())
    {
      return false;
    }

    if (!(node[proton::node_builder::keys::NAME].IsDefined() &&
          node[proton::node_builder::keys::ID].IsDefined() &&
          node[proton::node_builder::keys::PRODUCERS].IsDefined() &&
          node[proton::node_builder::keys::CONSUMERS].IsDefined()))
    {
      throw std::runtime_error("Bundle must contain name, ID, producers and consumers");
    }

    rhs.name = node[proton::node_builder::keys::NAME].as<std::string>();
    rhs.id = node[proton::node_builder::keys::ID].as<uint32_t>();
    YAML::Node signals = node[proton::node_builder::keys::SIGNALS];

    if (node[proton::node_builder::keys::PRODUCERS].IsSequence())
    {
      for (const auto & p : node[proton::node_builder::keys::PRODUCERS])
      {
        rhs.producers.push_back(p.as<std::string>());
      }
    }
    else
    {
      throw std::runtime_error("Bundle " + rhs.name + " must have a sequence of producers");
    }

    if (node[proton::node_builder::keys::CONSUMERS].IsSequence())
    {
      for (const auto & p : node[proton::node_builder::keys::CONSUMERS])
      {
        rhs.consumers.push_back(p.as<std::string>());
      }
    }
    else
    {
      throw std::runtime_error("Bundle " + rhs.name + " must have a sequence of producers");
    }

    if (signals.IsDefined() && !signals.IsNull())
    {
      if (signals.IsSequence())
      {
        // Get signal configs for this bundle
        for (const auto & signal : signals)
        {
          rhs.signals.push_back(signal.as<uint32_t>());
        }
      }
      else
      {
        throw std::runtime_error("Bundle " + rhs.name + " signals are not a list");
      }
    }

    rhs.period_ms = 0;
    auto period_node = node[proton::node_builder::keys::PERIOD_MS];
    if (period_node.IsDefined())
    {
      rhs.period_ms = period_node.as<uint32_t>();
    }

    return true;
  }
};

template <>
struct convert<proton::node_builder::EndpointConfig>
{
  static bool decode(const Node & node, proton::node_builder::EndpointConfig & rhs)
  {
    if (!node.IsDefined() || node.IsNull())
    {
      return false;
    }

    if (!(node[proton::node_builder::keys::ID].IsDefined() &&
          node[proton::node_builder::keys::TYPE].IsDefined()))
    {
      return false;
    }

    rhs.id = node[proton::node_builder::keys::ID].as<uint32_t>();
    rhs.type = node[proton::node_builder::keys::TYPE].as<std::string>();

    auto ip_node = node[proton::node_builder::keys::IP];
    auto port_node = node[proton::node_builder::keys::PORT];
    auto device_node = node[proton::node_builder::keys::DEVICE];

    if (rhs.type == proton::node_builder::transport_types::UDP4)
    {
      rhs.ip = ip_node.as<std::string>();
      rhs.port = port_node.as<uint32_t>();
    }
    else if (rhs.type == proton::node_builder::transport_types::SERIAL)
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

template <>
struct convert<proton::node_builder::NodeConfig>
{
  static bool decode(const Node & node, proton::node_builder::NodeConfig & rhs)
  {
    if (!node.IsDefined() || node.IsNull())
    {
      return false;
    }

    auto node_name = node[proton::node_builder::keys::NAME];
    auto node_id = node[proton::node_builder::keys::ID];
    auto endpoints = node[proton::node_builder::keys::ENDPOINTS];

    if (!(node_name.IsDefined() && node_id.IsDefined() && endpoints.IsDefined()))
    {
      throw std::runtime_error("Node name, ID, and endpoints must be defined");
    }

    rhs.name = node_name.as<std::string>();
    rhs.id = node_id.as<uint32_t>();
    if (endpoints.IsSequence())
    {
      for (const auto & endpoint : endpoints)
      {
        auto ep_id = endpoint[proton::node_builder::keys::ID];
        if (ep_id)
        {
          uint32_t id = ep_id.as<uint32_t>();
          rhs.endpoints.emplace(id, endpoint.as<proton::node_builder::EndpointConfig>());
        }
        else
        {
          throw std::runtime_error("Node " + rhs.name + " endpoint requires an ID");
        }
      }
    }
    else
    {
      throw std::runtime_error("Node " + rhs.name + " requires a sequence of endpoints");
    }

    return true;
  }
};

template <>
struct convert<proton::node_builder::ConnectionEndpointConfig>
{
  static bool decode(const Node & node, proton::node_builder::ConnectionEndpointConfig & rhs)
  {
    if (!node.IsDefined() || node.IsNull())
    {
      return false;
    }

    auto node_id = node[proton::node_builder::keys::ID];
    auto node_name = node[proton::node_builder::keys::NODE];
    if (!(node_id && node_name))
    {
      throw std::runtime_error("Connection element requires a node ID and name");
    }

    rhs.id = node_id.as<uint32_t>();
    rhs.node = node_name.as<std::string>();

    return true;
  }
};

template <>
struct convert<proton::node_builder::ConnectionConfig>
{
  static bool decode(const Node & node, proton::node_builder::ConnectionConfig & rhs)
  {
    if (!node.IsDefined() || node.IsNull())
    {
      return false;
    }

    auto first = node[proton::node_builder::keys::FIRST];
    auto second = node[proton::node_builder::keys::SECOND];

    if (!(first && second))
    {
      throw std::runtime_error("Node connection requires first and second element");
    }

    rhs.connection.first = first.as<proton::node_builder::ConnectionEndpointConfig>();
    rhs.connection.second = second.as<proton::node_builder::ConnectionEndpointConfig>();

    return true;
  }
};

}  // namespace YAML

namespace proton::node_builder
{

Config::Config(const std::string & file)
{
  yaml_node_ = YAML::LoadFile(file);

  // Get node configs
  for (auto node : yaml_node_[keys::NODES])
  {
    NodeConfig config = node.as<NodeConfig>();
    nodes_.emplace(config.name, config);
  }

  // Get connection configs
  for (auto node : yaml_node_[keys::CONNECTIONS])
  {
    ConnectionConfig config = node.as<ConnectionConfig>();
    connections_.push_back(config);
  }

  // Get bundle configs
  for (auto bundle : yaml_node_[keys::BUNDLES])
  {
    bundles_.push_back(bundle.as<BundleConfig>());
  }

  // Get signal configs
  for (auto signal : yaml_node_[keys::SIGNALS])
  {
    signals_.push_back(signal.as<SignalConfig>());
  }
}

}  // namespace proton::node_builder

#endif  // PROTON_NODE_BUILDER
