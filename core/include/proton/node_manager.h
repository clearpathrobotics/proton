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

#ifndef PROTON_NODE_MANAGER_H
#define PROTON_NODE_MANAGER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "proton/common.h"
#include "proton/registry.h"
#include "proton/transport.h"

#ifndef PROTON_MAX_PENDING_TRIGGERS
#define PROTON_MAX_PENDING_TRIGGERS 4
#endif

#ifdef __cplusplus
extern "C"
{
#endif

  typedef struct proton_endpoint
  {
    // It appears that nodes don't have ID's but their endpoints do
    // the endpoint ID's are not globally unique
    // To create a unique identifier for the node/endpoint combination, this is done with an
    // ID that is the index of the node in the nodes stanza, with the LSB being set with the endpoint ID
    uint32_t node_id;
    uint32_t endpoint_id;
    proton_transport_type_e transport_type;
  } proton_endpoint_t;

  typedef struct proton_core_node
  {
    uint32_t id;
    const proton_endpoint_t * destination_peers;
    size_t num_peers;
    proton_registry_t * registry;
    uint32_t pending_triggers[PROTON_MAX_PENDING_TRIGGERS];
    size_t trigger_head;
    size_t trigger_tail;
  } proton_core_node_t;  // TODO change to proton_node_t as part of protonc deprecation

  // Buffer here is post-transport-de-payloading. Presumably the user is handling where the data comes from, and
  // can de-payload it themselves using proton's framing API
  proton_status_e proton_node_receive(
    proton_core_node_t * node, const uint8_t * buffer, size_t len);

  // Output buffer here is also non-framed, but does specify which peers to send the buffer to
  proton_status_e proton_node_update(
    proton_core_node_t * node, uint64_t uptime_ms, uint8_t * buffer, size_t buffer_len,
    size_t * out_len, proton_endpoint_t * dest_peers, size_t num_dest_peers,
    size_t * num_selected_peers);

  // Set a bundle to be sent on the next update
  proton_status_e proton_node_trigger_bundle(proton_core_node_t * node, uint32_t bundle_id);

  proton_status_e proton_node_encode_bundle(
    proton_core_node_t * node, uint32_t bundle_id, uint64_t uptime_ms, uint8_t * buffer,
    size_t buffer_len, size_t * out_len, proton_endpoint_t * dest_peers, size_t num_dest_peers,
    size_t * num_selected_peers);

#ifdef __cplusplus
}
#endif

#endif  // PROTON_NODE_MANAGER_H
