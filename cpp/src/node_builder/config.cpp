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
  static bool decode(const Node & yaml_node, proton::node_builder::SignalConfig & signal_config)
  {
    if (!yaml_node.IsDefined() || yaml_node.IsNull())
    {
      return false;
    }

    signal_config.name = yaml_node[proton::node_builder::keys::NAME].as<std::string>();
    signal_config.type_string = yaml_node[proton::node_builder::keys::TYPE].as<std::string>();

    // Check that type string is a valid type
    bool valid_value_type = std::any_of(
      proton::node_builder::value_types::VALUE_TYPES.begin(),
      proton::node_builder::value_types::VALUE_TYPES.end(),
      [&signal_config](const std::string_view & val_type)
      { return signal_config.type_string == val_type; });

    if (!valid_value_type)
    {
      throw proton::node_builder::NodeBuilderException(
        "Signal value type " + signal_config.type_string + " is not a valid type");
    }

    signal_config.id = yaml_node[proton::node_builder::keys::ID].as<uint32_t>();

    auto value_key = yaml_node[proton::node_builder::keys::VALUE];
    // Default value defined
    signal_config.has_default_value = value_key.IsDefined() && !value_key.IsNull();

    // Signal is a capacity type
    bool is_capacity_type = signal_config.type_string == proton::node_builder::value_types::BYTES ||
                            signal_config.type_string == proton::node_builder::value_types::STRING;

    auto capacity_key = yaml_node[proton::node_builder::keys::CAPACITY];
    if (capacity_key.IsDefined())
    {
      signal_config.capacity = capacity_key.as<uint32_t>();
    }
    else
    {
      signal_config.capacity = 0;
    }

    if (signal_config.has_default_value)
    {
      signal_config.value = value_key;

      // Determine capacity based on the size of the default value
      if (
        value_key.IsScalar() &&
        signal_config.type_string == proton::node_builder::value_types::STRING)
      {
        std::string value = value_key.as<std::string>();
        size_t string_capacity = value.size();
        if (signal_config.capacity == 0 || signal_config.capacity == string_capacity)
        {
          signal_config.capacity = string_capacity + 1;
        }
        else if (signal_config.capacity < string_capacity)
        {
          std::stringstream ss;
          ss << "Error in signal: " << signal_config.name << ": Capacity " << signal_config.capacity
             << " is shorter than default value: " << string_capacity << " + 1";
          throw proton::node_builder::NodeBuilderException(ss.str());
        }
      }
      else if (value_key.IsSequence())
      {
        if (signal_config.type_string == proton::node_builder::value_types::BYTES)
        {
          if (signal_config.capacity == 0)
          {
            signal_config.capacity = value_key.size();
          }
          else if (signal_config.capacity < value_key.size())
          {
            std::stringstream ss;
            ss << "Error in signal: " << signal_config.name << ": Capacity "
               << signal_config.capacity << " is shorter than default value: " << value_key.size();
            throw proton::node_builder::NodeBuilderException(ss.str());
          }
        }
        else
        {
          throw proton::node_builder::NodeBuilderException(
            "Error in signal " + signal_config.name +
            ": only bytes type signals can have a sequence as a default value");
        }
      }
    }
    else if (is_capacity_type && signal_config.capacity == 0)
    {
      std::stringstream ss;
      ss << "Signal " << signal_config.name << " of type " << signal_config.type_string
         << " must define a capacity";
      throw proton::node_builder::NodeBuilderException(ss.str());
    }

    return true;
  }
};

template <>
struct convert<proton::node_builder::BundleConfig>
{
  static bool decode(const Node & yaml_node, proton::node_builder::BundleConfig & bundle_config)
  {
    if (!yaml_node.IsDefined() || yaml_node.IsNull())
    {
      return false;
    }

    if (!(yaml_node[proton::node_builder::keys::NAME].IsDefined() &&
          yaml_node[proton::node_builder::keys::ID].IsDefined() &&
          yaml_node[proton::node_builder::keys::PRODUCERS].IsDefined() &&
          yaml_node[proton::node_builder::keys::CONSUMERS].IsDefined()))
    {
      throw proton::node_builder::NodeBuilderException(
        "Bundle must contain name, ID, producers and consumers");
    }

    bundle_config.name = yaml_node[proton::node_builder::keys::NAME].as<std::string>();
    bundle_config.id = yaml_node[proton::node_builder::keys::ID].as<uint32_t>();
    YAML::Node signals = yaml_node[proton::node_builder::keys::SIGNALS];

    if (yaml_node[proton::node_builder::keys::PRODUCERS].IsSequence())
    {
      for (const auto & p : yaml_node[proton::node_builder::keys::PRODUCERS])
      {
        bundle_config.producers.push_back(p.as<std::string>());
      }
    }
    else
    {
      throw proton::node_builder::NodeBuilderException(
        "Bundle " + bundle_config.name + " must have a sequence of producers");
    }

    if (yaml_node[proton::node_builder::keys::CONSUMERS].IsSequence())
    {
      for (const auto & p : yaml_node[proton::node_builder::keys::CONSUMERS])
      {
        bundle_config.consumers.push_back(p.as<std::string>());
      }
    }
    else
    {
      throw proton::node_builder::NodeBuilderException(
        "Bundle " + bundle_config.name + " must have a sequence of consumers");
    }

    if (signals.IsDefined() && !signals.IsNull())
    {
      if (signals.IsSequence())
      {
        // Get signal configs for this bundle
        for (const auto & signal : signals)
        {
          bundle_config.signals.push_back(signal.as<uint32_t>());
        }
      }
      else
      {
        throw proton::node_builder::NodeBuilderException(
          "Bundle " + bundle_config.name + " signals are not a list");
      }
    }

    bundle_config.period_ms = 0;
    auto period_node = yaml_node[proton::node_builder::keys::PERIOD_MS];
    if (period_node.IsDefined())
    {
      bundle_config.period_ms = period_node.as<uint32_t>();
    }

    return true;
  }
};

template <>
struct convert<proton::node_builder::EndpointConfig>
{
  static bool decode(const Node & yaml_node, proton::node_builder::EndpointConfig & endpoint_config)
  {
    if (!yaml_node.IsDefined() || yaml_node.IsNull())
    {
      return false;
    }

    if (!(yaml_node[proton::node_builder::keys::ID].IsDefined() &&
          yaml_node[proton::node_builder::keys::TYPE].IsDefined()))
    {
      throw proton::node_builder::NodeBuilderException("Endpoint must define id and type");
    }

    endpoint_config.id = yaml_node[proton::node_builder::keys::ID].as<uint32_t>();
    endpoint_config.type = yaml_node[proton::node_builder::keys::TYPE].as<std::string>();

    const auto ip_node = yaml_node[proton::node_builder::keys::IP];
    const auto port_node = yaml_node[proton::node_builder::keys::PORT];
    const auto device_node = yaml_node[proton::node_builder::keys::DEVICE];

    if (endpoint_config.type == proton::node_builder::transport_types::UDP4)
    {
      if (!(ip_node && port_node))
      {
        throw proton::node_builder::NodeBuilderException("udp4 endpoints require ip and port");
      }
      endpoint_config.ip = ip_node.as<std::string>();
      endpoint_config.port = port_node.as<uint32_t>();
    }
    else if (endpoint_config.type == proton::node_builder::transport_types::SERIAL)
    {
      if (!device_node)
      {
        throw proton::node_builder::NodeBuilderException("serial endpoints require a device");
      }
      endpoint_config.device = device_node.as<std::string>();
    }
    else
    {
      throw proton::node_builder::NodeBuilderException(
        "Endpoint type " + endpoint_config.type + " is not a valid type");
    }

    return true;
  }
};

template <>
struct convert<proton::node_builder::NodeConfig>
{
  static bool decode(const Node & yaml_node, proton::node_builder::NodeConfig & node_config)
  {
    if (!yaml_node.IsDefined() || yaml_node.IsNull())
    {
      return false;
    }

    auto node_name = yaml_node[proton::node_builder::keys::NAME];
    auto node_id = yaml_node[proton::node_builder::keys::ID];
    auto endpoints = yaml_node[proton::node_builder::keys::ENDPOINTS];

    if (!(node_name.IsDefined() && node_id.IsDefined() && endpoints.IsDefined()))
    {
      throw proton::node_builder::NodeBuilderException(
        "Node name, ID, and endpoints must be defined");
    }

    node_config.name = node_name.as<std::string>();
    node_config.id = node_id.as<uint32_t>();
    if (endpoints.IsSequence())
    {
      for (const auto & endpoint : endpoints)
      {
        auto ep_id = endpoint[proton::node_builder::keys::ID];
        if (ep_id)
        {
          uint32_t id = ep_id.as<uint32_t>();
          node_config.endpoints.emplace(id, endpoint.as<proton::node_builder::EndpointConfig>());
        }
        else
        {
          throw proton::node_builder::NodeBuilderException(
            "Node " + node_config.name + " endpoint requires an ID");
        }
      }
    }
    else
    {
      throw proton::node_builder::NodeBuilderException(
        "Node " + node_config.name + " requires a sequence of endpoints");
    }

    return true;
  }
};

template <>
struct convert<proton::node_builder::ConnectionEndpointConfig>
{
  static bool decode(
    const Node & yaml_node, proton::node_builder::ConnectionEndpointConfig & conn_ep_config)
  {
    if (!yaml_node.IsDefined() || yaml_node.IsNull())
    {
      return false;
    }

    auto node_id = yaml_node[proton::node_builder::keys::ID];
    auto node_name = yaml_node[proton::node_builder::keys::NODE];
    if (!(node_id && node_name))
    {
      throw proton::node_builder::NodeBuilderException(
        "Connection element requires a node ID and name");
    }

    conn_ep_config.id = node_id.as<uint32_t>();
    conn_ep_config.node = node_name.as<std::string>();

    return true;
  }
};

template <>
struct convert<proton::node_builder::ConnectionConfig>
{
  static bool decode(const Node & yaml_node, proton::node_builder::ConnectionConfig & conn_config)
  {
    if (!yaml_node.IsDefined() || yaml_node.IsNull())
    {
      return false;
    }

    auto first = yaml_node[proton::node_builder::keys::FIRST];
    auto second = yaml_node[proton::node_builder::keys::SECOND];

    if (!(first && second))
    {
      throw proton::node_builder::NodeBuilderException(
        "Node connection requires first and second element");
    }

    conn_config.first = first.as<proton::node_builder::ConnectionEndpointConfig>();
    conn_config.second = second.as<proton::node_builder::ConnectionEndpointConfig>();

    return true;
  }
};

}  // namespace YAML

namespace proton::node_builder
{

Config::Config(const std::string & file)
{
  yaml_node_ = YAML::LoadFile(file);

  // Get yaml_node configs
  for (auto yaml_node : yaml_node_[keys::NODES])
  {
    NodeConfig config = yaml_node.as<NodeConfig>();
    nodes.emplace(config.name, config);
  }

  // Get connection configs
  for (auto yaml_node : yaml_node_[keys::CONNECTIONS])
  {
    ConnectionConfig config = yaml_node.as<ConnectionConfig>();
    connections.push_back(config);
  }

  // Get bundle configs
  for (auto bundle : yaml_node_[keys::BUNDLES])
  {
    bundles.push_back(bundle.as<BundleConfig>());
  }

  // Get signal configs
  for (auto signal : yaml_node_[keys::SIGNALS])
  {
    signals.push_back(signal.as<SignalConfig>());
  }
}

}  // namespace proton::node_builder

#endif  // PROTON_NODE_BUILDER
