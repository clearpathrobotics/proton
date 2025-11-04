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
  std::unique_lock lock(bundle_mutex_);
  bundles_.emplace(config.name, BundleHandle(config));
}

void BundleManager::addHeartbeat(std::string producer, std::string consumer) {
  BundleConfig config = {
    .name = producer,
    .id = 0x0,
    .producer = producer,
    .consumer = consumer
  };

  std::unique_lock lock(heartbeat_mutex_);
  heartbeat_bundles_.emplace(config.name, BundleHandle(config));
}

BundleHandle &BundleManager::getBundle(const std::string &bundle_name) {
  std::shared_lock lock(bundle_mutex_);
  try {
    return bundles_.at(bundle_name);
  } catch (std::out_of_range &e) {
    throw std::runtime_error("Invalid bundle name " + bundle_name);
  }
}

BundleHandle &BundleManager::getHeartbeat(const std::string &producer) {
  std::shared_lock lock(heartbeat_mutex_);
  try {
    return heartbeat_bundles_.at(producer);
  } catch (std::out_of_range &e) {
    throw std::runtime_error("Invalid producer " + producer);
  }
}

std::map<std::string, BundleHandle>& BundleManager::getBundleMap()
{
  std::shared_lock lock(bundle_mutex_);
  return bundles_;
}

BundleHandle &BundleManager::receiveBundle(const uint8_t *buffer,
                                           const uint32_t len,
                                           const std::string& producer) {
  auto bundle = Bundle();
  bundle.ParseFromArray(buffer, len);
  return setBundle(bundle, producer);
}

BundleHandle &BundleManager::setBundle(const Bundle &bundle, const std::string& producer) {
  // Heartbeat bundles have a length of 0
  if (bundle.ByteSizeLong() == 0)
  {
    for (auto &[name, handle] : heartbeat_bundles_) {
      if (handle.getName() == producer) {
        // Nothing to set, return handle
        return handle;
      }
    }
  }
  else
  {
    for (auto &[name, handle] : bundles_) {
      if (handle.getId() == bundle.id()) {
        {
          std::unique_lock lock(bundle_mutex_);
          handle.setBundle(bundle);
        }
        return handle;
      }
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
