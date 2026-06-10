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

#ifndef PROTON_NODE_BUILDER_CONFIG_HPP
#define PROTON_NODE_BUILDER_CONFIG_HPP

#include "proton/proton_config.h"

#if PROTON_NODE_BUILDER

#include "proton/node_manager.h"

#include <array>
#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include <yaml-cpp/yaml.h>

namespace proton::node_builder
{

namespace keys
{
inline constexpr std::string_view NODES = "nodes";
inline constexpr std::string_view BUNDLES = "bundles";
inline constexpr std::string_view NAME = "name";
inline constexpr std::string_view ENABLED = "enabled";
inline constexpr std::string_view PERIOD = "period";
inline constexpr std::string_view ENDPOINTS = "endpoints";
inline constexpr std::string_view TYPE = "type";
inline constexpr std::string_view IP = "ip";
inline constexpr std::string_view PORT = "port";
inline constexpr std::string_view DEVICE = "device";
inline constexpr std::string_view CONNECTIONS = "connections";
inline constexpr std::string_view FIRST = "first";
inline constexpr std::string_view SECOND = "second";
inline constexpr std::string_view NODE = "node";
inline constexpr std::string_view CAPACITY = "capacity";
inline constexpr std::string_view VALUE = "value";
inline constexpr std::string_view ID = "id";
inline constexpr std::string_view PRODUCERS = "producers";
inline constexpr std::string_view CONSUMERS = "consumers";
inline constexpr std::string_view SIGNALS = "signals";
inline constexpr std::string_view PERIOD_MS = "period_ms";
}  // namespace keys

namespace value_types
{
inline constexpr std::string_view DOUBLE = "double";
inline constexpr std::string_view FLOAT = "float";
inline constexpr std::string_view INT32 = "int32";
inline constexpr std::string_view INT64 = "int64";
inline constexpr std::string_view UINT32 = "uint32";
inline constexpr std::string_view UINT64 = "uint64";
inline constexpr std::string_view BOOL = "bool";
inline constexpr std::string_view STRING = "string";
inline constexpr std::string_view BYTES = "bytes";

constexpr std::array<std::string_view, 9> VALUE_TYPES = {DOUBLE, FLOAT, INT32,  INT64, UINT32,
                                                         UINT64, BOOL,  STRING, BYTES};

}  // namespace value_types

namespace transport_types
{
inline constexpr std::string_view UDP4 = "udp4";
inline constexpr std::string_view SERIAL = "serial";
}  // namespace transport_types

struct SignalConfig
{
  std::string name;
  uint32_t id;
  std::string type_string;
  uint32_t capacity;
  bool has_default_value;
  ::YAML::Node value;
};

struct BundleConfig
{
  std::string name;
  uint32_t id;
  uint32_t period_ms;
  std::vector<std::string> producers;
  std::vector<std::string> consumers;
  std::vector<uint32_t> signals;
};

struct EndpointConfig
{
  uint32_t id;
  std::string type;
  std::string device;
  std::string ip;
  uint32_t port;
};

struct NodeConfig
{
  std::string name;
  uint32_t id;
  std::map<uint32_t, EndpointConfig> endpoints;
};

struct ConnectionEndpointConfig
{
  uint32_t id;
  std::string node;
};

struct ConnectionConfig
{
  std::pair<ConnectionEndpointConfig, ConnectionEndpointConfig> connection;
};

class Config
{
public:
  Config(const std::string & file);
  virtual ~Config() = default;

  std::vector<BundleConfig> bundles_;
  std::map<std::string, NodeConfig> nodes_;
  std::vector<ConnectionConfig> connections_;
  std::vector<SignalConfig> signals_;
  ::YAML::Node yaml_node_;
};

}  // namespace proton::node_builder

#endif  // PROTON_NODE_BUILDER
#endif  // PROTON_NODE_BUILDER_CONFIG_HPP
