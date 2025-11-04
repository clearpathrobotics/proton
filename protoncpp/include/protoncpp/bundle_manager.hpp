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

#ifndef INC_PROTONCPP_BUNDLE_MANAGER_HPP_
#define INC_PROTONCPP_BUNDLE_MANAGER_HPP_

#include "protoncpp/bundle.hpp"
#include <shared_mutex>
#include <map>

namespace proton {

class BundleManager {
public:
  void addBundle(BundleConfig config);
  void addHeartbeat(std::string producer, std::string consumer);
  BundleHandle &getBundle(const std::string &bundle_name);
  BundleHandle &getHeartbeat(const std::string &producer);
  std::map<std::string, BundleHandle> &getBundleMap();
  BundleHandle &receiveBundle(const uint8_t *buffer, const uint32_t len, const std::string& producer);
  BundleHandle &setBundle(const Bundle &bundle, const std::string& producer);

  void printAllBundles();
  void printAllBundlesVerbose();

protected:
  std::vector<std::string> nodes_;
  std::map<std::string, BundleHandle> bundles_;
  std::map<std::string, BundleHandle> heartbeat_bundles_;
  mutable std::shared_mutex bundle_mutex_;
  mutable std::shared_mutex heartbeat_mutex_;
};

} // namespace proton

#endif // INC_PROTONCPP_BUNDLE_MANAGER_HPP_
