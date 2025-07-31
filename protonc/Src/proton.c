/**
 * Software License Agreement (proprietary)
 *
 * @copyright Copyright (c) 2025 Clearpath Robotics, Inc., All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, is not permitted without the
 * express permission of Clearpath Robotics.
 *
 * @author Roni Kreinin (rkreinin@clearpathrobotics.com)
 */

#include "proton.h"
#include <stdio.h>

void PROTON_InitProton(proton_t * proton, uint32_t id, proton_Signal * signals, proton_signal_schema_t * schema, uint32_t signal_count)
{
  if (signals && schema)
  {
    for (size_t i = 0; i < signal_count; i++)
    {
      PROTON_InitSignal(&signals[i], &schema[i]);
    }

    proton->arg.values = signals;
    proton->arg.capacity = signal_count;
    proton->arg.size = 0;
    proton->proton.id = id;
    proton->proton.signals = &proton->arg;
  }
  else
  {
    // Error
  }
}

void PROTON_InitDoubleSignal(proton_Signal * signal)
{
  signal->which_signal = proton_Signal_double_value_tag;
  signal->signal.double_value = 0.0;
}

void PROTON_InitFloatSignal(proton_Signal * signal)
{
  signal->which_signal = proton_Signal_float_value_tag;
  signal->signal.float_value = 0.0f;
}

void PROTON_InitInt32Signal(proton_Signal * signal)
{
  signal->which_signal = proton_Signal_int32_value_tag;
  signal->signal.int32_value = 0;
}

void PROTON_InitInt64Signal(proton_Signal * signal)
{
  signal->which_signal = proton_Signal_int64_value_tag;
  signal->signal.int64_value = 0;
}

void PROTON_InitUint32Signal(proton_Signal * signal)
{
  signal->which_signal = proton_Signal_uint32_value_tag;
  signal->signal.uint32_value = 0;
}

void PROTON_InitUint64Signal(proton_Signal * signal)
{
  signal->which_signal = proton_Signal_uint64_value_tag;
  signal->signal.uint64_value = 0;
}

void PROTON_InitBoolSignal(proton_Signal * signal)
{
  signal->which_signal = proton_Signal_bool_value_tag;
  signal->signal.bool_value = false;
}

void PROTON_InitStringSignal(proton_Signal * signal, proton_list_arg_t * arg)
{
  signal->which_signal = proton_Signal_string_value_tag;
  signal->signal.string_value = arg;
}

void PROTON_InitBytesSignal(proton_Signal * signal, proton_list_arg_t * arg)
{
  signal->which_signal = proton_Signal_bytes_value_tag;
  signal->signal.bytes_value = arg;
}

void PROTON_InitListDoublesSignal(proton_Signal * signal, proton_list_arg_t * arg)
{
  signal->which_signal = proton_Signal_list_double_value_tag;
  signal->signal.list_double_value.doubles = arg;
}

void PROTON_InitListFloatsSignal(proton_Signal * signal, proton_list_arg_t * arg)
{
  signal->which_signal = proton_Signal_list_float_value_tag;
  signal->signal.list_float_value.floats = arg;
}

void PROTON_InitListInt32sSignal(proton_Signal * signal, proton_list_arg_t * arg)
{
  signal->which_signal = proton_Signal_list_int32_value_tag;
  signal->signal.list_int32_value.int32s = arg;
}

void PROTON_InitListInt64sSignal(proton_Signal * signal, proton_list_arg_t * arg)
{
  signal->which_signal = proton_Signal_list_int64_value_tag;
  signal->signal.list_int64_value.int64s = arg;
}

void PROTON_InitListUint32sSignal(proton_Signal * signal, proton_list_arg_t * arg)
{
  signal->which_signal = proton_Signal_list_uint32_value_tag;
  signal->signal.list_uint32_value.uint32s = arg;
}

void PROTON_InitListUint64sSignal(proton_Signal * signal, proton_list_arg_t * arg)
{
  signal->which_signal = proton_Signal_list_uint64_value_tag;
  signal->signal.list_uint64_value.uint64s = arg;
}

void PROTON_InitListBoolsSignal(proton_Signal * signal, proton_list_arg_t * arg)
{
  signal->which_signal = proton_Signal_list_bool_value_tag;
  signal->signal.list_bool_value.bools = arg;
}

void PROTON_InitListStringsSignal(proton_Signal * signal, proton_list_arg_t * arg)
{
  signal->which_signal = proton_Signal_list_string_value_tag;
  signal->signal.list_string_value.strings = arg;
}

void PROTON_InitSignal(proton_Signal * signal, proton_signal_schema_t * schema)
{
  switch(schema->type)
  {
    case PROTON_SIGNAL_TYPE_BOOL_VALUE:
    {
      PROTON_InitBoolSignal(signal);
      break;
    }

    case PROTON_SIGNAL_TYPE_DOUBLE_VALUE:
    {
      PROTON_InitDoubleSignal(signal);
      break;
    }

    case PROTON_SIGNAL_TYPE_FLOAT_VALUE:
    {
      PROTON_InitFloatSignal(signal);
      break;
    }

    case PROTON_SIGNAL_TYPE_INT32_VALUE:
    {
      PROTON_InitInt32Signal(signal);
      break;
    }

    case PROTON_SIGNAL_TYPE_INT64_VALUE:
    {
      PROTON_InitInt64Signal(signal);
      break;
    }

    case PROTON_SIGNAL_TYPE_UINT32_VALUE:
    {
      PROTON_InitUint32Signal(signal);
      break;
    }

    case PROTON_SIGNAL_TYPE_UINT64_VALUE:
    {
      PROTON_InitUint64Signal(signal);
      break;
    }

    case PROTON_SIGNAL_TYPE_STRING_VALUE:
    {
      PROTON_InitStringSignal(signal, &schema->arg);
      break;
    }

    case PROTON_SIGNAL_TYPE_BYTES_VALUE:
    {
      PROTON_InitBytesSignal(signal, &schema->arg);
      break;
    }

    case PROTON_SIGNAL_TYPE_LIST_DOUBLE_VALUE:
    {
      PROTON_InitListDoublesSignal(signal, &schema->arg);
      break;
    }

    case PROTON_SIGNAL_TYPE_LIST_FLOAT_VALUE:
    {
      PROTON_InitListFloatsSignal(signal, &schema->arg);
      break;
    }

    case PROTON_SIGNAL_TYPE_LIST_INT32_VALUE:
    {
      PROTON_InitListInt32sSignal(signal, &schema->arg);
      break;
    }

    case PROTON_SIGNAL_TYPE_LIST_INT64_VALUE:
    {
      PROTON_InitListInt64sSignal(signal, &schema->arg);
      break;
    }

    case PROTON_SIGNAL_TYPE_LIST_UINT32_VALUE:
    {
      PROTON_InitListUint32sSignal(signal, &schema->arg);
      break;
    }

    case PROTON_SIGNAL_TYPE_LIST_UINT64_VALUE:
    {
      PROTON_InitListUint64sSignal(signal, &schema->arg);
      break;
    }

    case PROTON_SIGNAL_TYPE_LIST_BOOL_VALUE:
    {
      PROTON_InitListBoolsSignal(signal, &schema->arg);
      break;
    }

    case PROTON_SIGNAL_TYPE_LIST_STRING_VALUE:
    {
      PROTON_InitListStringsSignal(signal, &schema->arg);
      break;
    }

    default:
      break;
  }
}

bool PROTON_CopyStringToSignal(proton_Signal * signal, const char * string)
{
  // Check that this is a string_value signal
  if (signal->which_signal != proton_Signal_string_value_tag)
  {
    return false;
  }

  proton_list_arg_t * arg = (proton_list_arg_t *)signal->signal.string_value;

  // Check that string will fit in buffer
  if (strlen(string) > arg->capacity - 1)
  {
    return false;
  }

  strcpy(arg->values, string);
  arg->size = strlen(arg->values);

  return true;
}

bool PROTON_CopyStringToListString(proton_Signal * signal, const char * string, size_t index)
{
  // Check that this is a list_string_value signal
  if (signal->which_signal != proton_Signal_list_string_value_tag)
  {
    return false;
  }

  proton_list_arg_t * arg = (proton_list_arg_t *)signal->signal.list_string_value.strings;

  if (!arg)
  {
    return false;
  }

  // Check that index is in list
  if (index >= arg->capacity)
  {
    return false;
  }

  char * string_buf = ((char **)arg->values)[index];

  strcpy(string_buf, string);

  return true;
}

int PROTON_Encode(proton_Proton * msg, uint8_t * buffer, size_t buffer_length)
{
  pb_ostream_t stream = pb_ostream_from_buffer((pb_byte_t *)buffer, buffer_length);
  bool status = pb_encode(&stream, proton_Proton_fields, msg);
  size_t bytes_written = stream.bytes_written;

  if (status)
  {
    return bytes_written;
  }
  else
  {
    printf("Error: %s\r\n", stream.errmsg);
    return -1;
  }
}

int PROTON_Decode(proton_Proton * msg, const uint8_t * buffer, size_t buffer_length)
{
  pb_istream_t stream = pb_istream_from_buffer((const pb_byte_t *)buffer, buffer_length);
  bool status = pb_decode(&stream, proton_Proton_fields, msg);
  size_t bytes_left = stream.bytes_left;

  if (status)
  {
    return bytes_left;
  }
  else
  {
    return -1;
  }
}
