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

#include <cstdint>
#include <format>
#include <map>
#include <span>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

namespace proton::node_builder
{

template <typename T>
void find_duplicates(const std::span<T> & items, const std::string & name)
{
  std::unordered_set<T> seen;
  std::unordered_set<T> duplicates;

  for (const auto & i : items)
  {
    if (!seen.insert(i).second)
    {
      duplicates.insert(i);
    }
  }

  if (!duplicates.empty())
  {
    std::ostringstream oss;
    oss << "{";
    bool first = true;
    for (const auto & d : duplicates)
    {
      if (!first)
      {
        oss << ", ";
      }
      oss << d;
      first = false;
    }
    oss << "}";
    throw NodeBuilderException(std::format("Error: {} contains duplicates {}", name, oss.str()));
  }
}

void validate(const Config & config);
Config filter_for_target(const Config & config, const std::string & target_name);

class GeneratedNode
{
public:
  explicit GeneratedNode() = default;
  explicit GeneratedNode(const Config & config, const std::string & target_name);
  ~GeneratedNode() = default;

  // Non-copyable, move-only
  GeneratedNode(const GeneratedNode &) = delete;
  GeneratedNode & operator=(const GeneratedNode &) = delete;
  GeneratedNode(GeneratedNode &&) = default;
  GeneratedNode & operator=(GeneratedNode &&) = default;

  // Accessors
  proton_core_node_t * node() { return &node_; }
  const proton_core_node_t * node() const { return &node_; }
  proton_registry_t * registry() { return &registry_; }
  const proton_registry_t * registry() const { return &registry_; }

private:
  // Generation methods - called during construction
  void generate_endpoints(const Config & config, const std::string & target_name);
  void generate_signals(const Config & config);
  void generate_bundles(const Config & config);
  void init_registry();
  void init_node(const Config & config, const std::string & target_name);

  // Owned storage for endpoint peers
  std::vector<proton_endpoint_t> node_destination_peers_;

  // Owned storage for bundle ID lists (producer_ids, consumer_ids, signal_ids per bundle)
  std::map<uint32_t, std::vector<uint32_t>> bundle_producer_ids_;
  std::map<uint32_t, std::vector<uint32_t>> bundle_consumer_ids_;
  std::map<uint32_t, std::vector<uint32_t>> bundle_signal_ids_;

  // Owned storage for bundle descriptors
  std::vector<bundle_desc_t> bundle_table_;

  // Owned storage for bundle encode/decode signal pointers
  std::vector<proton_Signal> bundle_encode_decode_buffer_;
  std::vector<signal_desc_t> signal_registry_;

  // Owned storage for string/bytes signal value buffer (where actual values are stored)
  std::vector<std::vector<uint8_t>> signal_value_buffer_storage_;
  // Owned storage for string/bytes signal decode buffer (temporary decode space)
  std::vector<std::vector<uint8_t>> signal_decode_buffer_storage_;
  std::vector<uint8_t> signal_scratch_buffer_;

  // The actual node and registry structs (point into owned storage above)
  proton_core_node_t node_{};
  proton_registry_t registry_{};
};

}  // namespace proton::node_builder

#endif  // PROTON_NODE_BUILDER
#endif  // PROTON_NODE_BUILDER_GENERATOR_HPP
