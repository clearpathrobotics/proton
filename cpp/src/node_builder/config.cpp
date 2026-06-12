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
 * @author Tom Wallis (thomas.wallis@rockwellautomation.com)
 */

#include "proton/proton_config.h"

#if PROTON_NODE_BUILDER

#include "protoncpp/node_builder/config.hpp"

#include <algorithm>
#include <sstream>

namespace proton::node_builder
{

// Internal parsing functions

static SignalConfig parse_signal(const ConfigNode & node)
{
  SignalConfig signal_config;

  signal_config.name = node[keys::NAME].as_string();
  signal_config.type_string = node[keys::TYPE].as_string();

  // Check that type string is a valid type
  bool valid_value_type = std::any_of(
    value_types::VALUE_TYPES.begin(), value_types::VALUE_TYPES.end(),
    [&signal_config](const std::string_view & val_type)
    { return signal_config.type_string == val_type; });

  if (!valid_value_type)
  {
    throw NodeBuilderException(
      "Signal value type " + signal_config.type_string + " is not a valid type");
  }

  signal_config.id = node[keys::ID].as_uint32();

  auto value_key = node[keys::VALUE];
  signal_config.has_default_value = value_key.is_defined();

  bool is_capacity_type = signal_config.type_string == value_types::BYTES ||
                          signal_config.type_string == value_types::STRING;

  auto capacity_key = node[keys::CAPACITY];
  if (capacity_key.is_defined())
  {
    signal_config.capacity = capacity_key.as_uint32();
  }
  else
  {
    signal_config.capacity = 0;
  }

  if (signal_config.has_default_value)
  {
    signal_config.value = value_key.value();

    // Determine capacity based on the size of the default value
    if (value_key.is_scalar() && signal_config.type_string == value_types::STRING)
    {
      std::string value = value_key.as_string();
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
        throw NodeBuilderException(ss.str());
      }
    }
    else if (value_key.is_sequence())
    {
      if (signal_config.type_string == value_types::BYTES)
      {
        if (signal_config.capacity == 0)
        {
          signal_config.capacity = value_key.size();
        }
        else if (signal_config.capacity < value_key.size())
        {
          std::stringstream ss;
          ss << "Error in signal: " << signal_config.name << ": Capacity " << signal_config.capacity
             << " is shorter than default value: " << value_key.size();
          throw NodeBuilderException(ss.str());
        }
      }
      else
      {
        throw NodeBuilderException(
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
    throw NodeBuilderException(ss.str());
  }

  return signal_config;
}

static BundleConfig parse_bundle(const ConfigNode & node)
{
  BundleConfig bundle_config;

  if (!(node[keys::NAME].is_defined() && node[keys::ID].is_defined() &&
        node[keys::PRODUCERS].is_defined() && node[keys::CONSUMERS].is_defined()))
  {
    throw NodeBuilderException("Bundle must contain name, ID, producers and consumers");
  }

  bundle_config.name = node[keys::NAME].as_string();
  bundle_config.id = node[keys::ID].as_uint32();
  auto signals = node[keys::SIGNALS];

  if (node[keys::PRODUCERS].is_sequence())
  {
    for (const auto & p : node[keys::PRODUCERS])
    {
      bundle_config.producers.push_back(p.as_string());
    }
  }
  else
  {
    throw NodeBuilderException(
      "Bundle " + bundle_config.name + " must have a sequence of producers");
  }

  if (node[keys::CONSUMERS].is_sequence())
  {
    for (const auto & p : node[keys::CONSUMERS])
    {
      bundle_config.consumers.push_back(p.as_string());
    }
  }
  else
  {
    throw NodeBuilderException(
      "Bundle " + bundle_config.name + " must have a sequence of consumers");
  }

  if (signals.is_defined())
  {
    if (signals.is_sequence())
    {
      for (const auto & signal : signals)
      {
        bundle_config.signals.push_back(signal.as_uint32());
      }
    }
    else
    {
      throw NodeBuilderException("Bundle " + bundle_config.name + " signals are not a list");
    }
  }

  bundle_config.period_ms = 0;
  auto period_node = node[keys::PERIOD_MS];
  if (period_node.is_defined())
  {
    bundle_config.period_ms = period_node.as_uint32();
  }

  return bundle_config;
}

static EndpointConfig parse_endpoint(const ConfigNode & node)
{
  EndpointConfig endpoint_config;

  if (!(node[keys::ID].is_defined() && node[keys::TYPE].is_defined()))
  {
    throw NodeBuilderException("Endpoint must define id and type");
  }

  endpoint_config.id = node[keys::ID].as_uint32();
  endpoint_config.type = node[keys::TYPE].as_string();

  const auto ip_node = node[keys::IP];
  const auto port_node = node[keys::PORT];
  const auto device_node = node[keys::DEVICE];

  if (endpoint_config.type == transport_types::UDP4)
  {
    if (!(ip_node && port_node))
    {
      throw NodeBuilderException("udp4 endpoints require ip and port");
    }
    endpoint_config.ip = ip_node.as_string();
    endpoint_config.port = port_node.as_uint32();
  }
  else if (endpoint_config.type == transport_types::SERIAL)
  {
    if (!device_node)
    {
      throw NodeBuilderException("serial endpoints require a device");
    }
    endpoint_config.device = device_node.as_string();
  }
  else
  {
    throw NodeBuilderException("Endpoint type " + endpoint_config.type + " is not a valid type");
  }

  return endpoint_config;
}

static NodeConfig parse_node(const ConfigNode & node)
{
  NodeConfig node_config;

  auto node_name = node[keys::NAME];
  auto node_id = node[keys::ID];
  auto endpoints = node[keys::ENDPOINTS];

  if (!(node_name.is_defined() && node_id.is_defined() && endpoints.is_defined()))
  {
    throw NodeBuilderException("Node name, ID, and endpoints must be defined");
  }

  node_config.name = node_name.as_string();
  node_config.id = node_id.as_uint32();

  if (endpoints.is_sequence())
  {
    for (const auto & endpoint : endpoints)
    {
      auto ep_id = endpoint[keys::ID];
      if (ep_id)
      {
        uint32_t id = ep_id.as_uint32();
        node_config.endpoints.emplace(id, parse_endpoint(endpoint));
      }
      else
      {
        throw NodeBuilderException("Node " + node_config.name + " endpoint requires an ID");
      }
    }
  }
  else
  {
    throw NodeBuilderException("Node " + node_config.name + " requires a sequence of endpoints");
  }

  return node_config;
}

static ConnectionEndpointConfig parse_connection_endpoint(const ConfigNode & node)
{
  ConnectionEndpointConfig conn_ep_config;

  auto node_id = node[keys::ID];
  auto node_name = node[keys::NODE];

  if (!(node_id && node_name))
  {
    throw NodeBuilderException("Connection element requires a node ID and name");
  }

  conn_ep_config.id = node_id.as_uint32();
  conn_ep_config.node = node_name.as_string();

  return conn_ep_config;
}

static ConnectionConfig parse_connection(const ConfigNode & node)
{
  ConnectionConfig conn_config;

  auto first = node[keys::FIRST];
  auto second = node[keys::SECOND];

  if (!(first && second))
  {
    throw NodeBuilderException("Node connection requires first and second element");
  }

  conn_config.first = parse_connection_endpoint(first);
  conn_config.second = parse_connection_endpoint(second);

  return conn_config;
}

// Config class implementation

Config::Config(const ConfigTree & tree) { parse(tree); }

#if PROTON_NODE_BUILDER_YAML_PARSER
Config Config::from_yaml(const std::string & yaml_file)
{
  Config config = Config();
  config.parse(ConfigTree::from_yaml_file(yaml_file));
  return config;
}
#endif  // PROTON_NODE_BUILDER_YAML_PARSER

#if PROTON_NODE_BUILDER_JSON_PARSER
Config Config::from_json(const std::string & json_file)
{
  Config config = Config();
  config.parse(ConfigTree::from_json_file(json_file));
  return config;
}
#endif  // PROTON_NODE_BUILDER_JSON_PARSER

void Config::parse(const ConfigTree & tree)
{
  // Get node configs
  auto nodes_node = tree[keys::NODES];
  if (nodes_node.is_sequence())
  {
    for (const auto & node : nodes_node)
    {
      NodeConfig config = parse_node(node);
      nodes.emplace(config.name, config);
    }
  }

  // Get connection configs
  auto connections_node = tree[keys::CONNECTIONS];
  if (connections_node.is_sequence())
  {
    for (const auto & node : connections_node)
    {
      connections.push_back(parse_connection(node));
    }
  }

  // Get bundle configs
  auto bundles_node = tree[keys::BUNDLES];
  if (bundles_node.is_sequence())
  {
    for (const auto & bundle : bundles_node)
    {
      bundles.push_back(parse_bundle(bundle));
    }
  }

  // Get signal configs
  auto signals_node = tree[keys::SIGNALS];
  if (signals_node.is_sequence())
  {
    for (const auto & signal : signals_node)
    {
      signals.push_back(parse_signal(signal));
    }
  }
}

}  // namespace proton::node_builder

#endif  // PROTON_NODE_BUILDER
