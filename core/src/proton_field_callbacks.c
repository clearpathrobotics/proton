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

/**
 * Callback for encoding/decoding proton_Signal message.
 * Note that this was created for both encode/decode to handle the buffer management for string and bytes types
 * @param istream protobuf input stream, will be non-NULL when decoding
 * @param ostream protobuf output stream, will be non-NULL when encoding
 * @param field protobuf field being encoded/decoded. Used to find whether the signal type is string or bytes
 * @return true if successful, false if error
 */
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
