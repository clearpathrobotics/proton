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

#ifndef PROTON_NODE_ACCESS_HPP
#define PROTON_NODE_ACCESS_HPP

#include <cstddef>
#include <cstdint>
#include "proton/node_manager.h"

#if __cplusplus >= 202002L
#include <span>
#endif

namespace proton
{

static constexpr size_t MAX_PENDING_TRIGGERS = PROTON_MAX_PENDING_TRIGGERS;

class NodeAccess
{
public:
  using Endpoint = proton_endpoint_t;

  explicit NodeAccess(proton_core_node_t * node) : node_(node) {}

  proton_status_e receive(const uint8_t * buffer, size_t len)
  {
    return proton_node_receive(node_, buffer, len);
  }

  proton_status_e update(
    uint64_t uptime_ms, uint8_t * buffer, size_t buffer_len, size_t & out_len,
    Endpoint * dest_peers, size_t num_dest_peers, size_t & num_selected_peers)
  {
    return proton_node_update(
      node_, uptime_ms, buffer, buffer_len, &out_len, dest_peers, num_dest_peers,
      &num_selected_peers);
  }

  proton_status_e trigger_bundle(uint32_t bundle_id)
  {
    return proton_node_trigger_bundle(node_, bundle_id);
  }

  proton_status_e encode_bundle(
    uint32_t bundle_id, uint64_t uptime_ms, uint8_t * buffer, size_t buffer_len, size_t & out_len,
    Endpoint * dest_peers, size_t num_dest_peers, size_t & num_selected_peers)
  {
    return proton_node_encode_bundle(
      node_, bundle_id, uptime_ms, buffer, buffer_len, &out_len, dest_peers, num_dest_peers,
      &num_selected_peers);
  }

#if __cplusplus >= 202002L

  proton_status_e receive(std::span<const uint8_t> buffer)
  {
    return receive(buffer.data(), buffer.size());
  }

  proton_status_e update(
    uint64_t uptime_ms, std::span<uint8_t> buffer, size_t & out_len, std::span<Endpoint> peers,
    size_t & num_selected_peers)
  {
    return update(
      uptime_ms, buffer.data(), buffer.size(), out_len, peers.data(), peers.size(),
      num_selected_peers);
  }

  proton_status_e encode_bundle(
    uint32_t bundle_id, uint64_t uptime_ms, std::span<uint8_t> buffer, size_t & out_len,
    std::span<Endpoint> peers, size_t & num_selected_peers)
  {
    return encode_bundle(
      bundle_id, uptime_ms, buffer.data(), buffer.size(), out_len, peers.data(), peers.size(),
      num_selected_peers);
  }

#endif

private:
  proton_core_node_t * node_;
};

}  // namespace proton

#endif  // PROTON_NODE_ACCESS_HPP
