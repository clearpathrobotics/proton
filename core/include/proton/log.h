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
#include <string.h>

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

  /**
   * Bump the logger's dropped-since-last counter without pushing an entry.
   * Intended for macro use when arg encoding overflows the local buffer.
   */
  proton_status_e proton_log_note_drop(proton_logger_t * logger);

  /**
   * Format a deferred-log entry into `out` using `{}` placeholders in `fmt`.
   * `{{`/`}}` produce literal braces. Returns the number of bytes written
   * (excluding the terminating NUL). `out` is always NUL-terminated when
   * `out_cap > 0`.
   */
  size_t proton_log_format_entry(
    const char * fmt, const uint8_t * args, size_t args_len, char * out, size_t out_cap);

  // ---- Deferred-formatting arg encoding (public so macros can inline) --------

  typedef enum
  {
    PROTON_LOG_ARG_I32 = 0,
    PROTON_LOG_ARG_U32 = 1,
    PROTON_LOG_ARG_I64 = 2,
    PROTON_LOG_ARG_U64 = 3,
    PROTON_LOG_ARG_F64 = 4,
    PROTON_LOG_ARG_STR = 5,
    PROTON_LOG_ARG_PTR = 6,
    PROTON_LOG_ARG_CHR = 7,
  } proton_log_argtag_e;

  static inline size_t proton_log_put_(
    uint8_t * buf, size_t off, size_t cap, uint8_t tag, const void * src, size_t n)
  {
    if (off == SIZE_MAX || off + 1u + n > cap)
    {
      return SIZE_MAX;
    }
    buf[off] = tag;
    if (n > 0)
    {
      memcpy(buf + off + 1u, src, n);
    }
    return off + 1u + n;
  }

  static inline size_t proton_log_enc_i32_(uint8_t * b, size_t o, size_t c, int32_t v)
  {
    return proton_log_put_(b, o, c, (uint8_t)PROTON_LOG_ARG_I32, &v, sizeof(v));
  }
  static inline size_t proton_log_enc_u32_(uint8_t * b, size_t o, size_t c, uint32_t v)
  {
    return proton_log_put_(b, o, c, (uint8_t)PROTON_LOG_ARG_U32, &v, sizeof(v));
  }
  static inline size_t proton_log_enc_i64_(uint8_t * b, size_t o, size_t c, int64_t v)
  {
    return proton_log_put_(b, o, c, (uint8_t)PROTON_LOG_ARG_I64, &v, sizeof(v));
  }
  static inline size_t proton_log_enc_u64_(uint8_t * b, size_t o, size_t c, uint64_t v)
  {
    return proton_log_put_(b, o, c, (uint8_t)PROTON_LOG_ARG_U64, &v, sizeof(v));
  }
  static inline size_t proton_log_enc_f64_(uint8_t * b, size_t o, size_t c, double v)
  {
    return proton_log_put_(b, o, c, (uint8_t)PROTON_LOG_ARG_F64, &v, sizeof(v));
  }
  static inline size_t proton_log_enc_chr_(uint8_t * b, size_t o, size_t c, char v)
  {
    return proton_log_put_(b, o, c, (uint8_t)PROTON_LOG_ARG_CHR, &v, sizeof(v));
  }
  static inline size_t proton_log_enc_ptr_(uint8_t * b, size_t o, size_t c, const void * v)
  {
    uint64_t p = (uint64_t)(uintptr_t)v;
    return proton_log_put_(b, o, c, (uint8_t)PROTON_LOG_ARG_PTR, &p, sizeof(p));
  }
  static inline size_t proton_log_enc_str_(uint8_t * b, size_t o, size_t c, const char * s)
  {
    if (o == SIZE_MAX)
    {
      return SIZE_MAX;
    }
    size_t n = (s == NULL) ? 0u : strlen(s);
    if (n > (size_t)UINT16_MAX || o + 1u + 2u + n > c)
    {
      return SIZE_MAX;
    }
    b[o] = (uint8_t)PROTON_LOG_ARG_STR;
    uint16_t len16 = (uint16_t)n;
    memcpy(b + o + 1u, &len16, sizeof(len16));
    if (n > 0)
    {
      memcpy(b + o + 3u, s, n);
    }
    return o + 3u + n;
  }

/*
 * Type dispatch. In C we use _Generic; in C++ we use an overloaded function
 * template (C++ can't parse _Generic). Both paths funnel to the same static
 * inline encoders above.
 */
#ifdef __cplusplus
}  // extern "C"

static inline size_t proton_log_arg_(uint8_t * b, size_t o, size_t c, signed char v)
{
  return proton_log_enc_i32_(b, o, c, v);
}
static inline size_t proton_log_arg_(uint8_t * b, size_t o, size_t c, short v)
{
  return proton_log_enc_i32_(b, o, c, v);
}
static inline size_t proton_log_arg_(uint8_t * b, size_t o, size_t c, int v)
{
  return proton_log_enc_i32_(b, o, c, v);
}
static inline size_t proton_log_arg_(uint8_t * b, size_t o, size_t c, long v)
{
  return proton_log_enc_i64_(b, o, c, v);
}
static inline size_t proton_log_arg_(uint8_t * b, size_t o, size_t c, long long v)
{
  return proton_log_enc_i64_(b, o, c, v);
}
static inline size_t proton_log_arg_(uint8_t * b, size_t o, size_t c, unsigned char v)
{
  return proton_log_enc_u32_(b, o, c, v);
}
static inline size_t proton_log_arg_(uint8_t * b, size_t o, size_t c, unsigned short v)
{
  return proton_log_enc_u32_(b, o, c, v);
}
static inline size_t proton_log_arg_(uint8_t * b, size_t o, size_t c, unsigned int v)
{
  return proton_log_enc_u32_(b, o, c, v);
}
static inline size_t proton_log_arg_(uint8_t * b, size_t o, size_t c, unsigned long v)
{
  return proton_log_enc_u64_(b, o, c, v);
}
static inline size_t proton_log_arg_(uint8_t * b, size_t o, size_t c, unsigned long long v)
{
  return proton_log_enc_u64_(b, o, c, v);
}
static inline size_t proton_log_arg_(uint8_t * b, size_t o, size_t c, float v)
{
  return proton_log_enc_f64_(b, o, c, v);
}
static inline size_t proton_log_arg_(uint8_t * b, size_t o, size_t c, double v)
{
  return proton_log_enc_f64_(b, o, c, v);
}
static inline size_t proton_log_arg_(uint8_t * b, size_t o, size_t c, char v)
{
  return proton_log_enc_chr_(b, o, c, v);
}
static inline size_t proton_log_arg_(uint8_t * b, size_t o, size_t c, const char * v)
{
  return proton_log_enc_str_(b, o, c, v);
}
static inline size_t proton_log_arg_(uint8_t * b, size_t o, size_t c, char * v)
{
  return proton_log_enc_str_(b, o, c, v);
}
static inline size_t proton_log_arg_(uint8_t * b, size_t o, size_t c, const void * v)
{
  return proton_log_enc_ptr_(b, o, c, v);
}
static inline size_t proton_log_arg_(uint8_t * b, size_t o, size_t c, void * v)
{
  return proton_log_enc_ptr_(b, o, c, v);
}

extern "C"
{
#define PROTON_LOG_ENCODE_ARG(BUF, OFF, CAP, X) proton_log_arg_((BUF), (OFF), (CAP), (X))
#else
#define PROTON_LOG_ENCODE_ARG(BUF, OFF, CAP, X) \
  _Generic(                                     \
    (X),                                        \
    signed char: proton_log_enc_i32_,           \
    short: proton_log_enc_i32_,                 \
    int: proton_log_enc_i32_,                   \
    long: proton_log_enc_i64_,                  \
    long long: proton_log_enc_i64_,             \
    unsigned char: proton_log_enc_u32_,         \
    unsigned short: proton_log_enc_u32_,        \
    unsigned int: proton_log_enc_u32_,          \
    unsigned long: proton_log_enc_u64_,         \
    unsigned long long: proton_log_enc_u64_,    \
    float: proton_log_enc_f64_,                 \
    double: proton_log_enc_f64_,                \
    char: proton_log_enc_chr_,                  \
    char *: proton_log_enc_str_,                \
    const char *: proton_log_enc_str_,          \
    default: proton_log_enc_ptr_)((BUF), (OFF), (CAP), (X))
#endif

/* Variadic fan-out for up to 8 args, using ##__VA_ARGS__ so 0-args works. */
#define PROTON_LOG_NARG_(_0, _1, _2, _3, _4, _5, _6, _7, _8, N, ...) N
#define PROTON_LOG_NARG(...) PROTON_LOG_NARG_(_0, ##__VA_ARGS__, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define PROTON_LOG_CONCAT_(A, B) A##B
#define PROTON_LOG_CONCAT(A, B) PROTON_LOG_CONCAT_(A, B)

#define PROTON_LOG_FE_0(BUF, OFF, CAP)
#define PROTON_LOG_FE_1(BUF, OFF, CAP, X) (OFF) = PROTON_LOG_ENCODE_ARG((BUF), (OFF), (CAP), (X));
#define PROTON_LOG_FE_2(BUF, OFF, CAP, X, ...) \
  PROTON_LOG_FE_1(BUF, OFF, CAP, X)            \
  PROTON_LOG_FE_1(BUF, OFF, CAP, __VA_ARGS__)
#define PROTON_LOG_FE_3(BUF, OFF, CAP, X, ...) \
  PROTON_LOG_FE_1(BUF, OFF, CAP, X)            \
  PROTON_LOG_FE_2(BUF, OFF, CAP, __VA_ARGS__)
#define PROTON_LOG_FE_4(BUF, OFF, CAP, X, ...) \
  PROTON_LOG_FE_1(BUF, OFF, CAP, X)            \
  PROTON_LOG_FE_3(BUF, OFF, CAP, __VA_ARGS__)
#define PROTON_LOG_FE_5(BUF, OFF, CAP, X, ...) \
  PROTON_LOG_FE_1(BUF, OFF, CAP, X)            \
  PROTON_LOG_FE_4(BUF, OFF, CAP, __VA_ARGS__)
#define PROTON_LOG_FE_6(BUF, OFF, CAP, X, ...) \
  PROTON_LOG_FE_1(BUF, OFF, CAP, X)            \
  PROTON_LOG_FE_5(BUF, OFF, CAP, __VA_ARGS__)
#define PROTON_LOG_FE_7(BUF, OFF, CAP, X, ...) \
  PROTON_LOG_FE_1(BUF, OFF, CAP, X)            \
  PROTON_LOG_FE_6(BUF, OFF, CAP, __VA_ARGS__)
#define PROTON_LOG_FE_8(BUF, OFF, CAP, X, ...) \
  PROTON_LOG_FE_1(BUF, OFF, CAP, X)            \
  PROTON_LOG_FE_7(BUF, OFF, CAP, __VA_ARGS__)

#define PROTON_LOG_ENCODE_ARGS(BUF, OFF, CAP, ...) \
  PROTON_LOG_CONCAT(PROTON_LOG_FE_, PROTON_LOG_NARG(__VA_ARGS__))(BUF, OFF, CAP, ##__VA_ARGS__)

/*
 * Compile-time floor. Any call site whose LEVEL_ < PROTON_LOG_MIN_LEVEL is
 * elided by the compiler (the `if (0)` branch is optimized out).
 */
#ifndef PROTON_LOG_MIN_LEVEL
#define PROTON_LOG_MIN_LEVEL PROTON_LOG_LEVEL_TRACE
#endif

/*
 * Push a log entry to `LOGGER_`. `FMT_` must be a string literal (enforced by
 * the "" FMT_ concatenation). Args are captured by _Generic and stored raw in
 * the ring; formatting happens at drain time.
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
        uint8_t proton_log_argbuf_[PROTON_LOG_MAX_ARGS_SIZE];                                     \
        size_t proton_log_off_ = 0u;                                                              \
        PROTON_LOG_ENCODE_ARGS(                                                                   \
          proton_log_argbuf_, proton_log_off_, sizeof(proton_log_argbuf_), ##__VA_ARGS__)         \
        if (proton_log_off_ == SIZE_MAX)                                                          \
        {                                                                                         \
          (void)proton_log_note_drop(proton_log_logger_);                                         \
        }                                                                                         \
        else                                                                                      \
        {                                                                                         \
          uint64_t proton_log_ts_ = (proton_log_logger_->config.now_ms != NULL)                   \
                                      ? proton_log_logger_->config.now_ms()                       \
                                      : 0u;                                                       \
          (void)proton_log_push_raw(                                                              \
            proton_log_logger_, (uint8_t)(LEVEL_), proton_log_ts_, ("" FMT_), proton_log_argbuf_, \
            proton_log_off_);                                                                     \
        }                                                                                         \
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
