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
extern const id_to_index_t g_signal_id_lut[PROTON_SIGNAL_REGISTRY_SIZE];
extern const size_t g_signal_max_capacity[PROTON_SIGNAL_REGISTRY_SIZE];
extern signal_desc_t g_signal_registry[PROTON_SIGNAL_REGISTRY_SIZE];

static proton_signal_type_e proton_get_type_from_tag(pb_size_t tag)
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

const bundle_desc_t * proton_registry_get_bundle(uint32_t bundle_id)
{
  for (size_t i = 0; i < PROTON_ARRAY_SIZE(g_bundle_table); i++)
  {
    if (g_bundle_table[i].bundle_id == bundle_id)
    {
      return &g_bundle_table[i];
    }
  }

  return NULL;
}

bool proton_registry_get_signal(uint32_t signal_id, signal_desc_t * desc)
{
  if (desc == NULL)
  {
    return false;
  }

  for (size_t i = 0; i < PROTON_ARRAY_SIZE(g_signal_id_lut); i++)
  {
    if (g_signal_id_lut[i].id == signal_id)
    {
      desc = &g_signal_registry[g_signal_id_lut[i].idx];
      return true;
    }
  }

  return false;
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
      value = (void *)&g_signal_registry[g_signal_id_lut[i].idx].signal.signal;
      *len = g_signal_registry[g_signal_id_lut[i].idx].value_size;
      *type =
        proton_get_type_from_tag(g_signal_registry[g_signal_id_lut[i].idx].signal.which_signal);
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

  for (size_t i = 0; i < PROTON_ARRAY_SIZE(g_signal_id_lut); i++)
  {
    if (g_signal_id_lut[i].id == signal_id)
    {
      signal_desc_t * signal_desc = &g_signal_registry[g_signal_id_lut[i].idx];
      proton_Signal * signal = (proton_Signal *)&signal_desc->signal;
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
          break;
        }
        case (proton_Signal_bytes_value_tag):
        {
          if (g_signal_max_capacity[g_signal_id_lut[i].idx] >= len)
          {
            memcpy(signal->signal.bytes_value, (uint8_t *)value, len);
            signal_desc->value_size = len;
          }
          break;
        }

        default:
          return false;
      }

      return true;
    }
  }

  return false;
}
