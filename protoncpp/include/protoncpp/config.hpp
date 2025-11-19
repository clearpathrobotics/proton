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

#ifndef INC_PROTONCPP_CONFIG_HPP_
#define INC_PROTONCPP_CONFIG_HPP_

#include "stdint.h"
#include <map>
#include <shared_mutex>
#include <string>
#include <vector>

#include "yaml-cpp/yaml.h"
#include "protoncpp/bundle.pb.h"
#include "protoncpp/signal.pb.h"


namespace proton {

namespace keys {
  static const char *const NODES = "nodes";
  static const char *const BUNDLES = "bundles";
  static const char *const NAME = "name";
  static const char *const HEARTBEAT = "heartbeat";
  static const char *const ENABLED = "enabled";
  static const char *const PERIOD = "period";
  static const char *const ENDPOINTS = "endpoints";
  static const char *const TYPE = "type";
  static const char *const IP = "ip";
  static const char *const PORT = "port";
  static const char *const DEVICE = "device";
  static const char *const CONNECTIONS = "connections";
  static const char *const FIRST = "first";
  static const char *const SECOND = "second";
  static const char *const NODE = "node";
  static const char *const LENGTH = "length";
  static const char *const CAPACITY = "capacity";
  static const char *const VALUE = "value";
  static const char *const ID = "id";
  static const char *const PRODUCERS = "producers";
  static const char *const CONSUMERS = "consumers";
  static const char *const SIGNALS = "signals";
} // namespace keys

namespace value_types {
  static constexpr std::string_view DOUBLE = "double";
  static constexpr std::string_view FLOAT = "float";
  static constexpr std::string_view INT32 = "int32";
  static constexpr std::string_view INT64 = "int64";
  static constexpr std::string_view UINT32 = "uint32";
  static constexpr std::string_view UINT64 = "uint64";
  static constexpr std::string_view BOOL = "bool";
  static constexpr std::string_view STRING = "string";
  static constexpr std::string_view BYTES = "bytes";
  static constexpr std::string_view LIST_DOUBLE = "list_double";
  static constexpr std::string_view LIST_FLOAT = "list_float";
  static constexpr std::string_view LIST_INT32 = "list_int32";
  static constexpr std::string_view LIST_INT64 = "list_int64";
  static constexpr std::string_view LIST_UINT32 = "list_uint32";
  static constexpr std::string_view LIST_UINT64 = "list_uint64";
  static constexpr std::string_view LIST_BOOL = "list_bool";
  static constexpr std::string_view LIST_STRING = "list_string";
  static constexpr std::string_view LIST_BYTES = "list_bytes";
} // namespace value_types

namespace signal_map {
  const std::map<std::string_view, proton::Signal::SignalCase> SignalMap = {
      {value_types::DOUBLE, proton::Signal::SignalCase::kDoubleValue},
      {value_types::FLOAT, proton::Signal::SignalCase::kFloatValue},
      {value_types::INT32, proton::Signal::SignalCase::kInt32Value},
      {value_types::INT64, proton::Signal::SignalCase::kInt64Value},
      {value_types::UINT32, proton::Signal::SignalCase::kUint32Value},
      {value_types::UINT64, proton::Signal::SignalCase::kUint64Value},
      {value_types::BOOL, proton::Signal::SignalCase::kBoolValue},
      {value_types::STRING, proton::Signal::SignalCase::kStringValue},
      {value_types::BYTES, proton::Signal::SignalCase::kBytesValue},
      {value_types::LIST_DOUBLE, proton::Signal::SignalCase::kListDoubleValue},
      {value_types::LIST_FLOAT, proton::Signal::SignalCase::kListFloatValue},
      {value_types::LIST_INT32, proton::Signal::SignalCase::kListInt32Value},
      {value_types::LIST_INT64, proton::Signal::SignalCase::kListInt64Value},
      {value_types::LIST_UINT32, proton::Signal::SignalCase::kListUint32Value},
      {value_types::LIST_UINT64, proton::Signal::SignalCase::kListUint64Value},
      {value_types::LIST_BOOL, proton::Signal::SignalCase::kListBoolValue},
      {value_types::LIST_STRING, proton::Signal::SignalCase::kListStringValue},
      {value_types::LIST_BYTES, proton::Signal::SignalCase::kListBytesValue}
  };
}

namespace transport_types {
  inline static const std::string UDP4 = "udp4";
  inline static const std::string SERIAL = "serial";
}

struct SignalConfig {
  std::string name;
  std::string type_string;
  uint32_t length;
  uint32_t capacity;
  bool is_const;
  ::YAML::Node value;
};

struct BundleConfig {
  std::string name;
  uint32_t id;
  std::vector<std::string> producers;
  std::vector<std::string> consumers;
  std::vector<SignalConfig> signals;
};

struct EndpointConfig {
  std::string type;
  std::string device;
  std::string ip;
  uint32_t port;
};

struct HeartbeatConfig {
  bool enabled;
  uint32_t period;
};

struct NodeConfig {
  std::string name;
  HeartbeatConfig heartbeat;
  std::map<uint32_t, EndpointConfig> endpoints;
};

struct ConnectionEndpointConfig {
  uint32_t id;
  std::string node;
};

struct ConnectionConfig {
  std::pair<ConnectionEndpointConfig, ConnectionEndpointConfig> connection;
};

class Config {
public:
  Config();
  Config(std::string file);

  std::vector<BundleConfig> getBundles() { return bundles_; }
  std::map<std::string, NodeConfig>& getNodes() {
    std::shared_lock lock(mutex_);
    return nodes_;
  }

  std::vector<ConnectionConfig>& getConnections() {
    std::shared_lock lock(mutex_);
    return connections_;
  }

  std::string getName() { return name_; }
  ::YAML::Node getYamlNode() { return yaml_node_; }

private:
  std::vector<BundleConfig> bundles_;
  std::map<std::string, NodeConfig> nodes_;
  std::vector<ConnectionConfig> connections_;
  std::string name_;
  ::YAML::Node yaml_node_;
  mutable std::shared_mutex mutex_;
};

} // namespace proton


#endif // INC_PROTONCPP_CONFIG_HPP_
