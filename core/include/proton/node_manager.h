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

  /**
   * Description of a peer endpoint to send messages to
   * This is because the node manager is transport agnostic, but it needs to instruct
   * the user which bundle to send to which peer, and the transport information is
   * defined at the node level since it's a property of the peer, not the bundle
   */
  typedef struct proton_endpoint
  {
    uint32_t node_id;
    uint32_t endpoint_id;
    proton_transport_type_e transport_type;
  } proton_endpoint_t;

  /**
   * Top-level struct for proton interaction, this is the main struct that users will interact with
   * to send and receive bundles. It contains a pointer to the registry, as well as information about
   * each peer this node can send messages to.
   */
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

  /**
   * Receive a message for a node, decode it, and update the registry and bundle callbacks as necessary
   * The input buffer is expected to be a non-framed protobuf message, since framing is
   * transport-specific and the node manager is transport-agnostic. The user is expected to handle any framing
   * based on the transport API.
   *
   * This function will decode the message, update the signal registry with new information,
   * and call the relevant bundle callback if a bundle is successfully decoded.
   */
  proton_status_e proton_node_receive(
    proton_core_node_t * node, const uint8_t * buffer, size_t len);

  /**
   * Update function to be called periodically by the user to check if there are any messages to send
   * This function will encode bundles by a priority scheme:
   *   - "triggered" bundles (see proton_node_trigger_bundle) are prioritized over non-triggered bundles
   *   - "most overdue" bundles are prioritized over less overdue bundles
   *   - in the event of no overdue bundles, bundles with older last-send timestamps are prioritized over newer ones.
   * The priority order is essentially as follows:
   *   - "most overdue" triggered bundles
   *   - "most overdue" non-triggered bundles
   */
  proton_status_e proton_node_update(
    proton_core_node_t * node, uint64_t uptime_ms, uint8_t * buffer, size_t buffer_len,
    size_t * out_len, proton_endpoint_t * dest_peers, size_t num_dest_peers,
    size_t * num_selected_peers);

  /**
   * Set a bundle ID to be sent at the next available node update, according to priority rules.
   */
  proton_status_e proton_node_trigger_bundle(proton_core_node_t * node, uint32_t bundle_id);

  /**
   * Encode a bundle by ID and write it to the provided buffer
   */
  proton_status_e proton_node_encode_bundle(
    proton_core_node_t * node, uint32_t bundle_id, uint64_t uptime_ms, uint8_t * buffer,
    size_t buffer_len, size_t * out_len, proton_endpoint_t * dest_peers, size_t num_dest_peers,
    size_t * num_selected_peers);

#ifdef __cplusplus
}
#endif

#endif  // PROTON_NODE_MANAGER_H
