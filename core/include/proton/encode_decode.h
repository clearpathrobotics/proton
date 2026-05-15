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

#ifndef PROTON_ENCODE_DECODE_H
#define PROTON_ENCODE_DECODE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "proton/common.h"
#include "proton/registry.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /**
   * Encode a bundle from the registry into a Proton top-level message
   */
  proton_status_e proton_encode_bundle(
    proton_registry_t * registry, uint32_t bundle_id, uint8_t * buffer, size_t buffer_len,
    size_t * bytes_encoded);

  /**
   * Decode a Proton message from a buffer
   * If the message is a bundle, the registry will be updated with the decoded signals
   */
  proton_status_e proton_decode(proton_registry_t * registry, uint8_t * buffer, size_t buffer_len);

#ifdef __cplusplus
}
#endif

#endif  // PROTON_ENCODE_DECODE_H
