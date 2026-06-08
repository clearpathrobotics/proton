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

#ifndef PROTON_TRANSPORT_UDP4_HPP
#define PROTON_TRANSPORT_UDP4_HPP

#include <cstdint>
#include "proton/transport.h"

#if __cplusplus >= 202002L
#include <span>
#endif

namespace proton::transport::udp4
{

using Header = proton_udp4_header_t;
using Version = proton_udp4_version_e;

inline constexpr uint8_t CURRENT_VERSION = PROTON_CURRENT_UDP_VERSION;

inline proton_status_e fill_header(Header & header, uint8_t node_id = 0, uint8_t flags = 0)
{
  return proton_udp4_fill_header(&header, node_id, flags);
}

inline proton_status_e check_payload(
  const uint8_t * payload, const uint16_t payload_len, Header & header)
{
  return proton_udp4_check_payload(payload, payload_len, &header);
}

#if __cplusplus >= 202002L

inline proton_status_e check_payload(std::span<const uint8_t> payload, Header & header)
{
  return check_payload(payload.data(), static_cast<uint16_t>(payload.size()), header);
}

#endif

}  // namespace proton::transport::udp4
#endif  // PROTON_TRANSPORT_UDP4_HPP
