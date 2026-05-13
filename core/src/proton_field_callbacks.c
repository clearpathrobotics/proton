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

#include <stddef.h>
#include <string.h>
#include "pb.h"
#include "pb_decode.h"
#include "pb_encode.h"
#include "proton/common.h"
#include "proton/registry.h"

extern uint8_t * g_signal_decode_buffers[PROTON_SIGNAL_REGISTRY_SIZE];
extern const size_t g_signal_max_capacity[PROTON_SIGNAL_REGISTRY_SIZE];

// Scratch buffer used as a temporary decode target for string/bytes signals
static uint8_t g_signal_decode_scratch[PROTON_MAX_MESSAGE_SIZE];

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
  const bundle_desc_t * bundle_desc = proton_registry_get_bundle(bundle_id, NULL);

  if (!bundle_desc)
  {
    return false;
  }

  size_t signal_count = bundle_desc->signal_ids.count;

  // Decode
  if (istream)
  {
    proton_Signal * bundle_signals = proton_registry_get_bundle_signals(bundle_id);
    if (bundle_signals == NULL)
    {
      return false;
    }

    if (field->tag == proton_Bundle_signals_tag)
    {
      // Decode into a temporary with the scratch buffer pre-set for string/bytes,
      // so proton_Signal_callback has a valid destination without knowing the type yet.
      proton_Signal incoming = proton_Signal_init_zero;
      incoming.signal.string_value =
        g_signal_decode_scratch;  // covers bytes_value too (same union slot)
      if (!pb_decode_ex(istream, proton_Signal_fields, &incoming, PB_DECODE_NOINIT))
      {
        return false;
      }
      uint32_t incoming_id = incoming.id;

      // Validate: the incoming signal ID must belong to this bundle's definition
      bool id_in_bundle = false;
      size_t bundle_signal_slot = SIZE_MAX;
      for (size_t i = 0; i < signal_count; i++)
      {
        if (bundle_desc->signal_ids.ids[i] == incoming_id)
        {
          id_in_bundle = true;
          bundle_signal_slot = i;
          break;
        }
      }

      if (!id_in_bundle)
      {
        return false;
      }

      // One lookup: get signal registry index for decode buffers
      size_t signal_registry_idx = SIZE_MAX;
      if (proton_registry_get_signal(incoming_id, &signal_registry_idx) == NULL)
      {
        return false;
      }

      // Route the decoded value to the correct slot in the bundle signal array
      proton_Signal * signal = &bundle_signals[bundle_signal_slot];
      signal->id = incoming.id;
      signal->which_signal = incoming.which_signal;

      switch (incoming.which_signal)
      {
        case proton_Signal_string_value_tag:
        {
          char * decode_buf = (char *)g_signal_decode_buffers[signal_registry_idx];
          if (decode_buf == NULL)
          {
            return false;
          }
          size_t capacity = g_signal_max_capacity[signal_registry_idx];
          memcpy(decode_buf, g_signal_decode_scratch, capacity);
          signal->signal.string_value = decode_buf;
          break;
        }
        case proton_Signal_bytes_value_tag:
        {
          uint8_t * decode_buf = g_signal_decode_buffers[signal_registry_idx];
          if (decode_buf == NULL)
          {
            return false;
          }
          size_t capacity = g_signal_max_capacity[signal_registry_idx];
          memcpy(decode_buf, g_signal_decode_scratch, capacity);
          signal->signal.bytes_value = decode_buf;
          break;
        }
        default:
        {
          memcpy(&signal->signal, &incoming.signal, sizeof(incoming.signal));
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
      for (size_t i = 0; i < signal_count; i++)
      {
        uint32_t signal_id = bundle_desc->signal_ids.ids[i];
        signal_desc_t * desc = proton_registry_get_signal(signal_id, NULL);
        if (desc == NULL)
        {
          return false;
        }
        if (!pb_encode_tag_for_field(ostream, field))
        {
          return false;
        }
        if (!pb_encode_submessage(ostream, proton_Signal_fields, &desc->signal))
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
      signal->which_signal = proton_Signal_string_value_tag;
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
      signal->which_signal = proton_Signal_bytes_value_tag;
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
