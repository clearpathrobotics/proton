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

#include "protonc/proton.h"
#include <stdio.h>

proton_status_e proton_init_bundle(proton_bundle_handle_t *handle,
                                   uint32_t id,
                                   proton_signal_handle_t *signal_handles,
                                   uint32_t signal_count,
                                   proton_producer_t producers,
                                   proton_consumer_t consumers) {
  if (signal_handles && handle) {
    handle->signals.data = signal_handles;
    handle->signals.length = signal_count;
    handle->signals.size = 0;
    handle->bundle.id = id;
    handle->bundle.signals = &handle->signals;
    handle->producers = producers;
    handle->consumers = consumers;
    return PROTON_OK;
  }

  return PROTON_NULL_PTR_ERROR;
}

proton_status_e proton_init_signal(proton_signal_handle_t * handle,
                                   pb_size_t which_signal,
                                   void * data,
                                   size_t length,
                                   size_t capacity)
{
  if (!handle || !data)
  {
    return PROTON_NULL_PTR_ERROR;
  }

  switch(which_signal)
  {
    case proton_Signal_double_value_tag:
    case proton_Signal_float_value_tag:
    case proton_Signal_int32_value_tag:
    case proton_Signal_int64_value_tag:
    case proton_Signal_uint32_value_tag:
    case proton_Signal_uint64_value_tag:
    case proton_Signal_bool_value_tag:
    {
      break;
    }

    case proton_Signal_string_value_tag:
    {
      handle->signal.signal.string_value = &handle->value;
      break;
    }

    case proton_Signal_bytes_value_tag:
    {
      handle->signal.signal.bytes_value = &handle->value;
      break;
    }

    case proton_Signal_list_double_value_tag:
    {
      handle->signal.signal.list_double_value.doubles = &handle->value;
      break;
    }

    case proton_Signal_list_float_value_tag:
    {
      handle->signal.signal.list_float_value.floats = &handle->value;
      break;
    }

    case proton_Signal_list_int32_value_tag:
    {
      handle->signal.signal.list_int32_value.int32s = &handle->value;
      break;
    }

    case proton_Signal_list_int64_value_tag:
    {
      handle->signal.signal.list_int64_value.int64s = &handle->value;
      break;
    }

    case proton_Signal_list_uint32_value_tag:
    {
      handle->signal.signal.list_uint32_value.uint32s = &handle->value;
      break;
    }

    case proton_Signal_list_uint64_value_tag:
    {
      handle->signal.signal.list_uint64_value.uint64s = &handle->value;
      break;
    }

    case proton_Signal_list_bool_value_tag:
    {
      handle->signal.signal.list_bool_value.bools = &handle->value;
      break;
    }

    case proton_Signal_list_string_value_tag:
    {
      handle->signal.signal.list_string_value.strings = &handle->value;
      break;
    }

    case proton_Signal_list_bytes_value_tag:
    {
      handle->signal.signal.list_bytes_value.bytes = &handle->value;
      break;
    }

    default:
    {
      PROTON_PRINT("Invalid signal type %d", which_signal);
      return PROTON_ERROR;
    }
  }

  handle->signal.which_signal = which_signal;
  handle->value.data = data;
  handle->value.length = length;
  handle->value.capacity = capacity;
  handle->value.size = 0;

  return PROTON_OK;
}

proton_status_e proton_init_peer(proton_peer_t * peer,
                                proton_peer_id_t id,
                                proton_heartbeat_t heartbeat,
                                proton_transport_t transport,
                                proton_receive_t receive_func,
                                proton_mutex_lock_t lock_func,
                                proton_mutex_unlock_t unlock_func,
                                proton_buffer_t read_buf)
{
  if (peer && TRANSPORT_VALID(transport) && receive_func && lock_func && unlock_func && read_buf.data)
  {
    if (heartbeat.enabled && heartbeat.period == 0)
    {
      return PROTON_ERROR;
    }

    peer->id = id;
    peer->heartbeat = heartbeat;
    peer->transport = transport;
    peer->receive = receive_func;
    peer->atomic_buffer.lock = lock_func;
    peer->atomic_buffer.unlock = unlock_func;
    peer->atomic_buffer.buffer = read_buf;
    peer->state = PROTON_NODE_INACTIVE;

    return PROTON_OK;
  }

  return PROTON_NULL_PTR_ERROR;
}

proton_status_e proton_configure(proton_node_t * node,
                                 proton_heartbeat_t heartbeat,
                                 proton_mutex_lock_t lock_func,
                                 proton_mutex_unlock_t unlock_func,
                                 proton_buffer_t write_buf,
                                 proton_peer_t * peers,
                                 uint16_t peer_count) {
  if (node && lock_func && unlock_func && write_buf.data && peers && peer_count > 0) {
    node->peer_count = peer_count;
    node->peers = peers;
    node->heartbeat = heartbeat;
    node->atomic_buffer.buffer = write_buf;
    node->atomic_buffer.lock = lock_func;
    node->atomic_buffer.unlock = unlock_func;
    node->state = PROTON_NODE_INACTIVE;

    for (uint16_t p = 0; p < peer_count; p++)
    {
      if (peers[p].state == PROTON_NODE_UNCONFIGURED)
      {
        return PROTON_INVALID_STATE_ERROR;
      }

      peers[p].transport.state = PROTON_TRANSPORT_DISCONNECTED;
    }

    return PROTON_OK;
  }

  return PROTON_NULL_PTR_ERROR;
}

proton_status_e proton_activate(proton_node_t *node)
{
  if (node == NULL)
  {
    return PROTON_NULL_PTR_ERROR;
  }

  if (node->state == PROTON_NODE_UNCONFIGURED)
  {
    return PROTON_INVALID_STATE_ERROR;
  }

  node->state = PROTON_NODE_ACTIVE;
  return PROTON_OK;
}

proton_status_e proton_encode(proton_bundle_handle_t * handle, proton_buffer_t buffer, size_t *bytes_encoded) {
  if (handle == NULL || handle->signals.data == NULL || buffer.data == NULL || bytes_encoded == NULL)
  {
    return PROTON_NULL_PTR_ERROR;
  }

  proton_signal_handle_t *signal_handle;

  // Copy non-list field values from struct to signal
  for (uint8_t i = 0; i < handle->signals.length; i++) {
    signal_handle = &((proton_signal_handle_t *)handle->signals.data)[i];

    switch (signal_handle->signal.which_signal) {

    case proton_Signal_double_value_tag: {
      signal_handle->signal.signal.double_value =
          *((double *)signal_handle->value.data);
      break;
    }

    case proton_Signal_float_value_tag: {
      signal_handle->signal.signal.float_value =
          *((float *)signal_handle->value.data);
      break;
    }

    case proton_Signal_int32_value_tag: {
      signal_handle->signal.signal.int32_value =
          *((int32_t *)signal_handle->value.data);
      break;
    }

    case proton_Signal_int64_value_tag: {
      signal_handle->signal.signal.int64_value =
          *((int64_t *)signal_handle->value.data);
      break;
    }

    case proton_Signal_uint32_value_tag: {
      signal_handle->signal.signal.uint32_value =
          *((uint32_t *)signal_handle->value.data);
      break;
    }

    case proton_Signal_uint64_value_tag: {
      signal_handle->signal.signal.uint64_value =
          *((uint64_t *)signal_handle->value.data);
      break;
    }

    case proton_Signal_bool_value_tag: {
      signal_handle->signal.signal.bool_value =
          *((bool *)signal_handle->value.data);
      break;
    }

    default:
      break;
    }
  }

  // Create stream from atomic buffer
  pb_ostream_t stream =
      pb_ostream_from_buffer((pb_byte_t *)buffer.data, buffer.len);
  // Encode bundle into stream
  bool status = pb_encode(&stream, proton_Bundle_fields, &handle->bundle);

  if (status) {
    *bytes_encoded = stream.bytes_written;
    return PROTON_OK;
  } else {
    PROTON_PRINT("Encode error: %s\r\n", stream.errmsg);
    return PROTON_SERIALIZATION_ERROR;
  }
}

proton_status_e proton_decode_id(proton_buffer_t buffer, uint32_t *id) {
  if (id == NULL || buffer.data == NULL) {
    return PROTON_NULL_PTR_ERROR;
  }

  pb_wire_type_t wire_type;
  uint32_t tag;
  bool eof;
  proton_status_e status = PROTON_SERIALIZATION_ERROR;

  pb_istream_t stream =
      pb_istream_from_buffer((const pb_byte_t *)buffer.data, buffer.len);

  if (pb_decode_tag(&stream, &wire_type, &tag, &eof)) {
    if (tag == proton_Bundle_id_tag) {
      if (pb_decode_varint32(&stream, id)) {
        status = PROTON_OK;
      }
    }
    else if (tag == proton_Bundle_signals_tag) {
      // No id tag but signals tag exists means ID is 0
      *id = 0;
      status = PROTON_OK;
    }
  }

  if (status != PROTON_OK)
  {
    PROTON_PRINT("DecodeId error: %s\r\n", stream.errmsg);
  }

  return status;
}

proton_status_e proton_decode(proton_bundle_handle_t *handle, proton_buffer_t buffer, size_t length) {
  if (handle == NULL || handle->signals.data == NULL || buffer.data == NULL)
  {
    return PROTON_NULL_PTR_ERROR;
  }

  pb_istream_t stream =
      pb_istream_from_buffer((const pb_byte_t *)buffer.data, length);

  bool status = pb_decode(&stream, proton_Bundle_fields, &handle->bundle);

  if (status) {
    proton_signal_handle_t *signal_handle;

    for (uint8_t i = 0; i < handle->signals.length; i++) {
      signal_handle = &((proton_signal_handle_t *)handle->signals.data)[i];
      switch (signal_handle->signal.which_signal) {
      case proton_Signal_double_value_tag: {
        *((double *)signal_handle->value.data) =
            signal_handle->signal.signal.double_value;
        break;
      }

      case proton_Signal_float_value_tag: {
        *((float *)signal_handle->value.data) =
            signal_handle->signal.signal.float_value;
        break;
      }

      case proton_Signal_int32_value_tag: {
        *((int32_t *)signal_handle->value.data) =
            signal_handle->signal.signal.int32_value;
        break;
      }

      case proton_Signal_int64_value_tag: {
        *((int64_t *)signal_handle->value.data) =
            signal_handle->signal.signal.int64_value;
        break;
      }

      case proton_Signal_uint32_value_tag: {
        *((uint32_t *)signal_handle->value.data) =
            signal_handle->signal.signal.uint32_value;
        break;
      }

      case proton_Signal_uint64_value_tag: {
        *((uint64_t *)signal_handle->value.data) =
            signal_handle->signal.signal.uint64_value;
        break;
      }

      case proton_Signal_bool_value_tag: {
        *((bool *)signal_handle->value.data) =
            signal_handle->signal.signal.bool_value;
        break;
      }

      default:
        break;
      }
    }

    if (stream.bytes_left == 0) {
      return PROTON_OK;
    } else {
      return PROTON_SERIALIZATION_ERROR;
    }
  } else {
    PROTON_PRINT("Decode error: %s\r\n", stream.errmsg);
    return PROTON_SERIALIZATION_ERROR;
  }
}

proton_status_e proton_spin(proton_node_t *node, const uint8_t peer) {
  proton_status_e status;
  while (1) {
    status = proton_spin_once(node, peer);

    if (status != PROTON_OK) {
      PROTON_PRINT("Spin error %u\r\n", status);
    }
  }

  return PROTON_ERROR;
}

proton_status_e proton_spin_once(proton_node_t *node, const uint8_t peer) {
  if (node == NULL || node->peers == NULL) {
    return PROTON_NULL_PTR_ERROR;
  }

  if (node->state != PROTON_NODE_ACTIVE)
  {
    return PROTON_INVALID_STATE_ERROR;
  }

  if (peer >= node->peer_count)
  {
    return PROTON_ERROR;
  }

  proton_peer_t * peer_handle = &node->peers[peer];

  if (peer_handle == NULL)
  {
    return PROTON_ERROR;
  }

  size_t bytes_read = 0;

  switch(peer_handle->transport.state)
  {
    case PROTON_TRANSPORT_DISCONNECTED:
    {
      if (peer_handle->transport.connect())
      {
        peer_handle->transport.state = PROTON_TRANSPORT_CONNECTED;
      }
      else
      {
        peer_handle->transport.state = PROTON_TRANSPORT_ERROR;
        return PROTON_CONNECT_ERROR;
      }
      break;
    }

    case PROTON_TRANSPORT_CONNECTED:
    {
      // Lock atomic buffer
      if (!peer_handle->atomic_buffer.lock())
      {
        return PROTON_MUTEX_ERROR;
      }

      // Read from peer transport
      size_t bytes_read = peer_handle->transport.read(
                            peer_handle->atomic_buffer.buffer.data,
                            peer_handle->atomic_buffer.buffer.len);

      // Unlock atomic buffer
      if (!peer_handle->atomic_buffer.unlock())
      {
        return PROTON_MUTEX_ERROR;
      }

      if (bytes_read > 0) {
        // Receive bundle from read data
        if (peer_handle->receive(peer_handle->atomic_buffer.buffer.data, bytes_read) != PROTON_OK) {
          // Unlock atomic buffer
          if (!peer_handle->atomic_buffer.unlock())
          {
            return PROTON_MUTEX_ERROR;
          }

          return PROTON_READ_ERROR;
        }
      }
      if (!peer_handle->atomic_buffer.unlock())
      {
        return PROTON_MUTEX_ERROR;
      }

      break;
    }

    case PROTON_TRANSPORT_ERROR:
    {
      if (peer_handle->transport.disconnect())
      {
        peer_handle->transport.state = PROTON_TRANSPORT_DISCONNECTED;
      }
      else
      {
        return PROTON_CONNECT_ERROR;
      }
      break;
    }

    default:
    {
      return PROTON_ERROR;
    }
  }

  return PROTON_OK;
}

#ifdef PROTON_DEBUG
/**
 * @brief Weak implementation of print function. To be implemented by user if debug prints are needed.
 *
 */
__attribute__((weak)) int proton_print(const char * format, ...)
{
  return 0;
}
#endif

void proton_print_bundle(proton_Bundle bundle) {
  proton_list_t *args = (proton_list_t *)bundle.signals;
  PROTON_PRINT("Proton Bundle { \r\n");
  PROTON_PRINT("\tID: 0x%x\r\n", bundle.id);
  PROTON_PRINT("\tSignals { \r\n");
  for (int i = 0; i < args->length; i++) {
    proton_print_signal(((proton_signal_handle_t *)args->data)[i].signal);
  }
  PROTON_PRINT("\t}\r\n}\r\n");
}

void proton_print_signal(proton_Signal signal) {
  pb_size_t which = signal.which_signal;

  proton_list_t *arg;

  switch (which) {
  case proton_Signal_bool_value_tag: {
    PROTON_PRINT("\t\tbool_value: %d\r\n", signal.signal.bool_value);
    break;
  }

  case proton_Signal_double_value_tag: {
    PROTON_PRINT("\t\tdouble_value: %lf\r\n", signal.signal.double_value);
    break;
  }

  case proton_Signal_float_value_tag: {
    PROTON_PRINT("\t\tfloat_value: %f\r\n", signal.signal.float_value);
    break;
  }

  case proton_Signal_int32_value_tag: {
    PROTON_PRINT("\t\tint32_value: %d\r\n", signal.signal.int32_value);
    break;
  }

  case proton_Signal_int64_value_tag: {
    PROTON_PRINT("\t\tint64_value: %ld\r\n", signal.signal.int64_value);
    break;
  }

  case proton_Signal_uint32_value_tag: {
    PROTON_PRINT("\t\tuint32_value: %u\r\n", signal.signal.uint32_value);
    break;
  }

  case proton_Signal_uint64_value_tag: {
    PROTON_PRINT("\t\tuint64_value: %lu\r\n", signal.signal.uint64_value);
    break;
  }

  case proton_Signal_string_value_tag: {
    if (signal.signal.string_value) {
      arg = (proton_list_t *)signal.signal.string_value;
      PROTON_PRINT("\t\tstring_value: %s\r\n", (char *)arg->data);
    } else {
      PROTON_PRINT("\t\tNULL string\r\n");
    }
    break;
  }

  case proton_Signal_bytes_value_tag: {
    if (signal.signal.bytes_value) {
      arg = (proton_list_t *)signal.signal.bytes_value;
      PROTON_PRINT("\t\tbytes_value: [");
      for (int i = 0; i < arg->capacity; i++) {
        PROTON_PRINT("0x%x", ((uint8_t *)arg->data)[i]);
        if (i != arg->capacity - 1) {
          PROTON_PRINT(", ");
        }
      }
      PROTON_PRINT("]\r\n");
    } else {
      PROTON_PRINT("\t\tNULL bytes\r\n");
    }
    break;
  }

  case proton_Signal_list_double_value_tag: {
    if (signal.signal.list_double_value.doubles) {
      arg = (proton_list_t *)signal.signal.list_double_value.doubles;
      PROTON_PRINT("\t\tlist_double_value: {\r\n");
      for (int i = 0; i < arg->length; i++) {
        PROTON_PRINT("\t\t\t%lf\r\n", ((double *)arg->data)[i]);
      }
      PROTON_PRINT("\t\t}\r\n");
    } else {
      PROTON_PRINT("\t\tNULL double array \r\n");
    }
    break;
  }

  case proton_Signal_list_float_value_tag: {
    if (signal.signal.list_float_value.floats) {
      arg = (proton_list_t *)signal.signal.list_float_value.floats;
      PROTON_PRINT("\t\tlist_float_value: {\r\n");
      for (int i = 0; i < arg->length; i++) {
        PROTON_PRINT("\t\t\t%f\r\n", ((float *)arg->data)[i]);
      }
      PROTON_PRINT("\t\t}\r\n");
    } else {
      PROTON_PRINT("\t\tNULL float array \r\n");
    }
    break;
  }

  case proton_Signal_list_int32_value_tag: {
    if (signal.signal.list_int32_value.int32s) {
      arg = (proton_list_t *)signal.signal.list_int32_value.int32s;
      PROTON_PRINT("\t\tlist_int32_value: {\r\n");
      for (int i = 0; i < arg->length; i++) {
        PROTON_PRINT("\t\t\t%d\r\n", ((int32_t *)arg->data)[i]);
      }
      PROTON_PRINT("\t\t}\r\n");
    } else {
      PROTON_PRINT("\t\tNULL int32 array \r\n");
    }
    break;
  }

  case proton_Signal_list_int64_value_tag: {
    if (signal.signal.list_int64_value.int64s) {
      arg = (proton_list_t *)signal.signal.list_int64_value.int64s;
      PROTON_PRINT("\t\tlist_int64_value: {\r\n");
      for (int i = 0; i < arg->length; i++) {
        PROTON_PRINT("\t\t\t%ld\r\n", ((int64_t *)arg->data)[i]);
      }
      PROTON_PRINT("\t\t}\r\n");
    } else {
      PROTON_PRINT("\t\tNULL int64 array \r\n");
    }
    break;
  }

  case proton_Signal_list_uint32_value_tag: {
    if (signal.signal.list_uint32_value.uint32s) {
      arg = (proton_list_t *)signal.signal.list_uint32_value.uint32s;
      PROTON_PRINT("\t\tlist_uint32_value: {\r\n");
      for (int i = 0; i < arg->length; i++) {
        PROTON_PRINT("\t\t\t%u\r\n", ((uint32_t *)arg->data)[i]);
      }
      PROTON_PRINT("\t\t}\r\n");
    } else {
      PROTON_PRINT("\t\tNULL uint32 array \r\n");
    }
    break;
  }

  case proton_Signal_list_uint64_value_tag: {
    if (signal.signal.list_uint64_value.uint64s) {
      arg = (proton_list_t *)signal.signal.list_uint64_value.uint64s;
      PROTON_PRINT("\t\tlist_uint64_value: {\r\n");
      for (int i = 0; i < arg->length; i++) {
        PROTON_PRINT("\t\t\t%lu\r\n", ((uint64_t *)arg->data)[i]);
      }
      PROTON_PRINT("\t\t}\r\n");
    } else {
      PROTON_PRINT("\t\tNULL uint64 array \r\n");
    }
    break;
  }

  case proton_Signal_list_bool_value_tag: {
    if (signal.signal.list_bool_value.bools) {
      arg = (proton_list_t *)signal.signal.list_bool_value.bools;
      PROTON_PRINT("\t\tlist_bool_value: {\r\n");
      for (int i = 0; i < arg->length; i++) {
        PROTON_PRINT("\t\t\t%u\r\n", ((bool *)arg->data)[i]);
      }
      PROTON_PRINT("\t\t}\r\n");
    } else {
      PROTON_PRINT("\t\tNULL bool array \r\n");
    }
    break;
  }

  case proton_Signal_list_string_value_tag: {
    if (signal.signal.list_string_value.strings) {
      arg = (proton_list_t *)signal.signal.list_string_value.strings;
      PROTON_PRINT("\t\tlist_string_value: {\r\n");
      for (int i = 0; i < arg->length; i += arg->capacity) {
        PROTON_PRINT("\t\t\t%s\r\n", (((char(*)[arg->capacity])arg->data)[i]));
      }
      PROTON_PRINT("\t\t}\r\n");
    } else {
      PROTON_PRINT("\t\tNULL string list\r\n");
    }
    break;
  }

  case proton_Signal_list_bytes_value_tag: {
    if (signal.signal.list_bytes_value.bytes) {
      arg = (proton_list_t *)signal.signal.list_bytes_value.bytes;
      PROTON_PRINT("\t\tlist_bytes_value: {\r\n");
      for (int i = 0; i < arg->length; i++) {
        PROTON_PRINT("\t\t\t[");
        for (int j = 0; j < arg->capacity; j++) {
          uint8_t data = ((uint8_t(*)[arg->capacity])arg->data)[i][j];
          if (j == arg->capacity - 1) {
            if (i == arg->length - 1) {
              PROTON_PRINT("0x%x]\r\n", data);
            } else {
              PROTON_PRINT("0x%x],\r\n", data);
            }
          } else {
            PROTON_PRINT("0x%x, ", data);
          }
        }
      }
      PROTON_PRINT("\t\t}\r\n");
    } else {
      PROTON_PRINT("\t\tNULL bytes list\r\n");
    }
    break;
  }
  }
}
