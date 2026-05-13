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
#include "target_registry_sizes.h"

#include "pb.h"
#include "pb_decode.h"
#include "pb_encode.h"

proton_status_e proton_encode_bundle(
  uint32_t bundle_id, proton_buffer_t buffer, size_t * bytes_encoded)
{
  if (buffer.data == NULL || bytes_encoded == NULL)
  {
    return PROTON_NULL_PTR_ERROR;
  }

  // Find bundle in registry and get its slot index in one search
  const bundle_desc_t * bundle_desc = proton_registry_get_bundle(bundle_id, NULL);
  if (bundle_desc == NULL)
  {
    return PROTON_ERROR;
  }

  // Get a bundle
  proton_Bundle bundle = {
    .id = bundle_desc->bundle_id,
    .signals = proton_registry_get_bundle_signals(bundle_id),
  };

  // Encode bundle as part of Proton message
  proton_Proton proton_message = {
    .which_operation = proton_Proton_bundle_tag,
    .operation.bundle = bundle,
  };

  pb_ostream_t stream = pb_ostream_from_buffer((pb_byte_t *)buffer.data, buffer.len);

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

proton_status_e proton_decode_bundle(proton_buffer_t buffer)
{
  if (buffer.data == NULL)
  {
    return PROTON_NULL_PTR_ERROR;
  }

  pb_istream_t stream = pb_istream_from_buffer((const pb_byte_t *)buffer.data, buffer.len);
  proton_Proton proton_message;
  bool status = pb_decode(&stream, proton_Proton_fields, &proton_message);
  if (!status)
  {
    return PROTON_SERIALIZATION_ERROR;
  }

  // No other operations are defined yet
  if (proton_message.which_operation != proton_Proton_bundle_tag)
  {
    return PROTON_ERROR;
  }

  proton_Bundle bundle = proton_message.operation.bundle;
  size_t bundle_slot = 0;
  const bundle_desc_t * bundle_desc = proton_registry_get_bundle(bundle.id, &bundle_slot);
  // This bundle doesn't exist
  if (bundle_desc == NULL)
  {
    return PROTON_ERROR;
  }

  // Set signals in registry based on decoded values
  proton_Signal * bundle_signals = proton_registry_get_bundle_signals(bundle.id);
  for (size_t i = 0; i < bundle_desc->signal_ids.count; i++)
  {
    proton_Signal * signal_ptr = &bundle_signals[i];
    uint32_t signal_id = bundle_desc->signal_ids.ids[i];
    signal_desc_t * desc = proton_registry_get_signal(signal_id, NULL);
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

  if (stream.bytes_left == 0)
  {
    return PROTON_OK;
  }
  else
  {
    return PROTON_SERIALIZATION_ERROR;
  }
}
