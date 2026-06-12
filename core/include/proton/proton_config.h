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

#ifndef PROTON_CONFIG_HPP
#define PROTON_CONFIG_HPP

// Default to embedded mode (no allocation/RTTI) if not specified
#ifndef PROTON_ENABLE_ALLOC
#define PROTON_ENABLE_ALLOC 0
#endif

#ifndef PROTON_MAX_PENDING_TRIGGERS
#define PROTON_MAX_PENDING_TRIGGERS 4
#endif

#ifndef PROTON_NODE_BUILDER
#define PROTON_NODE_BUILDER 0
#endif

#ifndef PROTON_NODE_BUILDER_YAML_PARSER
#define PROTON_NODE_BUILDER_YAML_PARSER 0
#endif

#ifndef PROTON_NODE_BUILDER_JSON_PARSER
#define PROTON_NODE_BUILDER_JSON_PARSER 0
#endif

#endif  // PROTON_CONFIG_HPP
