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

#ifndef INC_PROTONCPP_MANAGER_HPP_
#define INC_PROTONCPP_MANAGER_HPP_

#include "protoncpp/bundle.pb.h"
#include "protoncpp/config.hpp"
#include <map>
#include <memory>

namespace proton {

using bytes = std::vector<uint8_t>;

class SignalHandle {
public:
  SignalHandle(proton::SignalConfig config, std::shared_ptr<proton::Signal> ptr, int idx);

  std::string name;
  std::string bundle_name;
  proton::Signal::SignalCase type;
  uint32_t length;
  uint32_t capacity;
  int index; // position in bundle.signals()
  std::shared_ptr<proton::Signal> signal;

  bool valid() const { return index >= 0; }

  // Primary template (not defined -> unsupported types will fail to compile)
  template <typename T> T getValue();
  template <typename T> void setValue(T value);
};

class BundleHandle {
public:
  BundleHandle(proton::BundleConfig config);

  void addSignal(const std::string &, proton::SignalConfig config);
  SignalHandle &getSignal(const std::string &signal_name);

  void printBundle();

  std::string name;
  uint32_t id;
  std::string producer;
  std::string consumer;
  std::shared_ptr<proton::Bundle> bundle;
  std::map<std::string, SignalHandle> signals_;
};

class BundleManager {
public:
  void addBundle(proton::BundleConfig config);
  BundleHandle &getBundle(const std::string &bundle_name);
  BundleHandle &receiveBundle(const uint8_t * buffer, const uint32_t len);

  BundleHandle &setBundle(const proton::Bundle &bundle);

  void printAllBundles();


protected:
  proton::Bundle *getBundlePtr(const std::string &bundle_name);
  std::map<std::string, BundleHandle> bundles_;
};

} // namespace proton

#endif // INC_PROTONCPP_MANAGER_HPP_
