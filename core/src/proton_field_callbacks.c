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

#include "pb.h"
#include "pb_decode.h"
#include "pb_encode.h"
#include "proton/registry.h"

extern proton_Signal * g_bundle_signal_ptrs[PROTON_BUNDLE_REGISTRY_SIZE];
extern const id_to_index_t g_bundle_id_lut[PROTON_BUNDLE_REGISTRY_SIZE];
extern const id_to_index_t g_signal_id_lut[PROTON_SIGNAL_REGISTRY_SIZE];
extern uint8_t * g_signal_decode_buffers[PROTON_SIGNAL_REGISTRY_SIZE];

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

  uint32_t bundle_id = msg->id;
  const bundle_desc_t * bundle_desc = proton_registry_get_bundle(msg->id);

  if (!bundle_desc)
  {
    return false;
  }

  size_t signal_count = bundle_desc->signal_ids.count;

  // Decode
  if (istream)
  {
    for (size_t i = 0; i < PROTON_ARRAY_SIZE(g_bundle_id_lut); i++)
    {
      if (g_bundle_id_lut[i].id == bundle_id)
      {
        msg->signals = g_bundle_signal_ptrs[g_bundle_id_lut[i].idx];
        break;
      }
    }

    proton_Signal * signal_list = (proton_Signal *)msg->signals;
    if (field->tag == proton_Bundle_signals_tag)
    {
      for (size_t i = 0; i < signal_count; i++)
      {
        proton_Signal * signal = &signal_list[i];
        uint32_t signal_id = bundle_desc->signal_ids.ids[i];
        signal_desc_t signal_desc;
        if (!proton_registry_get_signal(signal_id, &signal_desc))
        {
          return false;
        }

        signal->id = signal_id;
        signal->which_signal = signal_desc.signal.which_signal;

        switch (signal->which_signal)
        {
          // Set the decode buffer to something that is large enough for the signal, so that pb_decode
          // can write to it without modifying the actual registry
          case proton_Signal_bytes_value_tag:
          case proton_Signal_string_value_tag:
          {
            for (size_t j = 0; j < PROTON_ARRAY_SIZE(g_signal_id_lut); j++)
            {
              if (g_signal_id_lut[j].id == signal->id)
              {
                if (g_signal_decode_buffers[g_signal_id_lut[j].idx] == NULL)
                {
                  return false;
                }
                if (signal->which_signal == proton_Signal_string_value_tag)
                {
                  signal->signal.string_value =
                    (char *)g_signal_decode_buffers[g_signal_id_lut[j].idx];
                }
                else
                {
                  signal->signal.bytes_value = g_signal_decode_buffers[g_signal_id_lut[j].idx];
                }
                break;
              }
            }
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
    }
    return true;
  }
  // Encode
  else if (ostream)
  {
    if (field->tag == proton_Bundle_signals_tag)
    {
      for (size_t i = 0; i < signal_count; i++)
      {
        uint32_t signal_id = bundle_desc->signal_ids.ids[i];
        signal_desc_t signal_desc;
        if (!proton_registry_get_signal(signal_id, &signal_desc))
        {
          return false;
        }
        proton_Signal * signal = &signal_desc.signal;
        if (!pb_encode_tag_for_field(ostream, field))
        {
          return false;
        }
        if (!pb_encode_submessage(ostream, proton_Signal_fields, signal))
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

  proton_Signal * signal = (proton_Signal *)field->message;
  if (!signal)
  {
    return false;
  }

  // Decode
  if (istream)
  {
    size_t len = istream->bytes_left;

    if (field->tag == proton_Signal_string_value_tag)
    {
      size_t max_capacity = PROTON_ARRAY_SIZE(signal->signal.string_value);
      // Check that string is not larger than buffer
      if (len > max_capacity - 1)
      {
        return false;
      }

      if (!pb_read(istream, (pb_byte_t *)signal->signal.string_value, len))
      {
        return false;
      }
    }
    else if (field->tag == proton_Signal_bytes_value_tag)
    {
      size_t max_capacity = PROTON_ARRAY_SIZE(signal->signal.bytes_value);
      // Check that bytes array is not larger than buffer
      if (len > max_capacity)
      {
        return false;
      }

      if (!pb_read(istream, (uint8_t *)signal->signal.bytes_value, len))
      {
        return false;
      }
    }
  }
  // Encode
  else if (ostream)
  {
    if (field->tag == proton_Signal_string_value_tag)
    {
      size_t max_capacity = PROTON_ARRAY_SIZE(signal->signal.string_value);
      char * string = (char *)signal->signal.string_value;

      if (!pb_encode_tag_for_field(ostream, field))
      {
        return false;
      }

      if (!pb_encode_string(ostream, (pb_byte_t *)string, strnlen(string, max_capacity)))
      {
        return false;
      }
    }
    else if (field->tag == proton_Signal_bytes_value_tag)
    {
      size_t max_capacity = PROTON_ARRAY_SIZE(signal->signal.bytes_value);

      if (!pb_encode_tag_for_field(ostream, field))
      {
        return false;
      }

      if (!pb_encode_string(ostream, (uint8_t *)signal->signal.bytes_value, max_capacity))
      {
        return false;
      }
    }
  }

  return true;
}
