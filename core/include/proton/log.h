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
#include <stdio.h>

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
   * User-provided logger configuration. `entries` and `capacity` are required.
   * `lock`/`unlock` must be set as a pair or both NULL (single-context caller).
   */
  typedef struct proton_logger_config
  {
    proton_Log * entries;
    size_t capacity;
    proton_log_level_e min_level;
    proton_log_now_ms_fn now_ms;
    proton_log_lock_fn lock;
    proton_log_unlock_fn unlock;
    void * lock_arg;
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
    size_t count;
    uint32_t sequence;
  } proton_logger_t;

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
   * Push an already-formatted log message into the logger's ring buffer.
   * `text` need not be NUL-terminated; up to `sizeof(proton_Log.text) - 1` bytes
   * are copied, and the stored entry is always NUL-terminated.
   * Intended to be called by the PROTON_LOG_* macros; direct use is allowed.
   */
  proton_status_e proton_log_push(
    proton_logger_t * logger, uint8_t level, uint64_t timestamp_ms, const char * text,
    size_t text_len);

  /**
   * Pop the oldest ring buffer entry into `out`. Returns PROTON_EMPTY when
   * the ring is empty.
   */
  proton_status_e proton_log_pop(proton_logger_t * logger, proton_Log * out);

  /**
   * Pop the oldest ring buffer entry and encode it as a Proton{Log} wire message
   * into `buffer`. Returns PROTON_EMPTY if there is no entry to drain.
   */
  proton_status_e proton_log_encode_next(
    proton_logger_t * logger, uint8_t * buffer, size_t buffer_len, size_t * out_len);

/*
 * Compile-time floor. Any call site whose LEVEL_ < PROTON_LOG_MIN_LEVEL is
 * elided by the compiler (the `if (0)` branch is optimized out).
 */
#ifndef PROTON_LOG_MIN_LEVEL
#define PROTON_LOG_MIN_LEVEL PROTON_LOG_LEVEL_TRACE
#endif

/*
 * Push a log entry to `LOGGER_`. `FMT_` must be a string literal (enforced by
 * the ("" FMT_) concatenation). snprintf runs at the call site; over-long
 * messages are silently truncated to PROTON_LOG_MAX_MESSAGE_SIZE - 1 bytes.
 */
#define PROTON_LOG(LOGGER_, LEVEL_, FMT_, ...)                                                    \
  do                                                                                              \
  {                                                                                               \
    if ((int)(LEVEL_) >= (int)(PROTON_LOG_MIN_LEVEL))                                             \
    {                                                                                             \
      proton_logger_t * proton_log_logger_ = (LOGGER_);                                           \
      if (                                                                                        \
        proton_log_logger_ != NULL && (int)(LEVEL_) >= (int)proton_log_logger_->config.min_level) \
      {                                                                                           \
        char proton_log_text_[PROTON_LOG_MAX_MESSAGE_SIZE];                                       \
        int proton_log_n_ =                                                                       \
          snprintf(proton_log_text_, sizeof(proton_log_text_), ("" FMT_), ##__VA_ARGS__);         \
        if (proton_log_n_ < 0)                                                                    \
        {                                                                                         \
          proton_log_n_ = 0;                                                                      \
        }                                                                                         \
        else if ((size_t)proton_log_n_ >= sizeof(proton_log_text_))                               \
        {                                                                                         \
          proton_log_n_ = (int)(sizeof(proton_log_text_) - 1u);                                   \
        }                                                                                         \
        uint64_t proton_log_ts_ =                                                                 \
          (proton_log_logger_->config.now_ms != NULL) ? proton_log_logger_->config.now_ms() : 0u; \
        (void)proton_log_push(                                                                    \
          proton_log_logger_, (uint8_t)(LEVEL_), proton_log_ts_, proton_log_text_,                \
          (size_t)proton_log_n_);                                                                 \
      }                                                                                           \
    }                                                                                             \
  } while (0)

#define PROTON_LOG_D(LEVEL_, FMT_, ...) \
  PROTON_LOG(proton_log_default(), LEVEL_, FMT_, ##__VA_ARGS__)

#define PROTON_LOG_TRACE(FMT_, ...) PROTON_LOG_D(PROTON_LOG_LEVEL_TRACE, FMT_, ##__VA_ARGS__)
#define PROTON_LOG_DEBUG(FMT_, ...) PROTON_LOG_D(PROTON_LOG_LEVEL_DEBUG, FMT_, ##__VA_ARGS__)
#define PROTON_LOG_INFO(FMT_, ...) PROTON_LOG_D(PROTON_LOG_LEVEL_INFO, FMT_, ##__VA_ARGS__)
#define PROTON_LOG_WARN(FMT_, ...) PROTON_LOG_D(PROTON_LOG_LEVEL_WARN, FMT_, ##__VA_ARGS__)
#define PROTON_LOG_ERROR(FMT_, ...) PROTON_LOG_D(PROTON_LOG_LEVEL_ERROR, FMT_, ##__VA_ARGS__)
#define PROTON_LOG_FATAL(FMT_, ...) PROTON_LOG_D(PROTON_LOG_LEVEL_FATAL, FMT_, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif  // PROTON_LOG_H
