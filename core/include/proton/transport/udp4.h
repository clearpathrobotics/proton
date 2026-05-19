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

#ifndef PROTON_TRANSPORT_UDP4_H
#define PROTON_TRANSPORT_UDP4_H

#include <stdint.h>
#include <stdlib.h>

#include "proton/common.h"

#ifdef __cplusplus
extern "C"
{
#endif

  typedef enum
  {
    // NOTE: udp4 version 1 (protonc/cpp) has no header, and should be ignored
    UDP4_VERSION_1 = 0,
    UDP4_VERSION_2 = 1,
  } proton_udp4_version_e;

#define PROTON_CURRENT_UDP_VERSION ((uint8_t)UDP4_VERSION_2)

  /**
 * @struct proton_udp4_header_t scaffolding for any future protocol changes that may impact how we encode/decode
 */
  typedef struct proton_udp4_header
  {
    // Use the proton_udp4_version_e enum
    uint8_t version;
    // 0 = unspecified, left up to the user to differentiate based on where the message came from
    uint8_t node_id;
    // Scaffolding for flags that may be needed for future versions
    uint8_t flags;
    // Reserved for future revisions
    uint8_t reserved;
  } proton_udp4_header_t;

  /**
   * @brief Populate header to the current version with session information
   */
  proton_status_e proton_udp4_fill_header(
    proton_udp4_header_t * header, uint8_t node_id, uint8_t flags);

  /**
   * @brief Check for the presence of a header in the proton payload
   * @note proton UDP4 version 1 does not have a header. If no header is parsable, say it's a version 1
   */
  proton_status_e proton_udp4_check_payload(
    const uint8_t * payload, const uint16_t payload_len, proton_udp4_header_t * out_header);

#ifdef __cplusplus
}
#endif

#endif  // PROTON_TRANSPORT_UDP4_H
