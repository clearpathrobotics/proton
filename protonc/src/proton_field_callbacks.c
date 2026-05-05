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

#include <stdio.h>

#include "protonc/proton.h"

bool proton_Bundle_callback(
  pb_istream_t * istream, pb_ostream_t * ostream, const pb_field_t * field)
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

      switch (signal->which_signal)
      {
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

bool proton_Signal_callback(
  pb_istream_t * istream, pb_ostream_t * ostream, const pb_field_t * field)
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
