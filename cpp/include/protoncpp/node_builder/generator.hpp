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
 * @author Tom Wallis (thomas.wallis@rockwellautomation.com)
 */

#ifndef PROTON_NODE_BUILDER_GENERATOR_HPP
#define PROTON_NODE_BUILDER_GENERATOR_HPP

#include "proton/proton_config.h"

#if PROTON_NODE_BUILDER

#include "protoncpp/node_builder/config.hpp"

#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include <yaml-cpp/yaml.h>

namespace proton::node_builder
{

class NodeGenerator
{
public:
  NodeGenerator(const std::string & file, const std::string & target_name);
  virtual ~NodeGenerator() = default;

  proton_status_e validate_for_target();
  proton_core_node_t generate_node();

private:
  std::vector<BundleConfig> bundles_;
  std::map<std::string, NodeConfig> nodes_;
  std::vector<ConnectionConfig> connections_;
  std::string name_;
  YAML::Node yaml_node_;
};

}  // namespace proton::node_builder

#endif  // PROTON_NODE_BUILDER
#endif  // PROTON_NODE_BUILDER_GENERATOR_HPP
