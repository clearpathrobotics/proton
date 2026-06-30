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
 * @author Roni Kreinin (roni.kreinin@rockwellautomation.com)
 */

#ifndef PROTON_TRANSPORT_SERIAL_H
#define PROTON_TRANSPORT_SERIAL_H

#include <stdbool.h>
#include <stdint.h>

#include "proton/common.h"

#ifdef __cplusplus
extern "C"
{
#endif

// Serial Framing
// [0x50][0x52][Length byte 0][Length byte 1][Payload][CRC16 byte 0][CRC16 byte 1]
#define PROTON_FRAME_HEADER_MAGIC_BYTE_0 \
  (uint8_t)0x50  // Magic overhead byte 0 for serial synchronization
#define PROTON_FRAME_HEADER_MAGIC_BYTE_1 \
  (uint8_t)0x52  // Magic overhead byte 1 for serial synchronization
#define PROTON_FRAME_HEADER_LENGTH_OVERHEAD \
  sizeof(uint16_t)                                  // Length is sent in 2 bytes as a uint16_t
#define PROTON_FRAME_CRC_OVERHEAD sizeof(uint16_t)  // CRC is sent in 2 bytes as a uint16_t
#define PROTON_FRAME_HEADER_OVERHEAD                                                     \
  (sizeof(PROTON_FRAME_HEADER_MAGIC_BYTE_0) + sizeof(PROTON_FRAME_HEADER_MAGIC_BYTE_1) + \
   PROTON_FRAME_HEADER_LENGTH_OVERHEAD)
#define PROTON_FRAME_OVERHEAD (PROTON_FRAME_HEADER_OVERHEAD + PROTON_FRAME_CRC_OVERHEAD)

  /**
   * @brief Fill header of a serial frame
   *
   * @param header Pointer to start of header
   * @param payload_len Payload length
   * @return proton_status_e return status
   */
  proton_status_e proton_serial_fill_frame_header(uint8_t * header, const uint16_t payload_len);

  /**
   * @brief Fill CRC16 bytes of a serial frame
   *
   * @param payload Pointer to payload
   * @param payload_len Payload length
   * @param crc Pointer to start of CRC16
   * @return proton_status_e return status
   */
  proton_status_e proton_serial_fill_crc16(
    const uint8_t * payload, const uint16_t payload_len, uint8_t * crc);

  /**
   * @brief Check for a valid CRC16 in a framed payload
   *
   * @param payload Pointer to payload
   * @param payload_len Payload length
   * @param frame_crc Received CRC16
   * @return proton_status_e return status
   */
  proton_status_e proton_serial_check_framed_payload(
    const uint8_t * payload, const uint16_t payload_len, const uint16_t frame_crc);

  /**
   * @brief Get payload length from a framed payload
   *
   * @param framed_buf Framed payload buffer
   * @param length Output length
   * @return proton_status_e return status
   */
  proton_status_e proton_serial_get_framed_payload_length(
    const uint8_t * framed_buf, uint16_t * length);

#ifdef __cplusplus
}
#endif

#endif  // PROTON_TRANSPORT_SERIAL_H
