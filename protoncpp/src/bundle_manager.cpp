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
 */

#include "protoncpp/bundle_manager.hpp"

using namespace proton;

void BundleManager::addBundle(BundleConfig config) {
  std::unique_lock lock(bundle_mutex_);
  bundles_.emplace(config.name, BundleHandle(config));
}

void BundleManager::addHeartbeat(std::string producer, std::vector<std::string> consumers) {
  SignalConfig signal_config = {
    .name = "heartbeat",
    .type_string = std::string(value_types::UINT32.begin(), value_types::UINT32.end()),
    .is_const = false
  };

  BundleConfig config = {
    .name = producer,
    .id = 0,
    .producers = {producer},
    .consumers = consumers,
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
    try {
        return heartbeat_bundles_.at(bundle_name);
    } catch (std::out_of_range &e) {
      throw std::runtime_error("Invalid bundle name " + bundle_name);
    }
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

std::optional<std::string> BundleManager::updateBundle(const Bundle &bundle, const std::string& producer) {
  // Heartbeat bundles have an id of 0
  if (bundle.id() == 0)
  {
    // Heartbeat should have just one uint32_t signal
    if (bundle.signals_size() == 1 && bundle.signals(0).has_uint32_value())
    {
      for (auto &[name, handle] : heartbeat_bundles_) {
        if (handle.getName() == producer) {
          std::unique_lock lock(heartbeat_mutex_);
          handle.updateBundle(bundle);
          return handle.getName();
        }
      }
    }
    else
    {
      std::cerr << "Invalid heartbeat received" << std::endl;
      return std::nullopt;
    }
  }
  else
  {
    for (auto &[name, handle] : bundles_) {
      if (handle.getId() == bundle.id()) {
        std::unique_lock lock(bundle_mutex_);
        handle.updateBundle(bundle);
        return handle.getName();
      }
    }
  }

  std::cerr << "Invalid bundle received with ID " << bundle.id() << std::endl;
  return std::nullopt;
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
