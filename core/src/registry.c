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
#include "target_registry_sizes.h"

extern const bundle_desc_t g_bundle_table[PROTON_BUNDLE_REGISTRY_SIZE];
extern proton_Signal * g_bundle_signal_ptrs[PROTON_BUNDLE_REGISTRY_SIZE];
extern const id_to_index_t g_bundle_id_lut[PROTON_BUNDLE_REGISTRY_SIZE];
extern const id_to_index_t g_signal_id_lut[PROTON_SIGNAL_REGISTRY_SIZE];
extern const size_t g_signal_max_capacity[PROTON_SIGNAL_REGISTRY_SIZE];
extern signal_desc_t g_signal_registry[PROTON_SIGNAL_REGISTRY_SIZE];

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
      return -1;  // Invalid type
  }
}

const bundle_desc_t * proton_registry_get_bundle(uint32_t bundle_id, size_t * slot_idx)
{
  for (size_t i = 0; i < PROTON_ARRAY_SIZE(g_bundle_id_lut); i++)
  {
    if (g_bundle_id_lut[i].id == bundle_id)
    {
      size_t idx = g_bundle_id_lut[i].idx;
      if (slot_idx)
      {
        *slot_idx = idx;
      }
      return &g_bundle_table[idx];
    }
  }

  return NULL;
}

proton_Signal * proton_registry_get_bundle_signals(uint32_t bundle_id)
{
  for (size_t i = 0; i < PROTON_ARRAY_SIZE(g_bundle_id_lut); i++)
  {
    if (g_bundle_id_lut[i].id == bundle_id)
    {
      return g_bundle_signal_ptrs[g_bundle_id_lut[i].idx];
    }
  }

  return NULL;
}

signal_desc_t * proton_registry_get_signal(uint32_t signal_id, size_t * registry_idx)
{
  for (size_t i = 0; i < PROTON_ARRAY_SIZE(g_signal_id_lut); i++)
  {
    if (g_signal_id_lut[i].id == signal_id)
    {
      size_t idx = g_signal_id_lut[i].idx;
      if (registry_idx)
      {
        *registry_idx = idx;
      }
      return &g_signal_registry[idx];
    }
  }

  return NULL;
}

bool proton_signal_get_value(
  uint32_t signal_id, void * value, size_t * len, proton_signal_type_e * type)
{
  if (value == NULL || len == NULL || type == NULL)
  {
    return false;
  }

  for (size_t i = 0; i < PROTON_ARRAY_SIZE(g_signal_id_lut); i++)
  {
    if (g_signal_id_lut[i].id == signal_id)
    {
      *len = g_signal_registry[g_signal_id_lut[i].idx].value_size;
      proton_Signal * signal = (proton_Signal *)&g_signal_registry[g_signal_id_lut[i].idx].signal;
      *type = proton_get_type_from_tag(signal->which_signal);
      switch (*type)
      {
        case (PROTON_DOUBLE):
          *(double *)value = signal->signal.double_value;
          break;
        case (PROTON_FLOAT):
          *(float *)value = signal->signal.float_value;
          break;
        case (PROTON_INT32):
          *(int32_t *)value = signal->signal.int32_value;
          break;
        case (PROTON_INT64):
          *(int64_t *)value = signal->signal.int64_value;
          break;
        case (PROTON_UINT32):
          *(uint32_t *)value = signal->signal.uint32_value;
          break;
        case (PROTON_UINT64):
          *(uint64_t *)value = signal->signal.uint64_value;
          break;
        case (PROTON_BOOL):
          *(bool *)value = signal->signal.bool_value;
          break;
        case (PROTON_STRING):
          memcpy((char *)value, signal->signal.string_value, *len);
          break;
        case (PROTON_BYTES):
          memcpy((uint8_t *)value, signal->signal.bytes_value, *len);
          break;
        default:
          return false;
      }
      return true;
    }
  }

  return false;
}

bool proton_signal_set_value(uint32_t signal_id, const void * value, size_t len)
{
  if (value == NULL)
  {
    return false;
  }

  bool ret = false;

  for (size_t i = 0; i < PROTON_ARRAY_SIZE(g_signal_id_lut); i++)
  {
    if (g_signal_id_lut[i].id == signal_id)
    {
      signal_desc_t * signal_desc = &g_signal_registry[g_signal_id_lut[i].idx];
      proton_Signal * signal = (proton_Signal *)&signal_desc->signal;
      ret = true;
      switch (signal->which_signal)
      {
        case (proton_Signal_double_value_tag):
          signal->signal.double_value = *(double *)value;
          break;
        case (proton_Signal_float_value_tag):
          signal->signal.float_value = *(float *)value;
          break;
        case (proton_Signal_int32_value_tag):
          signal->signal.int32_value = *(int32_t *)value;
          break;
        case (proton_Signal_int64_value_tag):
          signal->signal.int64_value = *(int64_t *)value;
          break;
        case (proton_Signal_uint32_value_tag):
          signal->signal.uint32_value = *(uint32_t *)value;
          break;
        case (proton_Signal_uint64_value_tag):
          signal->signal.uint64_value = *(uint64_t *)value;
          break;
        case (proton_Signal_bool_value_tag):
          signal->signal.bool_value = *(bool *)value;
          break;
        case (proton_Signal_string_value_tag):
        {
          if (g_signal_max_capacity[g_signal_id_lut[i].idx] >= len)
          {
            memcpy(signal->signal.string_value, (char *)value, len);
            signal_desc->value_size = len;
          }
          else
          {
            ret = false;
          }
          break;
        }
        case (proton_Signal_bytes_value_tag):
        {
          if (g_signal_max_capacity[g_signal_id_lut[i].idx] >= len)
          {
            memcpy(signal->signal.bytes_value, (uint8_t *)value, len);
            signal_desc->value_size = len;
          }
          else
          {
            ret = false;
          }
          break;
        }

        default:
          ret = false;
      }
    }
  }

  return ret;
}
