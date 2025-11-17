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

#define PROTON_BUNDLE__HEARTBEAT 0 // Heartbeat bundle ID
#define PROTON_FRAME_HEADER_MAGIC_BYTE_0 (uint8_t)0x50
#define PROTON_FRAME_HEADER_MAGIC_BYTE_1 (uint8_t)0x52
#define PROTON_FRAME_HEADER_LENGTH_OVERHEAD sizeof(uint16_t)
#define PROTON_FRAME_CRC_OVERHEAD sizeof(uint16_t)
#define PROTON_FRAME_HEADER_OVERHEAD sizeof(PROTON_FRAME_HEADER_MAGIC_BYTE_0) + sizeof(PROTON_FRAME_HEADER_MAGIC_BYTE_1) + PROTON_FRAME_HEADER_LENGTH_OVERHEAD
#define PROTON_FRAME_OVERHEAD PROTON_FRAME_HEADER_OVERHEAD + PROTON_FRAME_CRC_OVERHEAD

typedef enum {
  PROTON_OK,
  PROTON_ERROR,
  PROTON_NULL_PTR_ERROR,
  PROTON_INVALID_STATE_ERROR,
  PROTON_INVALID_STATE_TRANSITION_ERROR,
  PROTON_CONNECTION_ERROR,
  PROTON_SERIALIZATION_ERROR,
  PROTON_READ_ERROR,
  PROTON_WRITE_ERROR,
  PROTON_CRC16_ERROR,
  PROTON_MUTEX_ERROR,
} proton_status_e;

typedef enum {
  PROTON_NODE_UNCONFIGURED,
  PROTON_NODE_INACTIVE,
  PROTON_NODE_ACTIVE
} proton_node_state_e;

typedef enum {
  PROTON_TRANSPORT_DISCONNECTED,
  PROTON_TRANSPORT_CONNECTED,
  PROTON_TRANSPORT_ERROR
} proton_transport_state_e;

typedef void (*proton_callback_t)(void);
typedef bool (*proton_transport_connect_t)(void);
typedef bool (*proton_transport_disconnect_t)(void);
typedef size_t (*proton_transport_read_t)(uint8_t *buf, size_t len);
typedef size_t (*proton_transport_write_t)(const uint8_t *buf, size_t len);
typedef proton_status_e (*proton_receive_t)(const uint8_t * buf, size_t len);
typedef bool (*proton_mutex_lock_t)();
typedef bool (*proton_mutex_unlock_t)();

typedef uint64_t proton_producer_t;
typedef uint64_t proton_consumer_t;
typedef uint64_t proton_peer_id_t;

typedef struct {
  size_t length; // Length of list
  size_t capacity; // Capacity of data buffer (strings and bytes only)
  size_t size; // Current size of list (for decoding only)
  void *data; // Pointer to start of data buffer
} proton_list_t;

typedef struct {
  proton_Signal signal;
  proton_list_t arg;
} proton_signal_handle_t;

typedef struct {
  proton_Bundle bundle;
  proton_list_t arg;
  proton_producer_t producers;
  proton_consumer_t consumers;
} proton_bundle_handle_t;

typedef struct {
  uint8_t * data;
  size_t len;
} proton_buffer_t;

typedef struct {
  proton_buffer_t buffer;
  proton_mutex_lock_t lock;
  proton_mutex_unlock_t unlock;
} proton_atomic_buffer_t;

typedef struct {
  proton_transport_state_e state;
  proton_transport_connect_t connect;
  proton_transport_disconnect_t disconnect;
  proton_transport_read_t read;
  proton_transport_write_t write;
} proton_transport_t;

typedef struct {
  bool enabled;
  uint32_t period;
} proton_heartbeat_t;

typedef struct {
  proton_node_state_e state;
  proton_peer_id_t id;
  proton_heartbeat_t heartbeat;
  proton_transport_t transport;
  proton_receive_t receive;
  proton_atomic_buffer_t atomic_buffer;
  const char * name;
} proton_peer_t;

typedef struct {
  proton_node_state_e state;
  proton_heartbeat_t heartbeat;
  proton_atomic_buffer_t atomic_buffer;
  uint16_t peer_count;
  proton_peer_t * peers;
  const char * name;
} proton_node_t;

#define proton_list_arg_init_default {NULL, 0, 0, 0}
#define proton_buffer_default {NULL, 0}
#define proton_transport_default {PROTON_TRANSPORT_DISCONNECTED, NULL, NULL, NULL, NULL}
#define proton_heartbeat_default {false, 0}
#define proton_atomic_buffer_default {proton_buffer_default, NULL, NULL}

#define proton_peer_default(name) {PROTON_NODE_UNCONFIGURED, 0, proton_heartbeat_default, proton_transport_default, NULL, proton_atomic_buffer_default, name}
#define proton_node_default(name) {PROTON_NODE_UNCONFIGURED, proton_heartbeat_default, proton_atomic_buffer_default, 0, NULL, name}

#define TRANSPORT_VALID(transport) (transport.connect != NULL && transport.disconnect != NULL && transport.read != NULL && transport.write != NULL)

proton_status_e PROTON_InitBundle(proton_bundle_handle_t *handle,
                                  uint32_t id,
                                  proton_signal_handle_t *signal_handles,
                                  uint32_t signal_count,
                                  proton_producer_t producers,
                                  proton_consumer_t consumers);

proton_status_e PROTON_InitPeer(proton_peer_t * peer,
                                proton_peer_id_t id,
                                proton_heartbeat_t heartbeat,
                                proton_transport_t transport,
                                proton_receive_t receive_func,
                                proton_mutex_lock_t lock_func,
                                proton_mutex_unlock_t unlock_func,
                                proton_buffer_t read_buf);

proton_status_e PROTON_Configure(proton_node_t * node,
                                 proton_heartbeat_t heartbeat,
                                 proton_mutex_lock_t lock_func,
                                 proton_mutex_unlock_t unlock_func,
                                 proton_buffer_t write_buf,
                                 proton_peer_t * peers,
                                 uint16_t peer_count);

proton_status_e PROTON_Activate(proton_node_t * node);

proton_status_e PROTON_Encode(proton_node_t * node, proton_bundle_handle_t *handle, size_t *bytes_encoded);
proton_status_e PROTON_Decode(proton_bundle_handle_t *handle, proton_peer_t * peer, size_t length);
proton_status_e PROTON_DecodeId(uint32_t *id, proton_peer_t * peer);

proton_status_e PROTON_Spin(proton_node_t *node, const uint8_t peer);
proton_status_e PROTON_SpinOnce(proton_node_t *node, const uint8_t peer);

proton_status_e PROTON_CRC16(const uint8_t *data, uint16_t len, uint16_t *crc16);
proton_status_e PROTON_FillFrameHeader(uint8_t * header, uint16_t payload_len);
proton_status_e PROTON_FillCRC16(const uint8_t * payload, const uint16_t payload_len, uint8_t * crc);
proton_status_e PROTON_CheckFramedPayload(const uint8_t *payload, const size_t payload_len, const uint16_t frame_crc);
proton_status_e PROTON_GetFramedPayloadLength(const uint8_t * framed_buf, uint16_t *length);

void PROTON_PrintBundle(proton_Bundle bundle);
void PROTON_PrintSignal(proton_Signal signal);

#endif // INC_PROTONC_PROTON_H_
