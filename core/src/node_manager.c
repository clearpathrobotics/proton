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

  uint32_t bundle_to_send = 0;
  uint64_t oldest_timestamp = UINT64_MAX;
  bool send_now_flag = false;
  size_t slot_id = 0;
  for (size_t i = 0; i < node->registry->bundle_count; i++)
  {
    // Prioritize in the following manner:
    // 1. Bundles marked to be sent now, prioritize the oldest send time
    // 2. If no send-now flags, send the oldest bundle
    bundle_desc_t * bundle_desc = &node->registry->bundle_table[i];
    // If this is our first send-now flag, the priority timestamp becomes the send-now timestamp
    if (bundle_desc->send_now)
    {
      if (!send_now_flag)
      {
        send_now_flag = true;
        bundle_to_send = bundle_desc->bundle_id;
        oldest_timestamp = bundle_desc->last_send_ms;
        slot_id = i;
      }
      else if (bundle_desc->last_send_ms <= oldest_timestamp)
      {
        bundle_to_send = bundle_desc->bundle_id;
        oldest_timestamp = bundle_desc->last_send_ms;
        slot_id = i;
      }
    }
    else if (!send_now_flag)
    {
      if (bundle_desc->last_send_ms <= oldest_timestamp)
      {
        bundle_to_send = bundle_desc->bundle_id;
        oldest_timestamp = bundle_desc->last_send_ms;
        slot_id = i;
      }
    }
  }

  // We have our priority bundle, encode it
  bundle_desc_t * priority_bundle = &node->registry->bundle_table[slot_id];
  // Need to edit bundle_desc to support endpoint ID's
  priority_bundle->send_now = false;
  priority_bundle->last_send_ms = uptime_ms;

  // Check the target endpoint buffer is big enough for the endpoints for this bundle
  if (priority_bundle->consumer_ids.count > num_dest_peers)
  {
    return PROTON_INSUFFICIENT_BUFFER_ERROR;
  }

  for (size_t i = 0; i < priority_bundle->consumer_ids.count; i++)
  {
    proton_endpoint_t * ep = &dest_peers[i];
    ep->id = priority_bundle->consumer_ids.ids[i];
    for (size_t j = 0; j < node->num_peers; j++)
    {
      if (priority_bundle->consumer_ids.ids[i] == node->destination_peers[j].id)
      {
        ep->transport_type = node->destination_peers[j].transport_type;
        break;
      }
    }
  }
  *num_selected_peers = priority_bundle->consumer_ids.count;

  return proton_encode_bundle(node->registry, bundle_to_send, buffer, buffer_len, out_len);
}
