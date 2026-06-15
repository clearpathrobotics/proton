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
#include "protoncpp/node_builder/config_tree.hpp"

#include <array>
#include <cstdint>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace proton::node_builder
{

class NodeBuilderException : public std::runtime_error
{
public:
  explicit NodeBuilderException(const std::string & message) : std::runtime_error(message) {}
};

namespace keys
{
inline constexpr std::string_view NODES = "nodes";
inline constexpr std::string_view BUNDLES = "bundles";
inline constexpr std::string_view NAME = "name";
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
  SignalConfig() = default;
  SignalConfig(std::string name, uint32_t id, std::string type_string)
  : name(std::move(name)),
    id(id),
    type_string(std::move(type_string)),
    value_size(0),
    capacity(0),
    has_default_value(false)
  {
  }
  SignalConfig(std::string name, uint32_t id, std::string type_string, uint16_t capacity)
  : name(std::move(name)),
    id(id),
    type_string(std::move(type_string)),
    value_size(capacity),
    capacity(capacity),
    has_default_value(false)
  {
  }

  std::string name;
  uint32_t id{};
  std::string type_string;
  uint16_t value_size{};
  uint16_t capacity{};
  bool has_default_value{};
  ConfigValue value;
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
  ConnectionEndpointConfig first;
  ConnectionEndpointConfig second;
};

/**
 * @class Config top-level configuration representation for generating nodes and registries
 */
class Config
{
public:
  explicit Config() = default;

  /**
   * @brief Construct from a ConfigTree (format-agnostic)
   */
  explicit Config(const ConfigTree & tree);

  /**
   * @brief Convenience: construct from YAML file path
   */
  static Config from_yaml(const std::string & yaml_file);

  /**
   * @brief Convenience: construct from JSON file path
   */
  static Config from_json(const std::string & json_file);

  ~Config() = default;

  std::vector<BundleConfig> bundles;
  std::map<std::string, NodeConfig> nodes;
  std::vector<ConnectionConfig> connections;
  std::vector<SignalConfig> signals;

private:
  void parse(const ConfigTree & tree);
};

}  // namespace proton::node_builder

#endif  // PROTON_NODE_BUILDER
#endif  // PROTON_NODE_BUILDER_CONFIG_HPP
