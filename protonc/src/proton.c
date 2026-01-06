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

/**
 * @brief Initialize a signal handle.
 *
 * @param handle Handle to initialize
 * @param which_signal Signal type
 * @param data Pointer to data buffer
 * @param length Length of list (for list types)
 * @param capacity Capacity of data buffer (for strings and bytes types)
 * @return proton_status_e Status of the initialization
 */
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

/**
 * @brief Initialize a bundle handle.
 *
 * @param handle Handle to initialize
 * @param id Bundle ID
 * @param signal_handles Array of signal handles
 * @param signal_count Number of signal handles
 * @param producers Producers bitmask
 * @param consumers Consumers bitmask
 * @return proton_status_e Status of the initialization
 */
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

/**
 * @brief Encode a bundle into a buffer.
 *
 * @param handle Bundle handle to encode
 * @param buffer Buffer to encode into
 * @param bytes_encoded Pointer to store the number of bytes encoded
 * @return proton_status_e Status of the encoding
 */
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
  pb_ostream_t stream = pb_ostream_from_buffer((pb_byte_t *)buffer.data, buffer.len);
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

/**
 * @brief Decode the Bundle ID from a buffer.
 *
 * @param buffer Buffer to decode from
 * @param id Pointer to store the decoded Bundle ID
 * @return proton_status_e Status of the decoding
 */
proton_status_e proton_decode_id(proton_buffer_t buffer, uint32_t *id) {
  if (id == NULL || buffer.data == NULL) {
    return PROTON_NULL_PTR_ERROR;
  }

  pb_wire_type_t wire_type;
  uint32_t tag;
  bool eof;
  proton_status_e status = PROTON_SERIALIZATION_ERROR;

  pb_istream_t stream = pb_istream_from_buffer((const pb_byte_t *)buffer.data, buffer.len);

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

/**
 * @brief Decode a bundle from a buffer.
 *
 * @param handle Bundle handle to decode into
 * @param buffer Buffer to decode from
 * @param length Length of the buffer to decode
 * @return proton_status_e Status of the decoding
 */
proton_status_e proton_decode(proton_bundle_handle_t *handle, proton_buffer_t buffer, size_t length) {
  if (handle == NULL || handle->signals.data == NULL || buffer.data == NULL)
  {
    return PROTON_NULL_PTR_ERROR;
  }

  if (length > buffer.len)
  {
    return PROTON_INSUFFICIENT_BUFFER_ERROR;
  }

  pb_istream_t stream = pb_istream_from_buffer((const pb_byte_t *)buffer.data, length);

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

/**
 * @brief Print a bundle to the debug output.
 *
 * @param bundle Bundle to print
 */
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

/**
 * @brief Print a signal to the debug output.
 *
 * @param signal Signal to print
 */
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
