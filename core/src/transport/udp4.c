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

#include "proton/transport/udp4.h"
#include <stdbool.h>
#include <string.h>

static bool check_udp4_version(uint8_t read_version)
{
  switch (read_version)
  {
    case (UDP4_VERSION_2):
      return true;

    case (UDP4_VERSION_1):
    default:
      return false;
  }
}

proton_status_e proton_udp4_fill_header(
  proton_udp4_header_t * header, uint8_t node_id, uint8_t flags)
{
  if (header == NULL)
  {
    return PROTON_NULL_PTR_ERROR;
  }

  header->version = PROTON_CURRENT_UDP_VERSION;
  header->node_id = node_id;
  header->flags = flags;
  header->reserved = 0;

  return PROTON_OK;
}

proton_status_e proton_udp4_check_payload(
  const uint8_t * payload, const uint16_t payload_len, proton_udp4_header_t * out_header)
{
  if (payload == NULL || out_header == NULL)
  {
    return PROTON_NULL_PTR_ERROR;
  }

  if (payload_len < sizeof(proton_udp4_header_t))
  {
    return PROTON_INSUFFICIENT_BUFFER_ERROR;
  }

  uint8_t udp4_version = payload[0];
  if (check_udp4_version(udp4_version))
  {
    memcpy(out_header, payload, sizeof(proton_udp4_header_t));
  }
  else
  {
    out_header->version = UDP4_VERSION_1;
    out_header->flags = 0;
    out_header->node_id = 0;
    out_header->reserved = 0;
  }

  return PROTON_OK;
}
