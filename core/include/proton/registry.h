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

#ifndef PROTON_REGISTRY_H
#define PROTON_REGISTRY_H

#include <stddef.h>
#include <stdint.h>

#include "proton_core/signal.pb.h"

#ifdef __cplusplus
extern "C"
{
#endif

  typedef enum
  {
    PROTON_DOUBLE = proton_Signal_double_value_tag,
    PROTON_FLOAT = proton_Signal_float_value_tag,
    PROTON_INT32 = proton_Signal_int32_value_tag,
    PROTON_INT64 = proton_Signal_int64_value_tag,
    PROTON_UINT32 = proton_Signal_uint32_value_tag,
    PROTON_UINT64 = proton_Signal_uint64_value_tag,
    PROTON_BOOL = proton_Signal_bool_value_tag,
    PROTON_STRING = proton_Signal_string_value_tag,
    PROTON_BYTES = proton_Signal_bytes_value_tag,
  } proton_signal_type_e;

  typedef struct proton_id_list
  {
    const uint32_t * ids;
    size_t count;
  } proton_id_list_t;

  typedef struct signal_entry
  {
    uint32_t id;
    proton_Signal signal;
  } signal_entry_t;

  typedef struct signal_desc
  {
    uint32_t id;
    proton_signal_type_e type;
    size_t capacity;  // For strings and bytes, capacity of the signal. For other types, this is 0.
  } signal_desc_t;

  typedef struct bundle_desc
  {
    uint32_t bundle_id;
    proton_id_list_t producer_ids;
    proton_id_list_t consumer_ids;
    proton_id_list_t signal_ids;
  } bundle_desc_t;

  typedef struct signal_id_to_index
  {
    uint32_t s_id;
    uint32_t idx;
  } signal_id_to_index_t;

  const bundle_desc_t * proton_registry_get_bundle(uint32_t bundle_id);

  bool proton_signal_set_value(uint32_t signal_id, const void * value);
  bool proton_signal_get_value(uint32_t signal_id, void * value);

#ifdef __cplusplus
}
#endif

#endif  // PROTON_REGISTRY_H
