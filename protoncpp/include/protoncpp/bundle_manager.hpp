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

#ifndef INC_PROTONCPP_BUNDLE_MANAGER_HPP_
#define INC_PROTONCPP_BUNDLE_MANAGER_HPP_

#include "protoncpp/bundle.hpp"
#include <shared_mutex>
#include <map>
#include <optional>

namespace proton {

class BundleManager {
public:
  void addBundle(BundleConfig config);
  void addHeartbeat(std::string producer, std::vector<std::string> consumers);
  BundleHandle &getBundle(const std::string &bundle_name);
  BundleHandle &getHeartbeat(const std::string &producer);
  std::map<std::string, BundleHandle> &getBundleMap();
  std::optional<std::string> updateBundle(const Bundle &bundle, const std::string& producer);

  void printAllBundles();
  void printAllBundlesVerbose();

protected:
  std::vector<std::string> nodes_;
  std::map<std::string, BundleHandle> bundles_;
  std::map<std::string, BundleHandle> heartbeat_bundles_;
  std::shared_mutex bundle_mutex_;
  std::shared_mutex heartbeat_mutex_;
};

} // namespace proton

#endif // INC_PROTONCPP_BUNDLE_MANAGER_HPP_
