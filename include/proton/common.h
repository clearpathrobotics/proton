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

#ifndef INC_PROTON_COMMON_H_
#define INC_PROTON_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

// Max message size
#define PROTON_MAX_MESSAGE_SIZE UINT16_MAX

// Heartbeat bundle ID
#define PROTON_HEARTBEAT_ID 0

// Serial Framing
// [0x50][0x52][Length byte 0][Length byte 1][Payload][CRC16 byte 0][CRC16 byte 1]
#define PROTON_FRAME_HEADER_MAGIC_BYTE_0 (uint8_t)0x50 // Magic overhead byte 0 for serial synchronization
#define PROTON_FRAME_HEADER_MAGIC_BYTE_1 (uint8_t)0x52 // Magic overhead byte 1 for serial synchronization
#define PROTON_FRAME_HEADER_LENGTH_OVERHEAD sizeof(uint16_t) // Length is sent in 2 bytes as a uint16_t
#define PROTON_FRAME_CRC_OVERHEAD sizeof(uint16_t) // CRC is sent in 2 bytes as a uint16_t
#define PROTON_FRAME_HEADER_OVERHEAD sizeof(PROTON_FRAME_HEADER_MAGIC_BYTE_0) + sizeof(PROTON_FRAME_HEADER_MAGIC_BYTE_1) + PROTON_FRAME_HEADER_LENGTH_OVERHEAD
#define PROTON_FRAME_OVERHEAD PROTON_FRAME_HEADER_OVERHEAD + PROTON_FRAME_CRC_OVERHEAD

/**
 * @brief Proton return status enum
 *
 */
typedef enum {
  PROTON_OK, // Success
  PROTON_ERROR, // Generic error
  PROTON_NULL_PTR_ERROR, // Null pointer error
  PROTON_INVALID_STATE_ERROR, // Function called in an invalid state
  PROTON_INVALID_STATE_TRANSITION_ERROR, // Invalid state transition attempted
  PROTON_SERIALIZATION_ERROR, // Error serializing or deserialing protobuf
  PROTON_INVALID_HEADER_ERROR, // Invalid header received over serial
  PROTON_CONNECT_ERROR, // Error while trying to connect
  PROTON_DISCONNECT_ERROR, // Error while trying to disconnect
  PROTON_READ_ERROR, // Error while trying to read
  PROTON_WRITE_ERROR, // Error while trying to write
  PROTON_CRC16_ERROR, // CRC16 mismatch
  PROTON_MUTEX_ERROR, // Failed to lock or unlock mutex
  PROTON_INSUFFICIENT_BUFFER_ERROR, // Buffer is too small to fit required data
} proton_status_e;

/**
 * @brief Proton node state enum
 *
 */
typedef enum {
  PROTON_NODE_UNCONFIGURED, // Node is unconfigured
  PROTON_NODE_INACTIVE, // Node has been configured but has not started sending or receiving bundles
  PROTON_NODE_ACTIVE // Node is configured and actively sending and receiving bundles
} proton_node_state_e;

/**
 * @brief Proton transport state enum
 *
 */
typedef enum {
  PROTON_TRANSPORT_DISCONNECTED, // Transport is disconnected
  PROTON_TRANSPORT_CONNECTED, // Transport is connected
  PROTON_TRANSPORT_ERROR // Transport is in an error state
} proton_transport_state_e;

/**
 * @brief Calculate CRC16 given a data buffer and length
 *
 * @param data Data buffer
 * @param len Buffer length
 * @param crc16 Output CRC16
 * @return proton_status_e return status
 */
static inline proton_status_e proton_crc16(const uint8_t *data, uint16_t len, uint16_t *crc16) {
  if (!data || !crc16)
  {
    return PROTON_NULL_PTR_ERROR;
  }

  uint16_t crc = 0xFFFF;
  for (uint16_t i = 0; i < len; i++) {
    crc ^= (uint16_t)data[i] << 8;
    for (int j = 0; j < 8; j++) {
      if (crc & 0x8000)
        crc = (crc << 1) ^ 0x1021;
      else
        crc <<= 1;
    }
  }

  *crc16 = crc;
  return PROTON_OK;
}

/**
 * @brief Fill header of a serial frame
 *
 * @param header Pointer to start of header
 * @param payload_len Payload length
 * @return proton_status_e return status
 */
static inline proton_status_e proton_fill_frame_header(uint8_t *header, const uint16_t payload_len) {
  if (!header) {
    return PROTON_NULL_PTR_ERROR;
  }

  header[0] = PROTON_FRAME_HEADER_MAGIC_BYTE_0;
  header[1] = PROTON_FRAME_HEADER_MAGIC_BYTE_1;
  header[2] = (uint8_t)(payload_len & 0xFF);
  header[3] = (uint8_t)(payload_len >> 8);

  return PROTON_OK;
}

/**
 * @brief Fill CRC16 bytes of a serial frame
 *
 * @param payload Pointer to payload
 * @param payload_len Payload length
 * @param crc Pointer to start of CRC16
 * @return proton_status_e return status
 */
static inline proton_status_e proton_fill_crc16(const uint8_t *payload,
                                  const uint16_t payload_len,
                                  uint8_t *crc) {
  if (!payload || !crc) {
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

/**
 * @brief Check for a valid CRC16 in a framed payload
 *
 * @param payload Pointer to payload
 * @param payload_len Payload length
 * @param frame_crc Received CRC16
 * @return proton_status_e return status
 */
static inline proton_status_e proton_check_framed_payload(const uint8_t *payload,
                                            const uint16_t payload_len,
                                            const uint16_t frame_crc) {
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

/**
 * @brief Get payload length from a framed payload
 *
 * @param framed_buf Framed payload buffer
 * @param length Output length
 * @return proton_status_e return status
 */
static inline proton_status_e proton_get_framed_payload_length(const uint8_t *framed_buf, uint16_t *length) {
  if (!framed_buf || !length)
  {
    return PROTON_NULL_PTR_ERROR;
  }

  // Check that magic bytes are correct
  if (framed_buf[0] != PROTON_FRAME_HEADER_MAGIC_BYTE_0 ||
      framed_buf[1] != PROTON_FRAME_HEADER_MAGIC_BYTE_1) {
    return PROTON_INVALID_HEADER_ERROR;
  }

  // Calculate length
  *length = framed_buf[2] | (framed_buf[3] << 8);

  return PROTON_OK;
}

#ifdef __cplusplus
}
#endif

#endif // INC_PROTON_COMMON_H_
