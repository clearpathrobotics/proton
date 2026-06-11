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

#include "proton/node_manager.h"
#include "protoncpp/node_builder/config.hpp"

namespace proton::node_builder
{

Config filter_for_target(const Config & config, const std::string & target_name);

}  // namespace proton::node_builder

#endif  // PROTON_NODE_BUILDER
#endif  // PROTON_NODE_BUILDER_GENERATOR_HPP
