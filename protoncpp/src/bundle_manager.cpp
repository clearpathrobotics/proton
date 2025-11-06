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
  SignalConfig signal_config = {
    .name = "heartbeat",
    .type_string = std::string(value_types::UINT32.begin(), value_types::UINT32.end()),
    .is_const = false
  };

  BundleConfig config = {
    .name = producer,
    .id = 0,
    .producer = producer,
    .consumer = consumer,
    .signals = std::vector<SignalConfig>({signal_config})
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

BundleHandle &BundleManager::setBundle(const Bundle &bundle, const std::string& producer) {
  // Heartbeat bundles have an id of 0
  if (bundle.id() == 0)
  {
    // Heartbeat should have just one uint32_t signal
    if (bundle.signals_size() == 1 && bundle.signals(0).has_uint32_value())
    {
      for (auto &[name, handle] : heartbeat_bundles_) {
        if (handle.getName() == producer) {
          std::unique_lock lock(heartbeat_mutex_);
          handle.setBundle(bundle);
          return handle;
        }
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
