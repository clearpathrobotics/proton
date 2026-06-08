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

#ifndef PROTON_TRANSPORT_SERIAL_HPP
#define PROTON_TRANSPORT_SERIAL_HPP

#include <cstddef>
#include <cstdint>
#include "proton/transport.h"

#if __cplusplus >= 202002L
#include <span>
#endif

namespace proton::transport::serial
{

static constexpr uint8_t FRAME_HEADER_MAGIC_BYTE_0 = PROTON_FRAME_HEADER_MAGIC_BYTE_0;
static constexpr uint8_t FRAME_HEADER_MAGIC_BYTE_1 = PROTON_FRAME_HEADER_MAGIC_BYTE_1;
static constexpr size_t FRAME_HEADER_LENGTH_OVERHEAD = PROTON_FRAME_HEADER_LENGTH_OVERHEAD;
static constexpr size_t FRAME_CRC_OVERHEAD = PROTON_FRAME_CRC_OVERHEAD;
static constexpr size_t FRAME_HEADER_OVERHEAD = PROTON_FRAME_HEADER_OVERHEAD;
static constexpr size_t FRAME_OVERHEAD = PROTON_FRAME_OVERHEAD;

inline proton_status_e fill_frame_header(uint8_t * header, const uint16_t payload_len)
{
  return proton_serial_fill_frame_header(header, payload_len);
}

inline proton_status_e fill_crc16(
  const uint8_t * payload, const uint16_t payload_len, uint8_t * crc)
{
  return proton_serial_fill_crc16(payload, payload_len, crc);
}

inline proton_status_e check_framed_payload(
  const uint8_t * payload, const uint16_t payload_len, const uint16_t frame_crc)
{
  return proton_serial_check_framed_payload(payload, payload_len, frame_crc);
}

inline proton_status_e get_framed_payload_length(const uint8_t * framed_buf, uint16_t * length)
{
  return proton_serial_get_framed_payload_length(framed_buf, length);
}

#if __cplusplus >= 202002L

inline proton_status_e fill_crc16(std::span<const uint8_t> payload, std::span<uint8_t> crc)
{
  return fill_crc16(payload.data(), static_cast<uint16_t>(payload.size()), crc.data());
}

inline proton_status_e check_framed_payload(
  std::span<const uint8_t> payload, const uint16_t frame_crc)
{
  return check_framed_payload(payload.data(), static_cast<uint16_t>(payload.size()), frame_crc);
}

inline proton_status_e get_framed_payload_length(
  std::span<const uint8_t> framed_buf, uint16_t & length)
{
  return get_framed_payload_length(framed_buf.data(), &length);
}

#endif

}  // namespace proton::transport::serial

#endif  // PROTON_TRANSPORT_SERIAL_HPP
