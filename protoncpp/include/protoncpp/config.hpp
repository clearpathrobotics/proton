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
#include <string>
#include <vector>

#include "protoncpp/bundle.pb.h"
#include "protoncpp/signal.pb.h"

namespace proton {

namespace keys {
  static const char *const NODES = "nodes";
  static const char *const BUNDLES = "bundles";
  static const char *const NAME = "name";
  static const char *const TRANSPORT = "transport";
  static const char *const TYPE = "type";
  static const char *const IP = "ip";
  static const char *const PORT = "port";
  static const char *const DEVICE = "device";
  static const char *const LENGTH = "length";
  static const char *const CAPACITY = "capacity";
  static const char *const ID = "id";
  static const char *const PRODUCER = "producer";
  static const char *const CONSUMER = "consumer";
  static const char *const SIGNALS = "signals";
} // namespace keys

namespace types {
  inline static const std::string DOUBLE = "double";
  inline static const std::string FLOAT = "float";
  inline static const std::string INT32 = "int32";
  inline static const std::string INT64 = "int64";
  inline static const std::string UINT32 = "uint32";
  inline static const std::string UINT64 = "uint64";
  inline static const std::string BOOL = "bool";
  inline static const std::string STRING = "string";
  inline static const std::string BYTES = "bytes";
} // namespace types

namespace signal_map {
  const std::map<std::string, proton::Signal::SignalCase> ScalarSignalMap = {
      {types::DOUBLE, proton::Signal::SignalCase::kDoubleValue},
      {types::FLOAT, proton::Signal::SignalCase::kFloatValue},
      {types::INT32, proton::Signal::SignalCase::kInt32Value},
      {types::INT64, proton::Signal::SignalCase::kInt64Value},
      {types::UINT32, proton::Signal::SignalCase::kUint32Value},
      {types::UINT64, proton::Signal::SignalCase::kUint64Value},
      {types::BOOL, proton::Signal::SignalCase::kBoolValue},
      {types::STRING, proton::Signal::SignalCase::kStringValue},
      {types::BYTES, proton::Signal::SignalCase::kBytesValue}};

  const std::map<std::string, proton::Signal::SignalCase> ListSignalMap = {
      {types::DOUBLE, proton::Signal::SignalCase::kListDoubleValue},
      {types::FLOAT, proton::Signal::SignalCase::kListFloatValue},
      {types::INT32, proton::Signal::SignalCase::kListInt32Value},
      {types::INT64, proton::Signal::SignalCase::kListInt64Value},
      {types::UINT32, proton::Signal::SignalCase::kListUint32Value},
      {types::UINT64, proton::Signal::SignalCase::kListUint64Value},
      {types::BOOL, proton::Signal::SignalCase::kListBoolValue},
      {types::STRING, proton::Signal::SignalCase::kListStringValue}};
}

struct SignalConfig {
  std::string name;
  std::string bundle_name;
  std::string type_string;
  uint32_t length;
  uint32_t capacity;
};

struct BundleConfig {
  std::string name;
  uint32_t id;
  std::string producer;
  std::string consumer;
  std::vector<SignalConfig> signals;
};

class TransportConfig {
public:
  inline static const std::string TYPE_UDP4 = "udp4";
  inline static const std::string TYPE_SERIAL = "serial";

  TransportConfig() {}

  TransportConfig(std::string device)
  {
    type_ = TYPE_SERIAL;
    device_ = device;
  }

  TransportConfig(std::string ip, uint32_t port)
  {
    type_ = TYPE_UDP4;
    ip_ = ip;
    port_ = port;
  }

  std::string getType() { return type_; }

  std::string getDevice() {
    if (type_ == TYPE_SERIAL) {
      return device_;
    }
    return std::string();
  }

  std::string getIP() {
    if (type_ == TYPE_UDP4) {
      return ip_;
    }
    return std::string();
  }

  uint32_t getPort() {
    if (type_ == TYPE_UDP4) {
      return port_;
    }
    return 0;
  }

private:
  std::string type_;
  std::string device_;
  std::string ip_;
  uint32_t port_;
};

class NodeConfig {
public:
  NodeConfig() {}
  NodeConfig(std::string name, std::string device) : name_(name), transport_(device) {}
  NodeConfig(std::string name, std::string ip, uint32_t port) : name_(name), transport_(ip, port) {}

  std::string getName() { return name_; }
  TransportConfig& getTransport() { return transport_; }

private:
  std::string name_;
  TransportConfig transport_;
};

class Config {
public:
  Config();
  Config(std::string file);

  std::vector<BundleConfig> getBundles() { return bundles_; }
  std::vector<NodeConfig> getNodes() { return nodes_; }
  std::string getName() { return name_; }

private:
  std::vector<BundleConfig> bundles_;
  std::vector<NodeConfig> nodes_;
  std::string name_;
};

} // namespace proton

#endif // INC_PROTONCPP_CONFIG_HPP_
