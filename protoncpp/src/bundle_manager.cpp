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
#include <mutex>

#include <mutex>

using namespace proton;

void BundleManager::addBundle(BundleConfig config)
{
  std::unique_lock lock(bundle_mutex_);
  bundles_.emplace(config.name, BundleHandle(config));
}

BundleHandle & BundleManager::getBundle(const std::string & bundle_name)
{
  std::shared_lock lock(bundle_mutex_);
  try
  {
    return bundles_.at(bundle_name);
  }
  catch (std::out_of_range & e)
  {
    throw std::runtime_error("Invalid bundle name " + bundle_name);
  }
}

std::map<std::string, BundleHandle> & BundleManager::getBundleMap()
{
  std::shared_lock lock(bundle_mutex_);
  return bundles_;
}

std::optional<std::string> BundleManager::updateBundle(
  const Bundle & bundle, const std::string & producer)
{
  for (auto & [name, handle] : bundles_)
  {
    if (handle.getId() == bundle.id())
    {
      std::unique_lock lock(bundle_mutex_);
      handle.updateBundle(bundle);
      return handle.getName();
    }
  }

  std::cerr << "Invalid bundle received with ID " << bundle.id() << std::endl;
  return std::nullopt;
}

void BundleManager::printAllBundles()
{
  for (auto & [name, handle] : bundles_)
  {
    handle.printBundle();
  }
}

void BundleManager::printAllBundlesVerbose()
{
  for (auto & [name, handle] : bundles_)
  {
    handle.printBundleVerbose();
  }
}
