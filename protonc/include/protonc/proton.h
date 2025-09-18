/**
 * Software License Agreement (proprietary)
 *
 * @copyright Copyright (c) 2025 Clearpath Robotics, Inc., All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, is not permitted without the express permission of Clearpath
 * Robotics.
 *
 * @author Roni Kreinin (rkreinin@clearpathrobotics.com)
 */

#ifndef INC_PROTONC_PROTON_H_
#define INC_PROTONC_PROTON_H_

#include "protonc/bundle.pb.h"
#include "pb.h"
#include "pb_decode.h"
#include "pb_encode.h"

#define PROTON_FRAME_HEADER_MAGIC_BYTE_0 (uint8_t)0x50
#define PROTON_FRAME_HEADER_MAGIC_BYTE_1 (uint8_t)0x52
#define PROTON_FRAME_HEADER_LENGTH_OVERHEAD sizeof(uint16_t)
#define PROTON_FRAME_CRC_OVERHEAD sizeof(uint16_t)
#define PROTON_FRAME_HEADER_OVERHEAD sizeof(PROTON_FRAME_HEADER_MAGIC_BYTE_0) + sizeof(PROTON_FRAME_HEADER_MAGIC_BYTE_1) + PROTON_FRAME_HEADER_LENGTH_OVERHEAD
#define PROTON_FRAME_OVERHEAD PROTON_FRAME_HEADER_OVERHEAD + PROTON_FRAME_CRC_OVERHEAD


typedef enum {
  PROTON_OK = 0,
  PROTON_ERR = 1,
  PROTON_READ_ERR = 2,
} proton_status_e;

typedef void (*proton_callback_t)(void);
typedef bool (*proton_transport_connect_t)(void);
typedef bool (*proton_transport_disconnect_t)(void);
typedef size_t (*proton_transport_read_t)(uint8_t *buf, size_t len);
typedef size_t (*proton_transport_write_t)(const uint8_t *buf, size_t len);
typedef bool (*proton_receive_t)(const uint8_t * buf, size_t len);

typedef struct {
  void *data; // Pointer to start of data buffer
  size_t length; // Length of list
  size_t capacity; // Capacity of data buffer (strings and bytes only)
  size_t size; // Current size of list (for decoding only)
} proton_list_t;

typedef struct {
  proton_Signal signal;
  proton_list_t arg;
} proton_signal_handle_t;

typedef struct {
  proton_Bundle bundle;
  proton_list_t arg;
} proton_bundle_handle_t;

typedef struct {
  proton_transport_connect_t connect;
  proton_transport_disconnect_t disconnect;
  proton_transport_read_t read;
  proton_transport_write_t write;
} proton_transport_t;

typedef struct {
  uint8_t * data;
  size_t len;
} proton_buffer_t;

typedef struct {
  proton_transport_t transport;
  bool connected;
  proton_receive_t receive;
  proton_buffer_t read_buf;
  proton_buffer_t write_buf;
} proton_node_t;

#define proton_list_arg_init_default {NULL, 0, 0, 0}

void PROTON_InitBundle(proton_bundle_handle_t *handle, uint32_t id,
                       proton_signal_handle_t *signal_handles, uint32_t signal_count);

void PROTON_InitNode(proton_node_t *node, proton_transport_t transport, proton_receive_t receive, proton_buffer_t read_buf, proton_buffer_t write_buf);

int PROTON_Encode(proton_bundle_handle_t *handle, uint8_t *buffer,
                  size_t buffer_length);
int PROTON_Decode(proton_bundle_handle_t *handle, const uint8_t *buffer,
                  const size_t buffer_length);
bool PROTON_DecodeId(uint32_t *id, const uint8_t *buffer, size_t buffer_length);

proton_status_e PROTON_Spin(proton_node_t *node);
proton_status_e PROTON_SpinOnce(proton_node_t *node);

uint16_t PROTON_CRC16(const uint8_t *data, uint16_t len);
bool PROTON_FillFrameHeader(uint8_t * header, uint16_t payload_len);
bool PROTON_FillCRC16(const uint8_t * payload, const uint16_t payload_len, uint8_t * crc);
bool PROTON_CheckFramedPayload(const uint8_t *payload, const size_t payload_len, const uint16_t frame_crc);
uint16_t PROTON_GetFramedPayloadLength(const uint8_t * framed_buf);

void PROTON_PrintBundle(proton_Bundle bundle);
void PROTON_PrintSignal(proton_Signal signal);

#endif // INC_PROTONC_PROTON_H_
