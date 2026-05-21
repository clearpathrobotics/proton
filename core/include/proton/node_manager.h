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

#ifdef __cplusplus
extern "C"
{
#endif

  typedef struct proton_peer
  {
    uint32_t id;
    proton_transport_type_e transport_type;
  } proton_peer_t;

  typedef struct proton_node
  {
    uint32_t id;
    proton_peer_t * peers;
    size_t num_peers;
    proton_registry_t * registry;
  } proton_node_t;

  proton_status_e proton_node_receive(proton_node_t * node, const uint8_t * buf, size_t len);
  proton_status_e proton_node_update(
    proton_node_t * node, uint64_t uptime_ms, uint8_t * buf, size_t * out_len,
    proton_peer_t * dest_peer);

#ifdef __cplusplus
}
#endif

#endif  // PROTON_NODE_MANAGER_H
