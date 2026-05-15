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

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "proton_core/bundle.pb.h"
#include "proton_core/proton.pb.h"
#include "proton_core/signal.pb.h"

#include "proton/common.h"

#ifdef __cplusplus
extern "C"
{
#endif

  typedef enum
  {
    PROTON_INVALID_TYPE = 0,
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

  typedef struct proton_buffer
  {
    uint8_t * data;
    size_t len;
  } proton_buffer_t;

  /**
   * Generic ID to list mapping struct
   */
  typedef struct proton_id_list
  {
    const uint32_t * ids;
    size_t count;
  } proton_id_list_t;

  /**
   * Descriptor for a signal in the registry
   * Contains information for encoding/decoding the signal, as well as the signal's current value
   */
  typedef struct signal_desc
  {
    uint32_t id;
    proton_signal_type_e type;
    // For strings and bytes, capacity of the signal. For other types, this is the size of the internal type.
    size_t value_size;
    proton_Signal signal;
  } signal_desc_t;

  /**
   * Descriptor for a bundle, containing the ID, which nodes produce/consume it, and the signals within
   */
  typedef struct bundle_desc
  {
    uint32_t bundle_id;
    proton_id_list_t producer_ids;
    proton_id_list_t consumer_ids;
    proton_id_list_t signal_ids;
  } bundle_desc_t;

  /**
   * Mapping from ID to index in the signal or bundle registries.
   */
  typedef struct signal_id_to_index
  {
    uint32_t id;
    uint32_t idx;
  } id_to_index_t;

  /**
   * proton_registry_t is a memory representation of the entire set of bundles and signals for a node.
   * It is used as the source of truth for encoding and decoding bundles, and can be queried for signal values.
   * The registry is provided by a user pre-populated with bundle and signal mapping
   * There is a script to provide this information at build time (generator.py) but is not required or part of the
   * proton_core library. The generator script is intended for usage within embedded systems where memory allocation is
   * limited or disallowed, providing a static memory block for the registry.
   */
  typedef struct proton_registry
  {
    // Bundle metadata and state
    // bundle_table is the table of all bundle descriptors
    // Since the bundle IDs are not contiguous, the bundle_id_lut is used to find the index of a bundle descriptor
    const bundle_desc_t * bundle_table;
    const id_to_index_t * bundle_id_lut;
    size_t bundle_count;

    // Buffers for encoding/decoding signals per bundle.
    // This is NOT the actual value of the signal, but a temporary buffer space.
    proton_Signal ** bundle_signal_ptrs;

    // Signal metadata and state
    // signal_registry is the table of all signal descriptors,
    // It also contains the current value of each signal. It is written to after a bundle is successfully decoded
    signal_desc_t * signal_registry;
    // Similar to how bundle ID's are not contiguous, signal ID's are looked up in a similar manner
    const id_to_index_t * signal_id_lut;
    // For signals that are bytes/string types, the max capacity of the signal is specified here
    const size_t * signal_max_capacity;
    // Space for string/bytes signals
    uint8_t ** signal_decode_buffers;
    size_t signal_count;
  } proton_registry_t;

  /**
   * Get the bundle from a registry by ID
   * slot_idx is optional output parameter for the index of the bundle in the registry
   * @return pointer to the bundle descriptor, or NULL if not found
   */
  const bundle_desc_t * proton_registry_get_bundle(
    const proton_registry_t * registry, uint32_t bundle_id, size_t * slot_idx);

  /**
   * Get the buffer for encoding/decoding signals from a bundle, by bundle ID
   * @return pointer to the buffer, or NULL if not found
   */
  proton_Signal * proton_registry_get_bundle_encode_decode_buffer(
    const proton_registry_t * registry, uint32_t bundle_id);

  /**
   * Get the signal from a registry by ID
   * registry_idx is optional output parameter for the index of the signal in the registry
   * @return pointer to the signal descriptor, or NULL if not found
   */
  signal_desc_t * proton_registry_get_signal(
    const proton_registry_t * registry, uint32_t signal_id, size_t * registry_idx);

  /**
   * Get the signal type from a protobuf tag
   * @return the signal type
   */
  proton_signal_type_e proton_get_type_from_tag(pb_size_t tag);

  /*
   * Typed signal accessors.
   *
   * Each accessor returns:
   *   PROTON_OK                         on success
   *   PROTON_NULL_PTR_ERROR             if a required pointer is NULL
   *   PROTON_ERROR                      if signal_id is not found, or the signal's
   *                                     stored type does not match the accessor type
   *   PROTON_INSUFFICIENT_BUFFER_ERROR  (string/bytes only) when the destination
   *                                     buffer is smaller than the stored value, or
   *                                     when set() is called with len exceeding the
   *                                     registered capacity for the signal
   *
   * For string/bytes getters, *out_len receives the number of bytes copied.
   */

  proton_status_e proton_signal_get_double(
    const proton_registry_t * registry, uint32_t signal_id, double * value);
  proton_status_e proton_signal_set_double(
    const proton_registry_t * registry, uint32_t signal_id, double value);

  proton_status_e proton_signal_get_float(
    const proton_registry_t * registry, uint32_t signal_id, float * value);
  proton_status_e proton_signal_set_float(
    const proton_registry_t * registry, uint32_t signal_id, float value);

  proton_status_e proton_signal_get_int32(
    const proton_registry_t * registry, uint32_t signal_id, int32_t * value);
  proton_status_e proton_signal_set_int32(
    const proton_registry_t * registry, uint32_t signal_id, int32_t value);

  proton_status_e proton_signal_get_int64(
    const proton_registry_t * registry, uint32_t signal_id, int64_t * value);
  proton_status_e proton_signal_set_int64(
    const proton_registry_t * registry, uint32_t signal_id, int64_t value);

  proton_status_e proton_signal_get_uint32(
    const proton_registry_t * registry, uint32_t signal_id, uint32_t * value);
  proton_status_e proton_signal_set_uint32(
    const proton_registry_t * registry, uint32_t signal_id, uint32_t value);

  proton_status_e proton_signal_get_uint64(
    const proton_registry_t * registry, uint32_t signal_id, uint64_t * value);
  proton_status_e proton_signal_set_uint64(
    const proton_registry_t * registry, uint32_t signal_id, uint64_t value);

  proton_status_e proton_signal_get_bool(
    const proton_registry_t * registry, uint32_t signal_id, bool * value);
  proton_status_e proton_signal_set_bool(
    const proton_registry_t * registry, uint32_t signal_id, bool value);

  proton_status_e proton_signal_get_string(
    const proton_registry_t * registry, uint32_t signal_id, char * buf, size_t capacity,
    size_t * out_len);
  proton_status_e proton_signal_set_string(
    const proton_registry_t * registry, uint32_t signal_id, const char * str, size_t len);

  proton_status_e proton_signal_get_bytes(
    const proton_registry_t * registry, uint32_t signal_id, uint8_t * buf, size_t capacity,
    size_t * out_len);
  proton_status_e proton_signal_set_bytes(
    const proton_registry_t * registry, uint32_t signal_id, const uint8_t * data, size_t len);

#ifdef __cplusplus
}
#endif

#endif  // PROTON_REGISTRY_H
