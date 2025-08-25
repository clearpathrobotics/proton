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

#ifndef INC_PROTON_PROTON_H_
#define INC_PROTON_PROTON_H_

#include "protonc/bundle.pb.h"
#include "pb.h"
#include "pb_decode.h"
#include "pb_encode.h"

typedef enum {
  PROTON_SIGNAL_TYPE_DOUBLE_VALUE,
  PROTON_SIGNAL_TYPE_FLOAT_VALUE,
  PROTON_SIGNAL_TYPE_INT32_VALUE,
  PROTON_SIGNAL_TYPE_INT64_VALUE,
  PROTON_SIGNAL_TYPE_UINT32_VALUE,
  PROTON_SIGNAL_TYPE_UINT64_VALUE,
  PROTON_SIGNAL_TYPE_BOOL_VALUE,
  PROTON_SIGNAL_TYPE_STRING_VALUE,
  PROTON_SIGNAL_TYPE_BYTES_VALUE,
  PROTON_SIGNAL_TYPE_LIST_DOUBLE_VALUE,
  PROTON_SIGNAL_TYPE_LIST_FLOAT_VALUE,
  PROTON_SIGNAL_TYPE_LIST_INT32_VALUE,
  PROTON_SIGNAL_TYPE_LIST_INT64_VALUE,
  PROTON_SIGNAL_TYPE_LIST_UINT32_VALUE,
  PROTON_SIGNAL_TYPE_LIST_UINT64_VALUE,
  PROTON_SIGNAL_TYPE_LIST_BOOL_VALUE,
  PROTON_SIGNAL_TYPE_LIST_STRING_VALUE,
} proton_signal_type_e;

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
  void *data;
  size_t capacity;
  size_t size;
} proton_list_t;

typedef struct {
  proton_Signal signal;
  proton_list_t arg;
} proton_signal_t;

typedef struct {
  proton_Bundle bundle;
  proton_list_t arg;
} proton_bundle_t;

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

#define proton_list_arg_init_default {NULL, 0, 0}

void PROTON_InitBundle(proton_bundle_t *bundle, uint32_t id,
                       proton_signal_t *signals, uint32_t signal_count);

void PROTON_InitNode(proton_node_t *node, proton_transport_t transport, proton_receive_t receive, proton_buffer_t read_buf, proton_buffer_t write_buf);

int PROTON_Encode(proton_bundle_t *bundle, uint8_t *buffer,
                  size_t buffer_length);
int PROTON_Decode(proton_bundle_t *bundle, const uint8_t *buffer,
                  const size_t buffer_length);
bool PROTON_DecodeId(uint32_t *id, const uint8_t *buffer, size_t buffer_length);

proton_status_e PROTON_Spin(proton_node_t *node);
proton_status_e PROTON_SpinOnce(proton_node_t *node);

void print_bundle(proton_Bundle bundle);
void print_signal(proton_Signal signal);

#endif // INC_PROTON_PROTON_H_
