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
#include <map>

namespace proton {

class BundleManager {
public:
  void addBundle(BundleConfig config);
  BundleHandle &getBundle(const std::string &bundle_name);
  std::map<std::string, BundleHandle> &getBundleMap();
  BundleHandle &receiveBundle(const uint8_t *buffer, const uint32_t len);
  BundleHandle &setBundle(const Bundle &bundle);

  void printAllBundles();
  void printAllBundlesVerbose();

protected:
  std::map<std::string, BundleHandle> bundles_;
};

} // namespace proton

#endif // INC_PROTONCPP_BUNDLE_MANAGER_HPP_
