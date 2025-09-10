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

#include "protoncpp/bundle_manager.hpp"

using namespace proton;

void BundleManager::addBundle(BundleConfig config) {
  bundles_.emplace(config.name, BundleHandle(config));
}

BundleHandle &BundleManager::getBundle(const std::string &bundle_name) {
  try {
    return bundles_.at(bundle_name);
  } catch (std::out_of_range &e) {
    throw std::runtime_error("Invalid bundle name " + bundle_name);
  }
}

BundleHandle &BundleManager::receiveBundle(const uint8_t *buffer,
                                           const uint32_t len) {
  auto bundle = Bundle();
  bundle.ParseFromArray(buffer, len);
  return setBundle(bundle);
}

BundleHandle &BundleManager::setBundle(const Bundle &bundle) {
  for (auto &[name, handle] : bundles_) {
    if (handle.getId() == bundle.id()) {
      *handle.getBundlePtr().get() = bundle;
      return handle;
    }
  }

  throw std::runtime_error("Invalid bundle received with ID " + bundle.id());
}

void BundleManager::printAllBundles() {
  for (auto &[name, handle] : bundles_) {
    handle.printBundle();
  }
}

void BundleManager::printAllBundlesVerbose() {
  for (auto &[name, handle] : bundles_) {
    handle.printBundleVerbose();
  }
}
