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

#ifndef PROTON_LOG_H
#define PROTON_LOG_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "proton/common.h"
#include "proton/generated/log.pb.h"
#include "proton/proton_config.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /**
   * Log severity levels. Values match the proton.Log.Level protobuf enum.
   */
  typedef enum
  {
    PROTON_LOG_LEVEL_TRACE = 0,
    PROTON_LOG_LEVEL_DEBUG = 1,
    PROTON_LOG_LEVEL_INFO = 2,
    PROTON_LOG_LEVEL_WARN = 3,
    PROTON_LOG_LEVEL_ERROR = 4,
    PROTON_LOG_LEVEL_FATAL = 5,
  } proton_log_level_e;

  typedef bool (*proton_log_lock_fn)(void * arg);
  typedef bool (*proton_log_unlock_fn)(void * arg);
  typedef uint64_t (*proton_log_now_ms_fn)(void);

  /**
   * Callback invoked by the decode path when an inbound Proton{Log} message is decoded.
   * The `log` pointer is only valid for the duration of the call.
   */
  typedef void (*proton_log_receive_fn)(const proton_Log * log, void * arg);

  /**
   * User-provided logger configuration. All fields except `ring_buffer` are optional.
   * `lock`/`unlock` must be set as a pair or both NULL (single-context caller).
   */
  typedef struct proton_logger_config
  {
    uint8_t * ring_buffer;
    size_t ring_buffer_size;
    proton_log_level_e min_level;
    proton_log_now_ms_fn now_ms;
    proton_log_lock_fn lock;
    proton_log_unlock_fn unlock;
    void * lock_arg;
    proton_log_receive_fn on_receive;
    void * on_receive_arg;
  } proton_logger_config_t;

  /**
   * Logger context. Owned by the caller; typically statically allocated.
   * Fields are exposed for static allocation; do not read/write directly.
   */
  typedef struct proton_logger
  {
    proton_logger_config_t config;
    size_t head;
    size_t tail;
    size_t used_bytes;
    uint32_t sequence;
    uint32_t dropped_since_last;
  } proton_logger_t;

  /**
   * Pop-side view of a ring entry's fixed-size header (args copied separately).
   */
  typedef struct proton_log_entry_header
  {
    uint8_t level;
    uint64_t timestamp_ms;
    uint32_t sequence;
    uint32_t dropped_since_last;
    const void * fmt_ref;
  } proton_log_entry_header_t;

  /**
   * Initialize a logger from user-provided config. The config is copied by value.
   */
  proton_status_e proton_log_init(proton_logger_t * logger, const proton_logger_config_t * config);

  /**
   * Register `logger` as the process/image-wide default used by the logging macros
   * and the decode-side dispatch path. Pass NULL to clear.
   */
  void proton_log_set_default(proton_logger_t * logger);

  /**
   * Retrieve the current default logger, or NULL if none is set.
   */
  proton_logger_t * proton_log_default(void);

  proton_status_e proton_log_set_min_level(proton_logger_t * logger, proton_log_level_e level);

  /**
   * Push a pre-encoded tagged argument blob into the logger's ring buffer.
   * Intended to be called by the PROTON_LOG_* macros; direct use is allowed.
   */
  proton_status_e proton_log_push_raw(
    proton_logger_t * logger, uint8_t level, uint64_t timestamp_ms, const void * fmt_ref,
    const uint8_t * args_blob, size_t args_len);

  /**
   * Pop the oldest ring buffer entry. Header fields are copied into `header_out`;
   * up to `args_cap` bytes of the args blob are copied into `args_out`, and the
   * actual arg length is written to `*args_len` (may exceed `args_cap`).
   *
   * Returns PROTON_EMPTY if there is no entry, PROTON_INSUFFICIENT_BUFFER_ERROR
   * if `args_cap < *args_len` (entry not consumed), or PROTON_OK on success.
   */
  proton_status_e proton_log_pop_raw(
    proton_logger_t * logger, proton_log_entry_header_t * header_out, uint8_t * args_out,
    size_t args_cap, size_t * args_len);

  /**
   * Pop the oldest ring buffer entry and encode it as a Proton{Log} wire message
   * into `buffer`. Returns PROTON_EMPTY if there is no entry to drain.
   */
  proton_status_e proton_log_drain(
    proton_logger_t * logger, uint8_t * buffer, size_t buffer_len, size_t * out_len);

  /**
   * Invoke the logger's on_receive callback with the given decoded Log message.
   * No-op if the callback is not set.
   */
  proton_status_e proton_log_dispatch(proton_logger_t * logger, const proton_Log * log);

#ifdef __cplusplus
}
#endif

#endif  // PROTON_LOG_H
