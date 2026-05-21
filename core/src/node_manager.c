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
  proton_core_node_t * node, uint64_t uptime_ms, uint8_t * buffer, size_t * out_len,
  proton_endpoint_t * dest_peers, size_t num_dest_peers, size_t * num_selected_peers)
{
}
