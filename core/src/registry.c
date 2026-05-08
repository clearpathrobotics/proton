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

#define PROTON_ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

extern const bundle_desc_t g_bundle_table[];
extern const signal_id_to_index_t g_signal_id_lut[];
extern proton_Signal g_signal_registry[];

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

bool proton_signal_get_value(uint32_t signal_id, void * value)
{
  if (value == NULL)
  {
    return false;
  }

  for (size_t i = 0; i < PROTON_ARRAY_SIZE(g_signal_id_lut); i++)
  {
    if (g_signal_id_lut[i].s_id == signal_id)
    {
      value = (void *)&g_signal_registry[g_signal_id_lut[i].idx].signal;
      return true;
    }
  }

  return false;
}

bool proton_signal_set_value(uint32_t signal_id, const void * value)
{
  if (value == NULL)
  {
    return false;
  }

  for (size_t i = 0; i < PROTON_ARRAY_SIZE(g_signal_id_lut); i++)
  {
    if (g_signal_id_lut[i].s_id == signal_id)
    {
      proton_Signal * signal = &g_signal_registry[g_signal_id_lut[i].idx];
      switch (signal->which_signal)
      {
        case (proton_Signal_double_value_tag):
          signal->double_value = *(double *)value;
          break;
        case (proton_Signal_float_value_tag):
          signal->float_value = *(float *)value;
          break;
        case (proton_Signal_int32_value_tag):
          signal->int32_value = *(int32_t *)value;
          break;
        case (proton_Signal_int64_value_tag):
          signal->int64_value = *(int64_t *)value;
          break;
        case (proton_Signal_uint32_value_tag):
          signal->uint32_value = *(uint32_t *)value;
          break;
        case (proton_Signal_uint64_value_tag):
          signal->uint64_value = *(uint64_t *)value;
          break;
        case (proton_Signal_bool_value_tag):
          signal->bool_value = *(bool *)value;
          break;

        default:
          return false;
      }

      return true;
    }
  }

  return false;
}
