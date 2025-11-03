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

void PROTON_InitBundle(proton_bundle_handle_t *handle, uint32_t id,
                       proton_signal_handle_t *signal_handles, uint32_t signal_count) {
  if (signal_handles && handle) {
    handle->arg.data = signal_handles;
    handle->arg.length = signal_count;
    handle->arg.size = 0;
    handle->bundle.id = id;
    handle->bundle.signals = &handle->arg;
  } else {
    // Error
  }
}

void PROTON_InitNode(proton_node_t *node, proton_transport_t transport,
                     proton_receive_t receive, proton_buffer_t read_buf,
                     proton_buffer_t write_buf) {
  node->transport = transport;
  node->receive = receive;
  node->read_buf = read_buf;
  node->write_buf = write_buf;
  node->connected = false;
}

int PROTON_Encode(proton_bundle_handle_t *handle, uint8_t *buffer,
                  size_t buffer_length) {
  proton_signal_handle_t *signal_handle;

  // Copy non-list field values from struct to signal
  for (uint8_t i = 0; i < handle->arg.length; i++) {
    signal_handle = &((proton_signal_handle_t *)handle->arg.data)[i];
    switch (signal_handle->signal.which_signal) {
    case proton_Signal_double_value_tag: {
      signal_handle->signal.signal.double_value = *((double *)signal_handle->arg.data);
      break;
    }

    case proton_Signal_float_value_tag: {
      signal_handle->signal.signal.float_value = *((float *)signal_handle->arg.data);
      break;
    }

    case proton_Signal_int32_value_tag: {
      signal_handle->signal.signal.int32_value = *((int32_t *)signal_handle->arg.data);
      break;
    }

    case proton_Signal_int64_value_tag: {
      signal_handle->signal.signal.int64_value = *((int64_t *)signal_handle->arg.data);
      break;
    }

    case proton_Signal_uint32_value_tag: {
      signal_handle->signal.signal.uint32_value = *((uint32_t *)signal_handle->arg.data);
      break;
    }

    case proton_Signal_uint64_value_tag: {
      signal_handle->signal.signal.uint64_value = *((uint64_t *)signal_handle->arg.data);
      break;
    }

    case proton_Signal_bool_value_tag: {
      signal_handle->signal.signal.bool_value = *((bool *)signal_handle->arg.data);
      break;
    }

    default:
      break;
    }
  }

  pb_ostream_t stream =
      pb_ostream_from_buffer((pb_byte_t *)buffer, buffer_length);
  bool status = pb_encode(&stream, proton_Bundle_fields, &handle->bundle);
  size_t bytes_written = stream.bytes_written;

  if (status) {
    return bytes_written;
  } else {
    printf("Error: %s\r\n", stream.errmsg);
    return -1;
  }
}

bool PROTON_DecodeId(uint32_t *id, const uint8_t *buffer,
                     size_t buffer_length) {
  pb_istream_t stream =
      pb_istream_from_buffer((const pb_byte_t *)buffer, buffer_length);

  pb_wire_type_t wire_type;
  uint32_t tag;
  bool eof;

  if (pb_decode_tag(&stream, &wire_type, &tag, &eof)) {
    if (tag == proton_Bundle_id_tag) {
      if (pb_decode_varint32(&stream, id)) {
        return true;
      }
    }
  }

  return false;
}

int PROTON_Decode(proton_bundle_handle_t *handle, const uint8_t *buffer,
                  size_t buffer_length) {
  pb_istream_t stream =
      pb_istream_from_buffer((const pb_byte_t *)buffer, buffer_length);

  bool status = pb_decode(&stream, proton_Bundle_fields, &handle->bundle);
  size_t bytes_left = stream.bytes_left;

  if (status) {
    proton_signal_handle_t * signal_handle;

    for (uint8_t i = 0; i < handle->arg.length; i++) {
      signal_handle = &((proton_signal_handle_t *)handle->arg.data)[i];
      switch (signal_handle->signal.which_signal) {
      case proton_Signal_double_value_tag: {
        *((double *)signal_handle->arg.data) = signal_handle->signal.signal.double_value;
        break;
      }

      case proton_Signal_float_value_tag: {
        *((float *)signal_handle->arg.data) = signal_handle->signal.signal.float_value;
        break;
      }

      case proton_Signal_int32_value_tag: {
        *((int32_t *)signal_handle->arg.data) = signal_handle->signal.signal.int32_value;
        break;
      }

      case proton_Signal_int64_value_tag: {
        *((int64_t *)signal_handle->arg.data) = signal_handle->signal.signal.int64_value;
        break;
      }

      case proton_Signal_uint32_value_tag: {
        *((uint32_t *)signal_handle->arg.data) = signal_handle->signal.signal.uint32_value;
        break;
      }

      case proton_Signal_uint64_value_tag: {
        *((uint64_t *)signal_handle->arg.data) = signal_handle->signal.signal.uint64_value;
        break;
      }

      case proton_Signal_bool_value_tag: {
        *((bool *)signal_handle->arg.data) = signal_handle->signal.signal.bool_value;
        break;
      }

      default:
        break;
      }
    }

    return bytes_left;
  } else {
    printf("Error: %s\r\n", stream.errmsg);
    return -1;
  }
}

// proton_status_e PROTON_Send(proton_node_t *node, )

proton_status_e PROTON_Spin(proton_node_t *node) {
  proton_status_e status;
  while (1) {
    status = PROTON_SpinOnce(node);

    if (status == PROTON_ERR) {
      return status;
    }
  }

  return PROTON_ERR;
}

proton_status_e PROTON_SpinOnce(proton_node_t *node) {
  if (!node) {
    return PROTON_ERR;
  }

  if (!node->connected) {
    if (!node->transport.connect) {
      return PROTON_ERR;
    }
    node->connected = node->transport.connect();
  } else {
    if (node->transport.read && node->read_buf.data) {
      size_t bytes_read =
          node->transport.read(node->read_buf.data, node->read_buf.len);
      if (bytes_read > 0) {
        if (!node->receive(node->read_buf.data, bytes_read)) {
          return PROTON_READ_ERR;
        }
      }
    }
    else
    {
      printf("Error\r\n");
      return PROTON_ERR;
    }
  }

  return PROTON_OK;
}

void PROTON_PrintBundle(proton_Bundle bundle) {
  proton_list_t *args = (proton_list_t *)bundle.signals;
  printf("Proton Bundle { \r\n");
  printf("\tID: 0x%x\r\n", bundle.id);
  printf("\tSignals { \r\n");
  for (int i = 0; i < args->length; i++) {
    PROTON_PrintSignal(((proton_signal_handle_t *)args->data)[i].signal);
  }
  printf("\t}\r\n}\r\n");
}

void PROTON_PrintSignal(proton_Signal signal) {
  pb_size_t which = signal.which_signal;

  proton_list_t * arg;

  switch (which) {
  case proton_Signal_bool_value_tag: {
    printf("\t\tbool_value: %d\r\n", signal.signal.bool_value);
    break;
  }

  case proton_Signal_double_value_tag: {
    printf("\t\tdouble_value: %lf\r\n", signal.signal.double_value);
    break;
  }

  case proton_Signal_float_value_tag: {
    printf("\t\tfloat_value: %f\r\n", signal.signal.float_value);
    break;
  }

  case proton_Signal_int32_value_tag: {
    printf("\t\tint32_value: %d\r\n", signal.signal.int32_value);
    break;
  }

  case proton_Signal_int64_value_tag: {
    printf("\t\tint64_value: %ld\r\n", signal.signal.int64_value);
    break;
  }

  case proton_Signal_uint32_value_tag: {
    printf("\t\tuint32_value: %u\r\n", signal.signal.uint32_value);
    break;
  }

  case proton_Signal_uint64_value_tag: {
    printf("\t\tuint64_value: %lu\r\n", signal.signal.uint64_value);
    break;
  }

  case proton_Signal_string_value_tag: {
    if (signal.signal.string_value) {
      arg = (proton_list_t *)signal.signal.string_value;
      printf("\t\tstring_value: %s\r\n", (char *)arg->data);
    } else {
      printf("\t\tNULL string\r\n");
    }
    break;
  }

  case proton_Signal_bytes_value_tag: {
    if (signal.signal.bytes_value) {
      arg = (proton_list_t *)signal.signal.bytes_value;
      printf("\t\tbytes_value: [");
      for (int i = 0; i < arg->capacity; i++) {
        printf("0x%x", ((uint8_t *)arg->data)[i]);
        if (i != arg->capacity - 1) {
          printf(", ");
        }
      }
      printf("]\r\n");
    } else {
      printf("\t\tNULL bytes\r\n");
    }
    break;
  }

  case proton_Signal_list_double_value_tag: {
    if (signal.signal.list_double_value.doubles) {
      arg = (proton_list_t *)signal.signal.list_double_value.doubles;
      printf("\t\tlist_double_value: {\r\n");
      for (int i = 0; i < arg->length; i++) {
        printf("\t\t\t%lf\r\n", ((double *)arg->data)[i]);
      }
      printf("\t\t}\r\n");
    } else {
      printf("\t\tNULL double array \r\n");
    }
    break;
  }

  case proton_Signal_list_float_value_tag: {
    if (signal.signal.list_float_value.floats) {
      arg = (proton_list_t *)signal.signal.list_float_value.floats;
      printf("\t\tlist_float_value: {\r\n");
      for (int i = 0; i < arg->length; i++) {
        printf("\t\t\t%f\r\n", ((float *)arg->data)[i]);
      }
      printf("\t\t}\r\n");
    } else {
      printf("\t\tNULL float array \r\n");
    }
    break;
  }

  case proton_Signal_list_int32_value_tag: {
    if (signal.signal.list_int32_value.int32s) {
      arg = (proton_list_t *)signal.signal.list_int32_value.int32s;
      printf("\t\tlist_int32_value: {\r\n");
      for (int i = 0; i < arg->length; i++) {
        printf("\t\t\t%d\r\n", ((int32_t *)arg->data)[i]);
      }
      printf("\t\t}\r\n");
    } else {
      printf("\t\tNULL int32 array \r\n");
    }
    break;
  }

  case proton_Signal_list_int64_value_tag: {
    if (signal.signal.list_int64_value.int64s) {
      arg = (proton_list_t *)signal.signal.list_int64_value.int64s;
      printf("\t\tlist_int64_value: {\r\n");
      for (int i = 0; i < arg->length; i++) {
        printf("\t\t\t%ld\r\n", ((int64_t *)arg->data)[i]);
      }
      printf("\t\t}\r\n");
    } else {
      printf("\t\tNULL int64 array \r\n");
    }
    break;
  }

  case proton_Signal_list_uint32_value_tag: {
    if (signal.signal.list_uint32_value.uint32s) {
      arg = (proton_list_t *)signal.signal.list_uint32_value.uint32s;
      printf("\t\tlist_uint32_value: {\r\n");
      for (int i = 0; i < arg->length; i++) {
        printf("\t\t\t%u\r\n", ((uint32_t *)arg->data)[i]);
      }
      printf("\t\t}\r\n");
    } else {
      printf("\t\tNULL uint32 array \r\n");
    }
    break;
  }

  case proton_Signal_list_uint64_value_tag: {
    if (signal.signal.list_uint64_value.uint64s) {
      arg = (proton_list_t *)signal.signal.list_uint64_value.uint64s;
      printf("\t\tlist_uint64_value: {\r\n");
      for (int i = 0; i < arg->length; i++) {
        printf("\t\t\t%lu\r\n", ((uint64_t *)arg->data)[i]);
      }
      printf("\t\t}\r\n");
    } else {
      printf("\t\tNULL uint64 array \r\n");
    }
    break;
  }

  case proton_Signal_list_bool_value_tag: {
    if (signal.signal.list_bool_value.bools) {
      arg = (proton_list_t *)signal.signal.list_bool_value.bools;
      printf("\t\tlist_bool_value: {\r\n");
      for (int i = 0; i < arg->length; i++) {
        printf("\t\t\t%u\r\n", ((bool *)arg->data)[i]);
      }
      printf("\t\t}\r\n");
    } else {
      printf("\t\tNULL bool array \r\n");
    }
    break;
  }

  case proton_Signal_list_string_value_tag: {
    if (signal.signal.list_string_value.strings) {
      arg = (proton_list_t *)signal.signal.list_string_value.strings;
      printf("\t\tlist_string_value: {\r\n");
      for (int i = 0; i < arg->length; i+= arg->capacity) {
        printf("\t\t\t%s\r\n", (((char (*)[arg->capacity])arg->data)[i]));
      }
      printf("\t\t}\r\n");
    } else {
      printf("\t\tNULL string list\r\n");
    }
    break;
  }

  case proton_Signal_list_bytes_value_tag: {
    if (signal.signal.list_bytes_value.bytes) {
      arg = (proton_list_t *)signal.signal.list_bytes_value.bytes;
      printf("\t\tlist_bytes_value: {\r\n");
      for (int i = 0; i < arg->length; i++) {
        printf("\t\t\t[");
        for (int j = 0; j < arg->capacity; j++)
        {
          uint8_t data = ((uint8_t (*)[arg->capacity])arg->data)[i][j];
          if (j == arg->capacity - 1)
          {
            if (i == arg->length - 1)
            {
              printf("0x%x]\r\n", data);
            }
            else
            {
              printf("0x%x],\r\n", data);
            }
          }
          else {
            printf("0x%x, ", data);
          }
        }
      }
      printf("\t\t}\r\n");
    } else {
      printf("\t\tNULL bytes list\r\n");
    }
    break;
  }
  }
}

uint16_t PROTON_CRC16(const uint8_t *data, uint16_t len) {
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
  return crc;
}

bool PROTON_FillFrameHeader(uint8_t * header, uint16_t payload_len) {
  if (header == NULL)
  {
    return false;
  }

  header[0] = PROTON_FRAME_HEADER_MAGIC_BYTE_0;
  header[1] = PROTON_FRAME_HEADER_MAGIC_BYTE_1;
  header[2] = (uint8_t)(payload_len & 0xFF);
  header[3] = (uint8_t)(payload_len >> 8);

  return true;
}

bool PROTON_FillCRC16(const uint8_t * payload, const uint16_t payload_len, uint8_t * crc)
{
  if (payload == NULL || crc == NULL)
  {
    return false;
  }

  uint16_t crc16 = PROTON_CRC16(payload, payload_len);
  crc[0] = (uint8_t)(crc16 & 0xFF);
  crc[1] = (uint8_t)(crc16 >> 8);

  return true;
}

bool PROTON_CheckFramedPayload(const uint8_t *payload, const size_t payload_len, const uint16_t frame_crc) {
  return PROTON_CRC16(payload, payload_len) == frame_crc;
}

uint16_t PROTON_GetFramedPayloadLength(const uint8_t *framed_buf) {
  if (framed_buf[0] != PROTON_FRAME_HEADER_MAGIC_BYTE_0 ||
      framed_buf[1] != PROTON_FRAME_HEADER_MAGIC_BYTE_1) {
    return 0;
  }

  return (framed_buf[2] | framed_buf[3] << 8);
}
