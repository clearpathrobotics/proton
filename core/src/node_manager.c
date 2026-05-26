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

#include "proton/node_manager.h"
#include "proton/encode_decode.h"

/**
 * Encode a bundle for sending from a node. Updates the bundle metadata in the registry
 * @NOTE the bundle ID should be set for this bundle in the registry before calling the function
 * Parameters:
 * - node: the node sending the bundle, used to access the registry and destination peer information
 * - slot_id: the index of the bundle in the registry, used to update the bundle metadata. This is looked up
 *   from the bundle ID for efficiency, since we already have the bundle descriptor from the registry lookup
 * - uptime_ms: the current uptime in milliseconds, used to update the bundle metadata for prioritization
 * - buffer: the buffer to encode the bundle into
 * - buffer_len: the length of the buffer
 * - out_len: output parameter for the number of bytes encoded into the buffer
 * - dest_peers: output parameter for the list of destination peers to send this bundle to
 * - num_dest_peers: the number of destination peers available in the dest_peers buffer
 * - num_selected_peers: output parameter for the number of peers selected for this bundle (should be >= num_dest_peers)
 * @return status of the operation
 */
static proton_status_e proton_node_encode_bundle_desc(
  proton_core_node_t * node, size_t slot_id, uint64_t uptime_ms, uint8_t * buffer,
  size_t buffer_len, size_t * out_len, proton_endpoint_t * dest_peers, size_t num_dest_peers,
  size_t * num_selected_peers)
{
  bundle_desc_t * bundle_handle = &node->registry->bundle_table[slot_id];

  // Check the target endpoint buffer is big enough for the endpoints for this bundle
  if (bundle_handle->consumer_ids.count > num_dest_peers)
  {
    return PROTON_INSUFFICIENT_BUFFER_ERROR;
  }

  size_t dest_idx = 0;
  for (size_t i = 0; i < bundle_handle->consumer_ids.count; i++)
  {
    for (size_t j = 0; j < node->num_peers; j++)
    {
      if (bundle_handle->consumer_ids.ids[i] == node->destination_peers[j].node_id)
      {
        proton_endpoint_t * ep = &dest_peers[dest_idx];
        ep->node_id = bundle_handle->consumer_ids.ids[i];
        ep->transport_type = node->destination_peers[j].transport_type;
        ep->endpoint_id = node->destination_peers[j].endpoint_id;
        dest_idx++;
        break;
      }
    }
  }
  *num_selected_peers = bundle_handle->consumer_ids.count;

  bundle_handle->last_send_ms = uptime_ms;
  bundle_handle->send_now = false;

  return proton_encode_bundle(
    node->registry, bundle_handle->bundle_id, buffer, buffer_len, out_len);
}

proton_status_e proton_node_receive(proton_core_node_t * node, const uint8_t * buffer, size_t len)
{
  if (node == NULL || node->registry == NULL || buffer == NULL)
  {
    return PROTON_NULL_PTR_ERROR;
  }

  proton_Proton msg = proton_Proton_init_default;

  proton_status_e decode_result = proton_decode(node->registry, buffer, len, &msg);

  if (decode_result != PROTON_OK)
  {
    return decode_result;
  }

  if (msg.which_operation == proton_Proton_bundle_tag)
  {
    size_t slot_id;
    uint32_t bundle_id = msg.operation.bundle.id;
    const bundle_desc_t * bundle_desc =
      proton_registry_get_bundle(node->registry, bundle_id, &slot_id);

    if (bundle_desc == NULL)
    {
      // If the bundle ID is not found, then this is likely not a bundle for this target
      return PROTON_INCORRECT_TARGET_ERROR;
    }

    // Get the bundle callback directly from the slot ID to save time
    proton_bundle_cb_t * callback_desc = &node->registry->bundle_callbacks[slot_id];

    if (callback_desc != NULL && callback_desc->cb != NULL)
    {
      callback_desc->cb(
        bundle_id, bundle_desc->signal_ids.ids, bundle_desc->signal_ids.count, callback_desc->arg);
    }
  }
  // No unsupported operation check here, handled in proton_decode

  return PROTON_OK;
}

proton_status_e proton_node_update(
  proton_core_node_t * node, uint64_t uptime_ms, uint8_t * buffer, size_t buffer_len,
  size_t * out_len, proton_endpoint_t * dest_peers, size_t num_dest_peers,
  size_t * num_selected_peers)
{
  if (
    node == NULL || node->registry == NULL || buffer == NULL || out_len == NULL ||
    dest_peers == NULL || num_selected_peers == NULL)
  {
    return PROTON_NULL_PTR_ERROR;
  }

  if (num_dest_peers == 0)
  {
    return PROTON_INSUFFICIENT_BUFFER_ERROR;
  }

  bool something_to_send = false;

  uint32_t bundle_to_send = 0;
  uint64_t most_overdue_ms = 0;
  bool send_now_flag = false;
  size_t slot_id = 0;

  // Mark any bundles in the pending trigger buffer to be sent now
  while (node->trigger_tail != node->trigger_head)
  {
    node->registry->bundle_table[node->pending_triggers[node->trigger_tail]].send_now = true;
    node->trigger_tail = (node->trigger_tail + 1) % PROTON_MAX_PENDING_TRIGGERS;
  }

  for (size_t i = 0; i < node->registry->bundle_count; i++)
  {
    bundle_desc_t * bundle_desc = &node->registry->bundle_table[i];
    // Check triggered bundles
    if (bundle_desc->send_now)
    {
      // If this is our first triggered bundle, we will skip looking at non-triggered bundles (via send_now_flag)
      if (!send_now_flag)
      {
        send_now_flag = true;
        bundle_to_send = bundle_desc->bundle_id;
        most_overdue_ms = (uptime_ms - bundle_desc->last_send_ms) - bundle_desc->period_ms;
        slot_id = i;
        something_to_send = true;
      }
      // Check if this triggered bundle is more overdue than our current candidate triggered bundle
      else if ((uptime_ms - bundle_desc->last_send_ms) - bundle_desc->period_ms >= most_overdue_ms)
      {
        bundle_to_send = bundle_desc->bundle_id;
        most_overdue_ms = (uptime_ms - bundle_desc->last_send_ms) - bundle_desc->period_ms;
        slot_id = i;
        something_to_send = true;
      }
    }
    // No triggered bundles, check non-triggered bundles for the most overdue bundle to send
    else if (!send_now_flag)
    {
      if (
        bundle_desc->period_ms != 0 &&
        uptime_ms - bundle_desc->last_send_ms >= bundle_desc->period_ms)
      {
        if ((uptime_ms - bundle_desc->last_send_ms) - bundle_desc->period_ms >= most_overdue_ms)
        {
          bundle_to_send = bundle_desc->bundle_id;
          most_overdue_ms = (uptime_ms - bundle_desc->last_send_ms) - bundle_desc->period_ms;
          slot_id = i;
          something_to_send = true;
        }
      }
    }
  }

  if (something_to_send)
  {
    // We have our priority bundle, encode it
    return proton_node_encode_bundle_desc(
      node, slot_id, uptime_ms, buffer, buffer_len, out_len, dest_peers, num_dest_peers,
      num_selected_peers);
  }

  return PROTON_OK;
}

proton_status_e proton_node_trigger_bundle(proton_core_node_t * node, uint32_t bundle_id)
{
  if (node == NULL || node->registry == NULL)
  {
    return PROTON_NULL_PTR_ERROR;
  }

  size_t slot_id;
  const bundle_desc_t * bundle_desc =
    proton_registry_get_bundle(node->registry, bundle_id, &slot_id);
  if (bundle_desc == NULL)
  {
    return PROTON_INCORRECT_TARGET_ERROR;
  }

  size_t next_head = (node->trigger_head + 1) % PROTON_MAX_PENDING_TRIGGERS;
  if (next_head == node->trigger_tail)
  {
    // Trigger buffer is full, cannot accept new triggers
    return PROTON_INSUFFICIENT_BUFFER_ERROR;
  }

  node->pending_triggers[node->trigger_head] = slot_id;
  node->trigger_head = next_head;

  return PROTON_OK;
}

proton_status_e proton_node_encode_bundle(
  proton_core_node_t * node, uint32_t bundle_id, uint64_t uptime_ms, uint8_t * buffer,
  size_t buffer_len, size_t * out_len, proton_endpoint_t * dest_peers, size_t num_dest_peers,
  size_t * num_selected_peers)
{
  if (
    node == NULL || node->registry == NULL || buffer == NULL || out_len == NULL ||
    dest_peers == NULL || num_selected_peers == NULL)
  {
    return PROTON_NULL_PTR_ERROR;
  }

  if (num_dest_peers == 0)
  {
    return PROTON_INSUFFICIENT_BUFFER_ERROR;
  }

  size_t slot_id;
  const bundle_desc_t * bundle_handle =
    proton_registry_get_bundle(node->registry, bundle_id, &slot_id);
  if (bundle_handle == NULL)
  {
    return PROTON_INCORRECT_TARGET_ERROR;
  }

  return proton_node_encode_bundle_desc(
    node, slot_id, uptime_ms, buffer, buffer_len, out_len, dest_peers, num_dest_peers,
    num_selected_peers);
}
