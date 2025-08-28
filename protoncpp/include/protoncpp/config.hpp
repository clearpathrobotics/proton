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

class SignalConfig {
public:
  inline static const std::string TYPE_DOUBLE = "double";
  inline static const std::string TYPE_FLOAT = "float";
  inline static const std::string TYPE_INT32 = "int32";
  inline static const std::string TYPE_INT64 = "int64";
  inline static const std::string TYPE_UINT32 = "uint32";
  inline static const std::string TYPE_UINT64 = "uint64";
  inline static const std::string TYPE_BOOL = "bool";
  inline static const std::string TYPE_STRING = "string";
  inline static const std::string TYPE_BYTES = "bytes";

  const std::map<std::string, proton::Signal::SignalCase> SignalMap = {
      {TYPE_DOUBLE, proton::Signal::SignalCase::kDoubleValue},
      {TYPE_FLOAT, proton::Signal::SignalCase::kFloatValue},
      {TYPE_INT32, proton::Signal::SignalCase::kInt32Value},
      {TYPE_INT64, proton::Signal::SignalCase::kInt64Value},
      {TYPE_UINT32, proton::Signal::SignalCase::kUint32Value},
      {TYPE_UINT64, proton::Signal::SignalCase::kUint64Value},
      {TYPE_BOOL, proton::Signal::SignalCase::kBoolValue},
      {TYPE_STRING, proton::Signal::SignalCase::kStringValue},
      {TYPE_BYTES, proton::Signal::SignalCase::kBytesValue}};

  const std::map<std::string, proton::Signal::SignalCase> ListSignalMap = {
      {TYPE_DOUBLE, proton::Signal::SignalCase::kListDoubleValue},
      {TYPE_FLOAT, proton::Signal::SignalCase::kListFloatValue},
      {TYPE_INT32, proton::Signal::SignalCase::kListInt32Value},
      {TYPE_INT64, proton::Signal::SignalCase::kListInt64Value},
      {TYPE_UINT32, proton::Signal::SignalCase::kListUint32Value},
      {TYPE_UINT64, proton::Signal::SignalCase::kListUint64Value},
      {TYPE_BOOL, proton::Signal::SignalCase::kListBoolValue},
      {TYPE_STRING, proton::Signal::SignalCase::kListStringValue}};

  SignalConfig();
  SignalConfig(std::string name, std::string bundle_name, std::string type, uint32_t length,
               uint32_t capacity);

  std::string getName() { return name_; }
  std::string getBundleName() { return bundle_name_; }
  std::string getTypeString() { return type_string_; }
  uint32_t getLength() { return length_; }
  uint32_t getCapacity() { return capacity_; }

private:
  std::string name_;
  std::string bundle_name_;
  std::string type_string_;
  uint32_t length_;
  uint32_t capacity_;
};

class BundleConfig {
public:
  BundleConfig();
  BundleConfig(std::string name, uint32_t id, std::string producer,
               std::string consumer, std::vector<SignalConfig> signals);

  std::string getName() { return name_; }
  uint32_t getID() { return id_; }
  std::string getProducer() { return producer_; }
  std::string getConsumer() { return consumer_; }
  std::vector<SignalConfig> getSignals()
  {
    return signals_;
  }

private:
  std::string name_;
  uint32_t id_;
  std::string producer_;
  std::string consumer_;
  std::vector<SignalConfig> signals_;
};

class Config {
public:
  Config();
  Config(std::string file);

  std::vector<BundleConfig> getBundles() { return bundles_; }

private:
  std::vector<BundleConfig> bundles_;
};

} // namespace proton

#endif // INC_PROTONCPP_CONFIG_HPP_
