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

proton_status_e proton_lock_registry(const proton_registry_t * registry)
{
  proton_status_e lock_status = PROTON_OK;
  if (registry->mutex_handles.lock != NULL)
  {
    lock_status =
      registry->mutex_handles.lock(registry->mutex_handles.mutex, registry->mutex_handles.arg);
  }

  return lock_status;
}

proton_status_e proton_unlock_registry(const proton_registry_t * registry)
{
  proton_status_e unlock_status = PROTON_OK;
  if (registry->mutex_handles.unlock != NULL)
  {
    unlock_status =
      registry->mutex_handles.unlock(registry->mutex_handles.mutex, registry->mutex_handles.arg);
  }

  return unlock_status;
}

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

pb_size_t proton_get_tag_from_type(proton_signal_type_e type)
{
  switch (type)
  {
    case PROTON_DOUBLE:
      return proton_Signal_double_value_tag;
    case PROTON_FLOAT:
      return proton_Signal_float_value_tag;
    case PROTON_INT32:
      return proton_Signal_int32_value_tag;
    case PROTON_INT64:
      return proton_Signal_int64_value_tag;
    case PROTON_UINT32:
      return proton_Signal_uint32_value_tag;
    case PROTON_UINT64:
      return proton_Signal_uint64_value_tag;
    case PROTON_BOOL:
      return proton_Signal_bool_value_tag;
    case PROTON_STRING:
      return proton_Signal_string_value_tag;
    case PROTON_BYTES:
      return proton_Signal_bytes_value_tag;
    default:
      return 0;
  }
}

uint16_t get_signal_value_size(proton_signal_type_e type, uint32_t capacity)
{
  switch (type)
  {
    case PROTON_DOUBLE:
      return sizeof(double);
    case PROTON_FLOAT:
      return sizeof(float);
    case PROTON_INT32:
      return sizeof(int32_t);
    case PROTON_INT64:
      return sizeof(int64_t);
    case PROTON_UINT32:
      return sizeof(uint32_t);
    case PROTON_UINT64:
      return sizeof(uint64_t);
    case PROTON_BOOL:
      return sizeof(bool);
    case PROTON_STRING:
    case PROTON_BYTES:
      return capacity;
    default:
      return 0;
  }
}

proton_signal_type_e string_to_signal_type(const char * type_str)
{
  if (strncmp(type_str, "double", strlen("double")) == 0)
  {
    return PROTON_DOUBLE;
  }
  else if (strncmp(type_str, "float", strlen("float")) == 0)
  {
    return PROTON_FLOAT;
  }
  else if (strncmp(type_str, "int32", strlen("int32")) == 0)
  {
    return PROTON_INT32;
  }
  else if (strncmp(type_str, "int64", strlen("int64")) == 0)
  {
    return PROTON_INT64;
  }
  else if (strncmp(type_str, "uint32", strlen("uint32")) == 0)
  {
    return PROTON_UINT32;
  }
  else if (strncmp(type_str, "uint64", strlen("uint64")) == 0)
  {
    return PROTON_UINT64;
  }
  else if (strncmp(type_str, "bool", strlen("bool")) == 0)
  {
    return PROTON_BOOL;
  }
  else if (strncmp(type_str, "string", strlen("string")) == 0)
  {
    return PROTON_STRING;
  }
  else if (strncmp(type_str, "bytes", strlen("bytes")) == 0)
  {
    return PROTON_BYTES;
  }
  else
  {
    return PROTON_INVALID_TYPE;
  }
}

const bundle_desc_t * proton_registry_get_bundle(
  const proton_registry_t * registry, uint32_t bundle_id, size_t * slot_idx)
{
  if (registry != NULL)
  {
    for (size_t i = 0; i < registry->bundle_count; i++)
    {
      if (registry->bundle_table[i].bundle_id == bundle_id)
      {
        if (slot_idx)
        {
          *slot_idx = i;
        }
        return &registry->bundle_table[i];
      }
    }
  }

  return NULL;
}

proton_Signal * proton_registry_get_bundle_encode_decode_buffer(const proton_registry_t * registry)
{
  return registry->encode_decode_buffer;
}

proton_bundle_cb_t * proton_registry_get_bundle_callback(
  const proton_registry_t * registry, uint32_t bundle_id)
{
  for (size_t i = 0; i < registry->bundle_count; i++)
  {
    if (registry->bundle_table[i].bundle_id == bundle_id)
    {
      return &registry->bundle_table[i].callback;
    }
  }

  return NULL;
}

void proton_registry_set_bundle_callback(
  proton_registry_t * registry, uint32_t bundle_id, proton_bundle_cb_f bundle_cb, void * context)
{
  for (size_t i = 0; i < registry->bundle_count; i++)
  {
    if (registry->bundle_table[i].bundle_id == bundle_id)
    {
      registry->bundle_table[i].callback.cb = bundle_cb;
      registry->bundle_table[i].callback.arg = context;
    }
  }
}

void proton_registry_set_bundle_period(
  proton_registry_t * registry, uint32_t bundle_id, uint32_t period_ms)
{
  for (size_t i = 0; i < registry->bundle_count; i++)
  {
    if (registry->bundle_table[i].bundle_id == bundle_id)
    {
      registry->bundle_table[i].period_ms = period_ms;
      return;
    }
  }
}

signal_desc_t * proton_registry_get_signal(
  const proton_registry_t * registry, uint32_t signal_id, size_t * registry_idx)
{
  for (size_t i = 0; i < registry->signal_count; i++)
  {
    if (registry->signal_registry[i].id == signal_id)
    {
      if (registry_idx)
      {
        *registry_idx = i;
      }
      return &registry->signal_registry[i];
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

  size_t signal_size = desc->value_size;
  if (expected_tag == proton_Signal_string_value_tag)
  {
    signal_size = strnlen(desc->signal.signal.string_value, desc->capacity);
    if (signal_size < desc->capacity)
    {
      signal_size += 1;  // Account for null terminator if not present
    }
  }

  if (capacity < signal_size)
  {
    return PROTON_INSUFFICIENT_BUFFER_ERROR;
  }

  memcpy(buf, desc->signal.signal.string_value, signal_size);
  *out_len = signal_size;
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

  size_t string_len = strnlen(str, len);
  if (string_len == len)
  {
    return PROTON_ERROR;  // Input string is not null-terminated within the specified length
  }
  else
  {
    string_len += 1;  // Account for null terminator
  }

  if (len > desc->capacity || desc->capacity < string_len)
  {
    return PROTON_INSUFFICIENT_BUFFER_ERROR;
  }

  desc->value_size = string_len;

  memcpy(desc->signal.signal.string_value, str, string_len);
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
  if (registry == NULL || (data == NULL && len > 0))
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
  if (desc->capacity < len)
  {
    return PROTON_INSUFFICIENT_BUFFER_ERROR;
  }

  if (len == 0)
  {
    memset(desc->signal.signal.bytes_value, 0, desc->capacity);
  }
  else
  {
    memcpy(desc->signal.signal.bytes_value, data, len);
  }

  desc->value_size = len;
  return PROTON_OK;
}
