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

#include "proton/transport/serial.h"

/**
 * @brief Calculate CRC16 given a data buffer and length
 *
 * @param data Data buffer
 * @param len Buffer length
 * @param crc16 Output CRC16
 * @return proton_status_e return status
 */
static inline proton_status_e proton_crc16(const uint8_t * data, uint16_t len, uint16_t * crc16)
{
  if (!data || !crc16)
  {
    return PROTON_NULL_PTR_ERROR;
  }

  uint16_t crc = 0xFFFF;
  for (uint16_t i = 0; i < len; i++)
  {
    crc ^= (uint16_t)data[i] << 8;
    for (int j = 0; j < 8; j++)
    {
      if (crc & 0x8000)
        crc = (crc << 1) ^ 0x1021;
      else
        crc <<= 1;
    }
  }

  *crc16 = crc;
  return PROTON_OK;
}

proton_status_e proton_serial_fill_frame_header(uint8_t * header, const uint16_t payload_len)
{
  if (!header)
  {
    return PROTON_NULL_PTR_ERROR;
  }

  header[0] = PROTON_FRAME_HEADER_MAGIC_BYTE_0;
  header[1] = PROTON_FRAME_HEADER_MAGIC_BYTE_1;
  header[2] = (uint8_t)(payload_len & 0xFF);
  header[3] = (uint8_t)(payload_len >> 8);

  return PROTON_OK;
}

proton_status_e proton_serial_fill_crc16(
  const uint8_t * payload, const uint16_t payload_len, uint8_t * crc)
{
  if (!payload || !crc)
  {
    return PROTON_NULL_PTR_ERROR;
  }

  uint16_t crc16;
  proton_status_e status = proton_crc16(payload, payload_len, &crc16);

  if (status == PROTON_OK)
  {
    crc[0] = (uint8_t)(crc16 & 0xFF);
    crc[1] = (uint8_t)(crc16 >> 8);
  }

  return status;
}

proton_status_e proton_serial_check_framed_payload(
  const uint8_t * payload, const uint16_t payload_len, const uint16_t frame_crc)
{
  if (!payload)
  {
    return PROTON_NULL_PTR_ERROR;
  }

  uint16_t crc;
  proton_status_e status = proton_crc16(payload, payload_len, &crc);

  if (status == PROTON_OK && crc != frame_crc)
  {
    // CRC16 mismatch
    return PROTON_CRC16_ERROR;
  }

  return status;
}

proton_status_e proton_serial_get_framed_payload_length(
  const uint8_t * framed_buf, uint16_t * length)
{
  if (!framed_buf || !length)
  {
    return PROTON_NULL_PTR_ERROR;
  }

  // Check that magic bytes are correct
  if (
    framed_buf[0] != PROTON_FRAME_HEADER_MAGIC_BYTE_0 ||
    framed_buf[1] != PROTON_FRAME_HEADER_MAGIC_BYTE_1)
  {
    return PROTON_INVALID_HEADER_ERROR;
  }

  // Calculate length
  *length = framed_buf[2] | (framed_buf[3] << 8);

  return PROTON_OK;
}
