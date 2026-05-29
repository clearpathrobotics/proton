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
 * @author Tom Wallis (thomas.wallis@rockwellautomation.com)
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "proton/common.h"
#include "proton/registry.h"

#include "pb.h"
#include "pb_decode.h"
#include "pb_encode.h"

/**
 * Callback for encoding a bundle, passing the registry as arg to access bundle ID and signals
 * @param ostream protobuf output stream
 * @param field protobuf field being encoded, used to get the bundle ID
 * @param arg pointer to the proton registry, used to look up the bundle definition and signals
 * @return true if successful, false if error
 */
static bool proton_encode_bundle_cb(
  pb_ostream_t * ostream, const pb_field_t * field, void * const * arg)
{
  if (!field || !arg)
  {
    return false;
  }

  proton_Bundle * msg = (proton_Bundle *)field->message;
  if (!msg)
  {
    return false;
  }

  const proton_registry_t * registry = (const proton_registry_t *)*arg;
  uint32_t bundle_id = msg->id;

  const bundle_desc_t * bundle_desc = proton_registry_get_bundle(registry, bundle_id, NULL);
  if (!bundle_desc)
  {
    return false;
  }

  for (size_t i = 0; i < bundle_desc->signal_ids.count; i++)
  {
    uint32_t signal_id = bundle_desc->signal_ids.ids[i];
    signal_desc_t * desc = proton_registry_get_signal(registry, signal_id, NULL);
    proton_Signal signal_msg = proton_Signal_init_zero;
    signal_msg.id = signal_id;
    signal_msg.which_signal = desc->signal.which_signal;
    proton_buffer_t string_buf;
    if (
      signal_msg.which_signal == proton_Signal_string_value_tag ||
      signal_msg.which_signal == proton_Signal_bytes_value_tag)
    {
      string_buf.data = (uint8_t *)desc->signal.signal.string_value;
      string_buf.len = desc->value_size;
      signal_msg.signal.string_value = &string_buf;
    }
    else
    {
      memcpy(&signal_msg.signal, &desc->signal.signal, desc->value_size);
    }

    if (desc == NULL)
    {
      return false;
    }
    if (!pb_encode_tag_for_field(ostream, field))
    {
      return false;
    }
    if (!pb_encode_submessage(ostream, proton_Signal_fields, &signal_msg))
    {
      return false;
    }
  }
  return true;
}

/**
 * Callback for decoding a bundle, passing the registry as arg to access bundle definition and route decoded signals
 * @param istream protobuf input stream
 * @param field protobuf field being decoded, used to get the bundle ID from the message
 * @param arg pointer to the proton registry, used to look up the bundle definition and signals
 * @return true if successful, false if error
 */
static bool proton_decode_bundle_cb(pb_istream_t * istream, const pb_field_t * field, void ** arg)
{
  if (!field || !arg)
  {
    return false;
  }

  proton_Bundle * msg = (proton_Bundle *)field->message;
  if (!msg)
  {
    return false;
  }

  uint32_t bundle_id = msg->id;
  const proton_registry_t * registry = *(const proton_registry_t **)arg;
  if (!registry)
  {
    return false;
  }

  const bundle_desc_t * bundle_desc = proton_registry_get_bundle(registry, bundle_id, NULL);

  if (!bundle_desc)
  {
    return false;
  }

  size_t signal_count = bundle_desc->signal_ids.count;
  proton_Signal * bundle_signals =
    proton_registry_get_bundle_encode_decode_buffer(registry, bundle_id);

  if (bundle_signals == NULL)
  {
    return false;
  }

  if (field->tag == proton_Bundle_signals_tag)
  {
    // Decode into a temporary with the scratch buffer pre-set for string/bytes,
    // so proton_Signal_callback has a valid destination without knowing the type yet.
    proton_buffer_t scratch_buf = {
      .data = registry->signal_scratch_buffer,
      .len = registry->signal_scratch_buffer_size,
    };
    proton_Signal incoming = proton_Signal_init_zero;
    incoming.signal.string_value = &scratch_buf;
    if (!pb_decode_ex(istream, proton_Signal_fields, &incoming, PB_DECODE_NOINIT))
    {
      return false;
    }
    uint32_t incoming_id = incoming.id;

    // Validate: the incoming signal ID must belong to this bundle
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

    // get signal registry index for decode buffers
    size_t signal_registry_idx = SIZE_MAX;
    if (proton_registry_get_signal(registry, incoming_id, &signal_registry_idx) == NULL)
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
        char * decode_buf = (char *)registry->signal_decode_buffers[signal_registry_idx];
        if (decode_buf == NULL)
        {
          return false;
        }
        size_t capacity = registry->signal_max_capacity[signal_registry_idx];
        if (scratch_buf.len > capacity)
        {
          return false;
        }
        memcpy(decode_buf, scratch_buf.data, scratch_buf.len);
        signal->signal.string_value = decode_buf;
        break;
      }
      case proton_Signal_bytes_value_tag:
      {
        uint8_t * decode_buf = registry->signal_decode_buffers[signal_registry_idx];
        if (decode_buf == NULL)
        {
          return false;
        }
        size_t capacity = registry->signal_max_capacity[signal_registry_idx];
        if (scratch_buf.len > capacity)
        {
          return false;
        }
        memcpy(decode_buf, scratch_buf.data, scratch_buf.len);
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

/**
 * Callback for decoding the oneof field in a Proton message, used to pass args to the various different operations
 */
static bool proton_operation_decode_cb(
  pb_istream_t * istream, const pb_field_t * field, void ** arg)
{
  (void)istream;

  if (!field || !arg)
  {
    return false;
  }

  proton_Proton * msg = (proton_Proton *)field->message;
  if (!msg)
  {
    return false;
  }

  switch (field->tag)
  {
    case proton_Proton_bundle_tag:
      msg->operation.bundle.signals.funcs.decode = proton_decode_bundle_cb;
      msg->operation.bundle.signals.arg = *arg;
      return true;
    default:
      return false;
  }

  return false;
}

static proton_status_e check_stream_bytes_left(const pb_istream_t * stream)
{
  if (stream->bytes_left == 0)
  {
    return PROTON_OK;
  }
  else
  {
    return PROTON_SERIALIZATION_ERROR;
  }
}

static proton_status_e proton_decode_bundle(
  proton_registry_t * registry, proton_Bundle * bundle, const pb_istream_t * stream)
{
  size_t bundle_slot = 0;
  const bundle_desc_t * bundle_desc =
    proton_registry_get_bundle(registry, bundle->id, &bundle_slot);
  // This bundle doesn't exist
  if (bundle_desc == NULL)
  {
    return PROTON_ERROR;
  }

  // Set signals in registry based on decoded values
  proton_Signal * bundle_signals =
    proton_registry_get_bundle_encode_decode_buffer(registry, bundle->id);
  for (size_t i = 0; i < bundle_desc->signal_ids.count; i++)
  {
    proton_Signal * signal_ptr = &bundle_signals[i];
    uint32_t signal_id = bundle_desc->signal_ids.ids[i];
    signal_desc_t * desc = proton_registry_get_signal(registry, signal_id, NULL);
    if (desc == NULL)
    {
      return PROTON_ERROR;
    }
    if (desc->type != proton_get_type_from_tag(signal_ptr->which_signal))
    {
      return PROTON_ERROR;
    }
    if (desc->type == PROTON_STRING || desc->type == PROTON_BYTES)
    {
      // value pointer in union points to the decode buffer — copy content into registry
      const void * src = signal_ptr->signal.string_value;  // same union slot as bytes_value
      memcpy(desc->signal.signal.string_value, src, desc->value_size);
    }
    else
    {
      memcpy(&desc->signal.signal, &signal_ptr->signal, desc->value_size);
    }
  }

  return check_stream_bytes_left(stream);
}

proton_status_e proton_encode_bundle(
  proton_registry_t * registry, uint32_t bundle_id, uint8_t * buffer, size_t buffer_len,
  size_t * bytes_encoded)
{
  if (registry == NULL || buffer == NULL || bytes_encoded == NULL)
  {
    return PROTON_NULL_PTR_ERROR;
  }

  // Find bundle in registry and get its slot index in one search
  const bundle_desc_t * bundle_desc = proton_registry_get_bundle(registry, bundle_id, NULL);
  if (bundle_desc == NULL)
  {
    return PROTON_ERROR;
  }

  // Get a bundle
  proton_Bundle bundle = {
    .id = bundle_desc->bundle_id,
    // Place registry at the signals pointer so it can be accessed in the callback
    .signals.funcs.encode = proton_encode_bundle_cb,
    .signals.arg = registry,
  };

  // Encode bundle as part of Proton message
  proton_Proton proton_message = {
    .which_operation = proton_Proton_bundle_tag,
    .operation.bundle = bundle,
  };

  pb_ostream_t stream = pb_ostream_from_buffer((pb_byte_t *)buffer, buffer_len);

  bool status = pb_encode(&stream, proton_Proton_fields, &proton_message);

  if (status)
  {
    *bytes_encoded = stream.bytes_written;
    return PROTON_OK;
  }
  else
  {
    return PROTON_SERIALIZATION_ERROR;
  }

  return PROTON_OK;
}

proton_status_e proton_decode(
  proton_registry_t * registry, const uint8_t * buffer, size_t buffer_len,
  proton_Proton * decoded_msg)
{
  if (registry == NULL || buffer == NULL || decoded_msg == NULL)
  {
    return PROTON_NULL_PTR_ERROR;
  }

  pb_istream_t stream = pb_istream_from_buffer((const pb_byte_t *)buffer, buffer_len);
  // Place registry at the signals pointer so it can be accessed in the callback
  decoded_msg->cb_operation.funcs.decode = proton_operation_decode_cb;
  decoded_msg->cb_operation.arg = registry;

  bool status = pb_decode(&stream, proton_Proton_fields, decoded_msg);
  if (!status)
  {
    return PROTON_SERIALIZATION_ERROR;
  }

  // No other operations are defined yet
  if (decoded_msg->which_operation == proton_Proton_bundle_tag)
  {
    proton_Bundle bundle = decoded_msg->operation.bundle;
    return proton_decode_bundle(registry, &bundle, &stream);
  }
  else
  {
    return PROTON_UNSUPPORTED_OPERATION_ERROR;
  }
}
