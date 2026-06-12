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

#ifndef PROTON_CORE_TESTS_CORE_UTILS_HPP
#define PROTON_CORE_TESTS_CORE_UTILS_HPP

#include <cstdlib>
#include <cstring>
#include "proton/node_manager.h"
#include "proton/registry.h"

#ifdef __cplusplus
extern "C"
{
#endif

  proton_registry_t copy_default_registry(proton_registry_t * original_registry)
  {
    proton_registry_t copy = *original_registry;
    // Deep copy the signal registry since we will be modifying signal values in some tests
    signal_desc_t * signal_registry_copy =
      (signal_desc_t *)malloc(sizeof(signal_desc_t) * copy.signal_count);
    memcpy(
      signal_registry_copy, original_registry->signal_registry,
      sizeof(signal_desc_t) * copy.signal_count);
    copy.signal_registry = signal_registry_copy;

    // Deep copy bundle table (includes callbacks and mutable state like last_send_ms)
    bundle_desc_t * bundle_table_copy =
      (bundle_desc_t *)malloc(sizeof(bundle_desc_t) * copy.bundle_count);
    memcpy(
      bundle_table_copy, original_registry->bundle_table,
      sizeof(bundle_desc_t) * copy.bundle_count);
    copy.bundle_table = bundle_table_copy;

    return copy;
  }

  proton_core_node_t copy_default_node(proton_core_node_t * original_node)
  {
    proton_core_node_t copy = *original_node;
    if (original_node->num_peers > 0)
    {
      proton_endpoint_t * peers_copy =
        (proton_endpoint_t *)malloc(sizeof(proton_endpoint_t) * original_node->num_peers);
      memcpy(
        peers_copy, original_node->destination_peers,
        sizeof(proton_endpoint_t) * original_node->num_peers);
      copy.destination_peers = peers_copy;
    }
    return copy;
  }

#ifdef __cplusplus
}
#endif

class BundleCallback
{
public:
  BundleCallback() = default;
  virtual ~BundleCallback() = default;

  virtual void callback(uint32_t bundle_id, const uint32_t * signal_ids, size_t num_ids) = 0;

protected:
  static void bundle_cb(
    uint32_t bundle_id, const uint32_t * signal_ids, size_t num_ids, void * context)
  {
    reinterpret_cast<BundleCallback *>(context)->callback(bundle_id, signal_ids, num_ids);
  }
};

#endif  // PROTON_CORE_TESTS_CORE_UTILS_HPP
