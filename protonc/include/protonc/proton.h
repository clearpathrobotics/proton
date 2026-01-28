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

#ifndef INC_PROTONC_PROTON_H_
#define INC_PROTONC_PROTON_H_

#include "proton/common.h"
#include "protonc/bundle.pb.h"
#include "pb.h"
#include "pb_decode.h"
#include "pb_encode.h"

typedef uint64_t proton_producer_t;
typedef uint64_t proton_consumer_t;

typedef struct {
  size_t length; // Length of list
  size_t capacity; // Capacity of data buffer (strings and bytes only)
  size_t size; // Current size of list (for decoding only)
  void *data; // Pointer to start of data buffer
} proton_list_t;

typedef struct {
  proton_Signal signal;
  proton_list_t value;
} proton_signal_handle_t;

typedef struct {
  proton_Bundle bundle;
  proton_list_t signals;
  proton_producer_t producers;
  proton_consumer_t consumers;
} proton_bundle_handle_t;

typedef struct {
  uint8_t * data;
  size_t len;
} proton_buffer_t;

typedef struct proton_bundle_heartbeat {
  uint32_t heartbeat;
} proton_bundle_heartbeat_t;

#define PROTON_LIST_ARG_DEFAULT_VALUE {NULL, 0, 0, 0}
#define PROTON_BUFFER_DEFAULT_VALUE {NULL, 0}
#define PROTON_HEARTBEAT_BUNDLE_DEFAULT_VALUE {0}

proton_status_e proton_init_signal(proton_signal_handle_t * handle,
                                   pb_size_t which_signal,
                                   void * data,
                                   size_t length,
                                   size_t capacity);

proton_status_e proton_init_bundle(proton_bundle_handle_t *handle,
                                  uint32_t id,
                                  proton_signal_handle_t *signal_handles,
                                  uint32_t signal_count,
                                  proton_producer_t producers,
                                  proton_consumer_t consumers);

proton_status_e proton_encode(proton_bundle_handle_t * handle, proton_buffer_t buffer, size_t *bytes_encoded);
proton_status_e proton_decode(proton_bundle_handle_t * handle, proton_buffer_t buffer, size_t length);
proton_status_e proton_decode_id(proton_buffer_t buffer, uint32_t *id);

void proton_print_bundle(proton_Bundle bundle);
void proton_print_signal(proton_Signal signal);

#ifdef PROTON_DEBUG
int proton_print(const char * format, ...);
#define PROTON_PRINT(...) proton_print(__VA_ARGS__);
#else
#define PROTON_PRINT(...)
#endif


#endif // INC_PROTONC_PROTON_H_
