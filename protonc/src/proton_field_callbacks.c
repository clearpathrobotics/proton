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

#include "protonc/proton.h"
#include <stdio.h>

bool proton_Bundle_callback(pb_istream_t *istream, pb_ostream_t *ostream, const pb_field_t *field)
{
  if (!field)
  {
    return false;
  }

  proton_Bundle * msg = (proton_Bundle *)field->message;

  if (!msg)
  {
    return false;
  }

  proton_list_t * signal_list = (proton_list_t *)msg->signals;
  proton_signal_handle_t * signal_handles = (proton_signal_handle_t *)signal_list->data;

  if (!signal_list || !signal_handles)
  {
    return false;
  }

  // Decode
  if (istream)
  {
    if (field->tag == proton_Bundle_signals_tag)
    {
      proton_Signal * signal = &(signal_handles[signal_list->size++].signal);

      // This is the last signal in the list, reset size counter
      if (signal_list->size == signal_list->length)
      {
        signal_list->size = 0;
      }

      switch(signal->which_signal)
      {
        case proton_Signal_list_double_value_tag:
        case proton_Signal_list_float_value_tag:
        case proton_Signal_list_int32_value_tag:
        case proton_Signal_list_int64_value_tag:
        case proton_Signal_list_uint32_value_tag:
        case proton_Signal_list_uint64_value_tag:
        case proton_Signal_list_bool_value_tag:
        case proton_Signal_list_string_value_tag:
        case proton_Signal_list_bytes_value_tag:
        case proton_Signal_string_value_tag:
        case proton_Signal_bytes_value_tag:
        {
          // Do not initialise void * fields
          if (!pb_decode_ex(istream, proton_Signal_fields, signal, PB_DECODE_NOINIT))
          {
            return false;
          }
          break;
        }

        default:
        {
          if (!pb_decode(istream, proton_Signal_fields, signal))
          {
            return false;
          }
          break;
        }
      }
    }
    return true;
  }
  // Encode
  else if (ostream)
  {
    if (field->tag == proton_Bundle_signals_tag)
    {
      for (size_t i = 0; i < signal_list->length; i++)
      {
        if (!pb_encode_tag_for_field(ostream, field))
        {
          return false;
        }
        if (!pb_encode_submessage(ostream, proton_Signal_fields, &(signal_handles[i].signal)))
        {
          return false;
        }
      }
    }
    return true;
  }

  return false;
}

bool proton_Signal_callback(pb_istream_t *istream, pb_ostream_t *ostream, const pb_field_t *field)
{
  if (!field)
  {
    return false;
  }

  proton_Signal * msg = (proton_Signal *)field->message;

  if (!msg)
  {
    return false;
  }

  // Decode
  if (istream)
  {
    if (field->tag == proton_Signal_string_value_tag)
    {
      proton_list_t * arg = (proton_list_t *)msg->signal.string_value;

      if (!arg)
      {
        return false;
      }

      size_t len = istream->bytes_left;

      // Check that string is not larger than buffer
      if (len > arg->capacity - 1)
      {
        return false;
      }

      if (!pb_read(istream, (pb_byte_t *)arg->data, len))
      {
        return false;
      }

      arg->size = len;
    }
    else if (field->tag == proton_Signal_bytes_value_tag)
    {
      proton_list_t * arg = (proton_list_t *)msg->signal.bytes_value;

      if (!arg)
      {
        return false;
      }

      size_t len = istream->bytes_left;

      // Check that bytes array is not larger than buffer
      if (len > arg->capacity)
      {
        return false;
      }

      if (!pb_read(istream, (uint8_t *)arg->data, len))
      {
        return false;
      }

      arg->size = len;
    }
  }
  // Encode
  else if (ostream)
  {
    if (field->tag == proton_Signal_string_value_tag)
    {
      proton_list_t * arg = (proton_list_t *)msg->signal.string_value;

      if (!arg)
      {
        return false;
      }

      char * string = (char *)arg->data;

      if (!pb_encode_tag_for_field(ostream, field))
      {
        return false;
      }

      if (!pb_encode_string(ostream, (pb_byte_t *)string, strlen(string)))
      {
        return false;
      }
    }
    else if (field->tag == proton_Signal_bytes_value_tag)
    {
      proton_list_t * arg = (proton_list_t *)msg->signal.bytes_value;

      if (!arg)
      {
        return false;
      }

      if (!pb_encode_tag_for_field(ostream, field))
      {
        return false;
      }

      if (!pb_encode_string(ostream, (uint8_t *)arg->data, arg->capacity))
      {
        return false;
      }
    }
  }

  return true;
}

bool proton_ListDoubles_callback(pb_istream_t *istream, pb_ostream_t *ostream, const pb_field_t *field)
{
  if (!field)
  {
    return false;
  }

  proton_ListDoubles * msg = (proton_ListDoubles *)field->message;

  if (!msg)
  {
    return false;
  }

  proton_list_t * arg = (proton_list_t *)msg->doubles;

  if (!arg)
  {
    return false;
  }

  // Decode
  if (istream && field)
  {
    double * value = &(((double *)arg->data)[arg->size++]);
    if (arg->size == arg->length)
    {
      arg->size = 0;
    }

    if (!pb_decode_fixed64(istream, value))
    {
      return false;
    }
  }
  // Encode
  else if (ostream && field)
  {
    if (field->tag == proton_ListDoubles_doubles_tag)
    {
      for (size_t i = 0; i < arg->length; i++)
      {
        if (!pb_encode_tag_for_field(ostream, field))
        {
          return false;
        }
        if (!pb_encode_fixed64(ostream, &(((double *)arg->data)[i])))
        {
          return false;
        }
      }
    }
  }

  return true;
}

bool proton_ListFloats_callback(pb_istream_t *istream, pb_ostream_t *ostream, const pb_field_t *field)
{
  if (!field)
  {
    return false;
  }

  proton_ListFloats * msg = (proton_ListFloats *)field->message;

  if (!msg)
  {
    return false;
  }

  proton_list_t * arg = (proton_list_t *)msg->floats;

  if (!arg)
  {
    return false;
  }

  // Decode
  if (istream && field)
  {
    float * value = &(((float *)arg->data)[arg->size++]);
    if (arg->size == arg->length)
    {
      arg->size = 0;
    }

    if (!pb_decode_fixed32(istream, value))
    {
      return false;
    }
  }
  // Encode
  else if (ostream && field)
  {
    if (field->tag == proton_ListFloats_floats_tag)
    {
      for (size_t i = 0; i < arg->length; i++)
      {
        if (!pb_encode_tag_for_field(ostream, field))
        {
          return false;
        }
        if (!pb_encode_fixed32(ostream, &(((float *)arg->data)[i])))
        {
          return false;
        }
      }
    }
  }

  return true;
}

bool proton_ListInt32s_callback(pb_istream_t *istream, pb_ostream_t *ostream, const pb_field_t *field)
{
  if (!field)
  {
    return false;
  }

  proton_ListInt32s * msg = (proton_ListInt32s *)field->message;

  if (!msg)
  {
    return false;
  }

  proton_list_t * arg = (proton_list_t *)msg->int32s;

  if (!arg)
  {
    return false;
  }

  // Decode
  if (istream && field)
  {
    int32_t * value = &(((int32_t *)arg->data)[arg->size++]);
    if (arg->size == arg->length)
    {
      arg->size = 0;
    }

    if (!pb_decode_varint32(istream, (uint32_t *)value))
    {
      return false;
    }
  }
  // Encode
  else if (ostream && field)
  {
    if (field->tag == proton_ListInt32s_int32s_tag)
    {
      for (size_t i = 0; i < arg->length; i++)
      {
        if (!pb_encode_tag_for_field(ostream, field))
        {
          return false;
        }
        if (!pb_encode_varint(ostream, ((int32_t *)arg->data)[i]))
        {
          return false;
        }
      }
    }
  }

  return true;
}

bool proton_ListInt64s_callback(pb_istream_t *istream, pb_ostream_t *ostream, const pb_field_t *field)
{
  if (!field)
  {
    return false;
  }

  proton_ListInt64s * msg = (proton_ListInt64s *)field->message;

  if (!msg)
  {
    return false;
  }

  proton_list_t * arg = (proton_list_t *)msg->int64s;

  if (!arg)
  {
    return false;
  }

  // Decode
  if (istream && field)
  {
    int64_t * value = &(((int64_t *)arg->data)[arg->size++]);
    if (arg->size == arg->length)
    {
      arg->size = 0;
    }

    if (!pb_decode_varint(istream, (uint64_t *)value))
    {
      return false;
    }
  }
  // Encode
  else if (ostream && field)
  {
    if (field->tag == proton_ListInt64s_int64s_tag)
    {
      for (size_t i = 0; i < arg->length; i++)
      {
        if (!pb_encode_tag_for_field(ostream, field))
        {
          return false;
        }
        if (!pb_encode_varint(ostream, ((uint64_t *)arg->data)[i]))
        {
          return false;
        }
      }
    }
  }

  return true;
}

bool proton_ListUint32s_callback(pb_istream_t *istream, pb_ostream_t *ostream, const pb_field_t *field)
{
  if (!field)
  {
    return false;
  }

  proton_ListUint32s * msg = (proton_ListUint32s *)field->message;

  if (!msg)
  {
    return false;
  }

  proton_list_t * arg = (proton_list_t *)msg->uint32s;

  if (!arg)
  {
    return false;
  }

  // Decode
  if (istream && field)
  {
    uint32_t * value = &(((uint32_t *)arg->data)[arg->size++]);
    if (arg->size == arg->length)
    {
      arg->size = 0;
    }

    if (!pb_decode_varint32(istream, value))
    {
      return false;
    }
  }
  // Encode
  else if (ostream && field)
  {
    if (field->tag == proton_ListUint32s_uint32s_tag)
    {
      for (size_t i = 0; i < arg->length; i++)
      {
        if (!pb_encode_tag_for_field(ostream, field))
        {
          return false;
        }
        if (!pb_encode_varint(ostream, ((uint32_t *)arg->data)[i]))
        {
          return false;
        }
      }
    }
  }

  return true;
}

bool proton_ListUint64s_callback(pb_istream_t *istream, pb_ostream_t *ostream, const pb_field_t *field)
{
  if (!field)
  {
    return false;
  }

  proton_ListUint64s * msg = (proton_ListUint64s *)field->message;

  if (!msg)
  {
    return false;
  }

  proton_list_t * arg = (proton_list_t *)msg->uint64s;

  if (!arg)
  {
    return false;
  }

  // Decode
  if (istream && field)
  {
    uint64_t * value = &(((uint64_t *)arg->data)[arg->size++]);
    if (arg->size == arg->length)
    {
      arg->size = 0;
    }

    if (!pb_decode_varint(istream, value))
    {
      return false;
    }
  }
  // Encode
  else if (ostream && field)
  {
    if (field->tag == proton_ListUint64s_uint64s_tag)
    {
      for (size_t i = 0; i < arg->length; i++)
      {
        if (!pb_encode_tag_for_field(ostream, field))
        {
          return false;
        }
        if (!pb_encode_varint(ostream, ((uint64_t *)arg->data)[i]))
        {
          return false;
        }
      }
    }
  }

  return true;
}

bool proton_ListBools_callback(pb_istream_t *istream, pb_ostream_t *ostream, const pb_field_t *field)
{
  if (!field)
  {
    return false;
  }

  proton_ListBools * msg = (proton_ListBools *)field->message;

  if (!msg)
  {
    return false;
  }

  proton_list_t * arg = (proton_list_t *)msg->bools;

  if (!arg)
  {
    return false;
  }

  // Decode
  if (istream && field)
  {
    bool * value = &(((bool *)arg->data)[arg->size++]);
    if (arg->size == arg->length)
    {
      arg->size = 0;
    }

    if (!pb_decode_bool(istream, value))
    {
      return false;
    }
  }
  // Encode
  else if (ostream && field)
  {
    if (field->tag == proton_ListBools_bools_tag)
    {
      for (size_t i = 0; i < arg->length; i++)
      {
        if (!pb_encode_tag_for_field(ostream, field))
        {
          return false;
        }
        if (!pb_encode_varint(ostream, ((bool *)arg->data)[i]))
        {
          return false;
        }
      }
    }
  }

  return true;
}

bool proton_ListStrings_callback(pb_istream_t *istream, pb_ostream_t *ostream, const pb_field_t *field)
{
  if (!field)
  {
    return false;
  }

  proton_ListStrings * msg = (proton_ListStrings *)field->message;

  if (!msg)
  {
    return false;
  }

  // Decode
  if (istream)
  {
    if (field->tag == proton_ListStrings_strings_tag)
    {
      proton_list_t * arg = (proton_list_t *)msg->strings;

      if (!arg)
      {
        return false;
      }

      size_t len = istream->bytes_left;

      char * string = ((char (*)[arg->capacity])arg->data)[arg->size++];

      if (string == NULL)
      {
        return false;
      }

      if (arg->size == arg->length)
      {
        arg->size = 0;
      }

      if (!pb_read(istream, (pb_byte_t *)string, len))
      {
        return false;
      }
    }
  }
  // Encode
  else if (ostream)
  {
    if (field->tag == proton_ListStrings_strings_tag)
    {
      proton_list_t * arg = (proton_list_t *)msg->strings;

      if (!arg)
      {
        return false;
      }

      for (int i=0; i < arg->length; i++)
      {
        char * string = ((char (*)[arg->capacity])arg->data)[i];

        if (string == NULL)
        {
          return false;
        }

        if (!pb_encode_tag_for_field(ostream, field))
        {
          return false;
        }

        if (!pb_encode_string(ostream, (pb_byte_t *)string, strlen(string)))
        {
          return false;
        }
      }
    }
  }

  return true;
}

bool proton_ListBytes_callback(pb_istream_t *istream, pb_ostream_t *ostream, const pb_field_t *field)
{
  if (!field)
  {
    return false;
  }

  proton_ListBytes * msg = (proton_ListBytes *)field->message;

  if (!msg)
  {
    return false;
  }

  // Decode
  if (istream)
  {
    if (field->tag == proton_ListBytes_bytes_tag)
    {
      proton_list_t * arg = (proton_list_t *)msg->bytes;

      if (!arg)
      {
        return false;
      }

      size_t len = istream->bytes_left;

      uint8_t * bytes = ((uint8_t (*)[arg->capacity])arg->data)[arg->size++];

      if (bytes == NULL)
      {
        return false;
      }

      if (arg->size == arg->length)
      {
        arg->size = 0;
      }

      if (!pb_read(istream, bytes, len))
      {
        return false;
      }
    }
  }
  // Encode
  else if (ostream)
  {
    if (field->tag == proton_ListBytes_bytes_tag)
    {
      proton_list_t * arg = (proton_list_t *)msg->bytes;

      if (!arg)
      {
        return false;
      }

      for (int i=0; i < arg->length; i++)
      {
        uint8_t * bytes = ((uint8_t (*)[arg->capacity])arg->data)[i];

        if (bytes == NULL)
        {
          return false;
        }

        if (!pb_encode_tag_for_field(ostream, field))
        {
          return false;
        }

        if (!pb_encode_string(ostream, bytes, arg->capacity))
        {
          return false;
        }
      }
    }
  }

  return true;
}
