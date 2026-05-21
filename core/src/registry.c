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

#include "proton/registry.h"
#include <string.h>

proton_signal_type_e proton_get_type_from_tag(pb_size_t tag)
{
  switch (tag)
  {
    case proton_Signal_double_value_tag:
      return PROTON_DOUBLE;
    case proton_Signal_float_value_tag:
      return PROTON_FLOAT;
    case proton_Signal_int32_value_tag:
      return PROTON_INT32;
    case proton_Signal_int64_value_tag:
      return PROTON_INT64;
    case proton_Signal_uint32_value_tag:
      return PROTON_UINT32;
    case proton_Signal_uint64_value_tag:
      return PROTON_UINT64;
    case proton_Signal_bool_value_tag:
      return PROTON_BOOL;
    case proton_Signal_string_value_tag:
      return PROTON_STRING;
    case proton_Signal_bytes_value_tag:
      return PROTON_BYTES;
    default:
      return PROTON_INVALID_TYPE;
  }
}

const bundle_desc_t * proton_registry_get_bundle(
  const proton_registry_t * registry, uint32_t bundle_id, size_t * slot_idx)
{
  for (size_t i = 0; i < registry->bundle_count; i++)
  {
    if (registry->bundle_id_lut[i].id == bundle_id)
    {
      size_t idx = registry->bundle_id_lut[i].idx;
      if (slot_idx)
      {
        *slot_idx = idx;
      }
      return &registry->bundle_table[idx];
    }
  }

  return NULL;
}

proton_Signal * proton_registry_get_bundle_encode_decode_buffer(
  const proton_registry_t * registry, uint32_t bundle_id)
{
  for (size_t i = 0; i < registry->bundle_count; i++)
  {
    if (registry->bundle_id_lut[i].id == bundle_id)
    {
      return registry->bundle_signal_ptrs[registry->bundle_id_lut[i].idx];
    }
  }

  return NULL;
}

proton_bundle_cb_t * proton_registry_get_bundle_callback(
  const proton_registry_t * registry, uint32_t bundle_id)
{
  for (size_t i = 0; i < registry->bundle_count; i++)
  {
    if (registry->bundle_id_lut[i].id == bundle_id)
    {
      return &registry->bundle_callbacks[registry->bundle_id_lut[i].idx];
    }
  }

  return NULL;
}

void proton_registry_set_bundle_callback(
  proton_registry_t * registry, uint32_t bundle_id, proton_bundle_cb_f bundle_cb, void * context)
{
  for (size_t i = 0; i < registry->bundle_count; i++)
  {
    if (registry->bundle_id_lut[i].id == bundle_id)
    {
      registry->bundle_callbacks[registry->bundle_id_lut[i].idx].cb = bundle_cb;
      registry->bundle_callbacks[registry->bundle_id_lut[i].idx].arg = context;
    }
  }
}

signal_desc_t * proton_registry_get_signal(
  const proton_registry_t * registry, uint32_t signal_id, size_t * registry_idx)
{
  for (size_t i = 0; i < registry->signal_count; i++)
  {
    if (registry->signal_id_lut[i].id == signal_id)
    {
      size_t idx = registry->signal_id_lut[i].idx;
      if (registry_idx)
      {
        *registry_idx = idx;
      }
      return &registry->signal_registry[idx];
    }
  }

  return NULL;
}

/*
 * Typed scalar accessors. Each pair validates that the signal exists and that
 * its stored type matches the requested type before reading or writing.
 */
#define PROTON_DEFINE_SCALAR_ACCESSORS(NAME, CTYPE, ENUM_TAG, UNION_FIELD)        \
  proton_status_e proton_signal_get_##NAME(                                       \
    const proton_registry_t * registry, uint32_t signal_id, CTYPE * value)        \
  {                                                                               \
    if (registry == NULL || value == NULL)                                        \
    {                                                                             \
      return PROTON_NULL_PTR_ERROR;                                               \
    }                                                                             \
    signal_desc_t * desc = proton_registry_get_signal(registry, signal_id, NULL); \
    if (desc == NULL)                                                             \
    {                                                                             \
      return PROTON_ERROR;                                                        \
    }                                                                             \
    if (desc->signal.which_signal != (ENUM_TAG))                                  \
    {                                                                             \
      return PROTON_ERROR;                                                        \
    }                                                                             \
    *value = desc->signal.signal.UNION_FIELD;                                     \
    return PROTON_OK;                                                             \
  }                                                                               \
  proton_status_e proton_signal_set_##NAME(                                       \
    const proton_registry_t * registry, uint32_t signal_id, CTYPE value)          \
  {                                                                               \
    if (registry == NULL)                                                         \
    {                                                                             \
      return PROTON_NULL_PTR_ERROR;                                               \
    }                                                                             \
    signal_desc_t * desc = proton_registry_get_signal(registry, signal_id, NULL); \
    if (desc == NULL)                                                             \
    {                                                                             \
      return PROTON_ERROR;                                                        \
    }                                                                             \
    if (desc->signal.which_signal != (ENUM_TAG))                                  \
    {                                                                             \
      return PROTON_ERROR;                                                        \
    }                                                                             \
    desc->signal.signal.UNION_FIELD = value;                                      \
    return PROTON_OK;                                                             \
  }

PROTON_DEFINE_SCALAR_ACCESSORS(double, double, proton_Signal_double_value_tag, double_value)
PROTON_DEFINE_SCALAR_ACCESSORS(float, float, proton_Signal_float_value_tag, float_value)
PROTON_DEFINE_SCALAR_ACCESSORS(int32, int32_t, proton_Signal_int32_value_tag, int32_value)
PROTON_DEFINE_SCALAR_ACCESSORS(int64, int64_t, proton_Signal_int64_value_tag, int64_value)
PROTON_DEFINE_SCALAR_ACCESSORS(uint32, uint32_t, proton_Signal_uint32_value_tag, uint32_value)
PROTON_DEFINE_SCALAR_ACCESSORS(uint64, uint64_t, proton_Signal_uint64_value_tag, uint64_value)
PROTON_DEFINE_SCALAR_ACCESSORS(bool, bool, proton_Signal_bool_value_tag, bool_value)

#undef PROTON_DEFINE_SCALAR_ACCESSORS

/*
 * Internal helper for string/bytes get. The two only differ in which union slot
 * is read from, but the pointer occupies the same union storage, so we use
 * string_value as the access path.
 */
static proton_status_e proton_signal_get_buffer(
  const proton_registry_t * registry, uint32_t signal_id, pb_size_t expected_tag, void * buf,
  size_t capacity, size_t * out_len)
{
  if (registry == NULL || buf == NULL || out_len == NULL)
  {
    return PROTON_NULL_PTR_ERROR;
  }
  signal_desc_t * desc = proton_registry_get_signal(registry, signal_id, NULL);
  if (desc == NULL)
  {
    return PROTON_ERROR;
  }
  if (desc->signal.which_signal != expected_tag)
  {
    return PROTON_ERROR;
  }
  if (capacity < desc->value_size)
  {
    return PROTON_INSUFFICIENT_BUFFER_ERROR;
  }

  size_t string_capacity = desc->value_size;
  if (expected_tag == proton_Signal_string_value_tag)
  {
    string_capacity = strnlen(desc->signal.signal.string_value, desc->value_size);
    if (string_capacity < desc->value_size)
    {
      string_capacity += 1;  // Account for null terminator if not present
    }
  }

  memcpy(buf, desc->signal.signal.string_value, string_capacity);
  *out_len = string_capacity;
  return PROTON_OK;
}

proton_status_e proton_signal_get_string(
  const proton_registry_t * registry, uint32_t signal_id, char * buf, size_t capacity,
  size_t * out_len)
{
  return proton_signal_get_buffer(
    registry, signal_id, proton_Signal_string_value_tag, buf, capacity, out_len);
}

proton_status_e proton_signal_set_string(
  const proton_registry_t * registry, uint32_t signal_id, const char * str, size_t len)
{
  if (registry == NULL || str == NULL)
  {
    return PROTON_NULL_PTR_ERROR;
  }
  size_t idx = 0;
  signal_desc_t * desc = proton_registry_get_signal(registry, signal_id, &idx);
  if (desc == NULL)
  {
    return PROTON_ERROR;
  }
  if (desc->signal.which_signal != proton_Signal_string_value_tag)
  {
    return PROTON_ERROR;
  }
  if (registry->signal_max_capacity[idx] < len)
  {
    return PROTON_INSUFFICIENT_BUFFER_ERROR;
  }

  size_t capacity = strnlen(str, len);
  if (capacity == len)
  {
    return PROTON_ERROR;  // Input string is not null-terminated within the specified length
  }
  else
  {
    capacity += 1;  // Account for null terminator
  }

  memcpy(desc->signal.signal.string_value, str, capacity);
  desc->value_size = len;
  return PROTON_OK;
}

proton_status_e proton_signal_get_bytes(
  const proton_registry_t * registry, uint32_t signal_id, uint8_t * buf, size_t capacity,
  size_t * out_len)
{
  return proton_signal_get_buffer(
    registry, signal_id, proton_Signal_bytes_value_tag, buf, capacity, out_len);
}

proton_status_e proton_signal_set_bytes(
  const proton_registry_t * registry, uint32_t signal_id, const uint8_t * data, size_t len)
{
  if (registry == NULL || data == NULL)
  {
    return PROTON_NULL_PTR_ERROR;
  }
  size_t idx = 0;
  signal_desc_t * desc = proton_registry_get_signal(registry, signal_id, &idx);
  if (desc == NULL)
  {
    return PROTON_ERROR;
  }
  if (desc->signal.which_signal != proton_Signal_bytes_value_tag)
  {
    return PROTON_ERROR;
  }
  if (registry->signal_max_capacity[idx] < len)
  {
    return PROTON_INSUFFICIENT_BUFFER_ERROR;
  }
  memcpy(desc->signal.signal.bytes_value, data, len);
  desc->value_size = len;
  return PROTON_OK;
}
