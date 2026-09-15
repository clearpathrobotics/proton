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

#include "proton/log.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "pb_encode.h"
#include "proton/generated/proton.pb.h"

// Ring entry header layout (host-local; use memcpy, alignment-agnostic).
//  0..1:  u16 total_len   (0 = wrap sentinel; else RING_HEADER_SIZE + args_len)
//  2:     u8  level
//  3:     u8  reserved
//  4..7:  u32 sequence
//  8..11: u32 dropped_since_last
//  12..19:u64 timestamp_ms
//  20..27:u64 fmt_ref
//  28+:   args blob
#define RING_HEADER_SIZE 28u
#define RING_OFF_TOTAL_LEN 0u
#define RING_OFF_LEVEL 2u
#define RING_OFF_SEQUENCE 4u
#define RING_OFF_DROPPED 8u
#define RING_OFF_TIMESTAMP 12u
#define RING_OFF_FMT_REF 20u
#define RING_OFF_ARGS 28u

static proton_logger_t * s_default_logger = NULL;

#define RING_DEFINE_RW(SUFFIX, CTYPE)                                          \
  static void ring_read_##SUFFIX(const uint8_t * buf, size_t off, CTYPE * out) \
  {                                                                            \
    memcpy(out, buf + off, sizeof(*out));                                      \
  }                                                                            \
  static void ring_write_##SUFFIX(uint8_t * buf, size_t off, CTYPE v)          \
  {                                                                            \
    memcpy(buf + off, &v, sizeof(v));                                          \
  }

RING_DEFINE_RW(u16, uint16_t)
RING_DEFINE_RW(u32, uint32_t)
RING_DEFINE_RW(u64, uint64_t)

#undef RING_DEFINE_RW

static proton_status_e lock_logger(const proton_logger_config_t * cfg)
{
  if (cfg->lock == NULL)
  {
    return PROTON_OK;
  }
  return cfg->lock(cfg->lock_arg) ? PROTON_OK : PROTON_MUTEX_ERROR;
}

static proton_status_e unlock_logger(const proton_logger_config_t * cfg)
{
  if (cfg->unlock == NULL)
  {
    return PROTON_OK;
  }
  return cfg->unlock(cfg->lock_arg) ? PROTON_OK : PROTON_MUTEX_ERROR;
}

proton_status_e proton_log_init(proton_logger_t * logger, const proton_logger_config_t * config)
{
  if (logger == NULL || config == NULL)
  {
    return PROTON_NULL_PTR_ERROR;
  }
  if (config->ring_buffer == NULL || config->ring_buffer_size < RING_HEADER_SIZE + 1u)
  {
    return PROTON_INSUFFICIENT_BUFFER_ERROR;
  }
  if ((config->lock == NULL) != (config->unlock == NULL))
  {
    return PROTON_ERROR;
  }

  memset(logger, 0, sizeof(*logger));
  logger->config = *config;
  return PROTON_OK;
}

void proton_log_set_default(proton_logger_t * logger) { s_default_logger = logger; }

proton_logger_t * proton_log_default(void) { return s_default_logger; }

proton_status_e proton_log_set_min_level(proton_logger_t * logger, proton_log_level_e level)
{
  if (logger == NULL)
  {
    return PROTON_NULL_PTR_ERROR;
  }
  logger->config.min_level = level;
  return PROTON_OK;
}

proton_status_e proton_log_push_raw(
  proton_logger_t * logger, uint8_t level, uint64_t timestamp_ms, const void * fmt_ref,
  const uint8_t * args_blob, size_t args_len)
{
  if (logger == NULL || (args_len > 0 && args_blob == NULL))
  {
    return PROTON_NULL_PTR_ERROR;
  }
  if (args_len > PROTON_LOG_MAX_ARGS_SIZE || args_len > (size_t)UINT16_MAX - RING_HEADER_SIZE)
  {
    return PROTON_INSUFFICIENT_BUFFER_ERROR;
  }

  const size_t ring_size = logger->config.ring_buffer_size;
  const size_t needed = RING_HEADER_SIZE + args_len;
  if (needed > ring_size)
  {
    logger->dropped_since_last++;
    return PROTON_INSUFFICIENT_BUFFER_ERROR;
  }

  proton_status_e s = lock_logger(&logger->config);
  if (s != PROTON_OK)
  {
    return s;
  }

  uint8_t * buf = logger->config.ring_buffer;
  size_t head = logger->head;

  if (head + needed > ring_size)
  {
    const size_t wrap_cost = ring_size - head;
    if (logger->used_bytes + wrap_cost + needed > ring_size)
    {
      logger->dropped_since_last++;
      (void)unlock_logger(&logger->config);
      return PROTON_INSUFFICIENT_BUFFER_ERROR;
    }
    if (head + sizeof(uint16_t) <= ring_size)
    {
      ring_write_u16(buf, head + RING_OFF_TOTAL_LEN, 0);
    }
    logger->used_bytes += wrap_cost;
    head = 0;
  }
  else if (logger->used_bytes + needed > ring_size)
  {
    logger->dropped_since_last++;
    (void)unlock_logger(&logger->config);
    return PROTON_INSUFFICIENT_BUFFER_ERROR;
  }

  const uint32_t seq = logger->sequence++;
  const uint32_t dropped = logger->dropped_since_last;
  logger->dropped_since_last = 0;

  ring_write_u16(buf, head + RING_OFF_TOTAL_LEN, (uint16_t)needed);
  buf[head + RING_OFF_LEVEL] = level;
  buf[head + RING_OFF_LEVEL + 1u] = 0;
  ring_write_u32(buf, head + RING_OFF_SEQUENCE, seq);
  ring_write_u32(buf, head + RING_OFF_DROPPED, dropped);
  ring_write_u64(buf, head + RING_OFF_TIMESTAMP, timestamp_ms);
  ring_write_u64(buf, head + RING_OFF_FMT_REF, (uint64_t)(uintptr_t)fmt_ref);
  if (args_len > 0)
  {
    memcpy(buf + head + RING_OFF_ARGS, args_blob, args_len);
  }

  head += needed;
  if (head == ring_size)
  {
    head = 0;
  }
  logger->head = head;
  logger->used_bytes += needed;

  return unlock_logger(&logger->config);
}

// Advances tail past any wrap markers (explicit u16=0 or implicit "no room for length at tail").
// Returns true if a real entry is now readable at logger->tail; false if the ring is empty.
static bool skip_wrap_sentinels_locked(proton_logger_t * logger)
{
  const size_t ring_size = logger->config.ring_buffer_size;
  const uint8_t * buf = logger->config.ring_buffer;

  while (logger->used_bytes > 0)
  {
    if (logger->tail + sizeof(uint16_t) > ring_size)
    {
      const size_t wrap_cost = ring_size - logger->tail;
      logger->used_bytes -= wrap_cost;
      logger->tail = 0;
      continue;
    }
    uint16_t total_len;
    ring_read_u16(buf, logger->tail + RING_OFF_TOTAL_LEN, &total_len);
    if (total_len == 0)
    {
      const size_t wrap_cost = ring_size - logger->tail;
      logger->used_bytes -= wrap_cost;
      logger->tail = 0;
      continue;
    }
    return true;
  }
  return false;
}

proton_status_e proton_log_pop_raw(
  proton_logger_t * logger, proton_log_entry_header_t * header_out, uint8_t * args_out,
  size_t args_cap, size_t * args_len)
{
  if (logger == NULL || header_out == NULL || args_len == NULL)
  {
    return PROTON_NULL_PTR_ERROR;
  }

  proton_status_e s = lock_logger(&logger->config);
  if (s != PROTON_OK)
  {
    return s;
  }

  if (!skip_wrap_sentinels_locked(logger))
  {
    (void)unlock_logger(&logger->config);
    return PROTON_EMPTY;
  }

  const size_t ring_size = logger->config.ring_buffer_size;
  const uint8_t * buf = logger->config.ring_buffer;
  const size_t tail = logger->tail;

  uint16_t total_len;
  ring_read_u16(buf, tail + RING_OFF_TOTAL_LEN, &total_len);
  if (total_len < RING_HEADER_SIZE || tail + total_len > ring_size)
  {
    logger->tail = logger->head;
    logger->used_bytes = 0;
    (void)unlock_logger(&logger->config);
    return PROTON_ERROR;
  }

  const size_t entry_args_len = (size_t)total_len - RING_HEADER_SIZE;
  *args_len = entry_args_len;
  if (entry_args_len > args_cap)
  {
    (void)unlock_logger(&logger->config);
    return PROTON_INSUFFICIENT_BUFFER_ERROR;
  }

  header_out->level = buf[tail + RING_OFF_LEVEL];
  ring_read_u32(buf, tail + RING_OFF_SEQUENCE, &header_out->sequence);
  ring_read_u32(buf, tail + RING_OFF_DROPPED, &header_out->dropped_since_last);
  ring_read_u64(buf, tail + RING_OFF_TIMESTAMP, &header_out->timestamp_ms);
  uint64_t fmt_ref_raw;
  ring_read_u64(buf, tail + RING_OFF_FMT_REF, &fmt_ref_raw);
  header_out->fmt_ref = (const void *)(uintptr_t)fmt_ref_raw;

  if (entry_args_len > 0 && args_out != NULL)
  {
    memcpy(args_out, buf + tail + RING_OFF_ARGS, entry_args_len);
  }

  size_t new_tail = tail + total_len;
  if (new_tail == ring_size)
  {
    new_tail = 0;
  }
  logger->tail = new_tail;
  logger->used_bytes -= total_len;

  return unlock_logger(&logger->config);
}

proton_status_e proton_log_drain(
  proton_logger_t * logger, uint8_t * buffer, size_t buffer_len, size_t * out_len)
{
  if (logger == NULL || buffer == NULL || out_len == NULL)
  {
    return PROTON_NULL_PTR_ERROR;
  }

  proton_log_entry_header_t hdr;
  uint8_t args[PROTON_LOG_MAX_ARGS_SIZE];
  size_t args_len = 0;
  proton_status_e s = proton_log_pop_raw(logger, &hdr, args, sizeof(args), &args_len);
  if (s != PROTON_OK)
  {
    return s;
  }

  proton_Log log = proton_Log_init_zero;
  log.level = (proton_Log_Level)hdr.level;
  log.timestamp_ms = hdr.timestamp_ms;
  log.sequence = hdr.sequence;
  log.dropped_since_last = hdr.dropped_since_last;
  (void)proton_log_format_entry(
    (const char *)hdr.fmt_ref, args, args_len, log.text, sizeof(log.text));

  proton_Proton msg = proton_Proton_init_zero;
  msg.which_operation = proton_Proton_log_tag;
  msg.operation.log = log;

  pb_ostream_t stream = pb_ostream_from_buffer((pb_byte_t *)buffer, buffer_len);
  if (!pb_encode(&stream, proton_Proton_fields, &msg))
  {
    return PROTON_SERIALIZATION_ERROR;
  }
  *out_len = stream.bytes_written;
  return PROTON_OK;
}

proton_status_e proton_log_note_drop(proton_logger_t * logger)
{
  if (logger == NULL)
  {
    return PROTON_NULL_PTR_ERROR;
  }
  proton_status_e s = lock_logger(&logger->config);
  if (s != PROTON_OK)
  {
    return s;
  }
  logger->dropped_since_last++;
  return unlock_logger(&logger->config);
}

static size_t fmt_append(char * out, size_t off, size_t cap, const char * s, size_t n)
{
  if (off >= cap)
  {
    return off;
  }
  size_t room = cap - off;
  size_t copy = (n < room) ? n : room;
  memcpy(out + off, s, copy);
  return off + copy;
}

static size_t fmt_append_cstr(char * out, size_t off, size_t cap, const char * s)
{
  return fmt_append(out, off, cap, s, strlen(s));
}

static size_t fmt_one_arg(
  char * out, size_t off, size_t cap, const uint8_t * args, size_t args_len, size_t * apos)
{
  if (*apos >= args_len)
  {
    return fmt_append_cstr(out, off, cap, "<!args>");
  }
  uint8_t tag = args[(*apos)++];
  char tmp[48];
  int written = 0;

  switch (tag)
  {
    case PROTON_LOG_ARG_I32:
    {
      if (*apos + sizeof(int32_t) > args_len)
      {
        return fmt_append_cstr(out, off, cap, "<!trunc>");
      }
      int32_t v;
      memcpy(&v, args + *apos, sizeof(v));
      *apos += sizeof(v);
      written = snprintf(tmp, sizeof(tmp), "%" PRId32, v);
      break;
    }
    case PROTON_LOG_ARG_U32:
    {
      if (*apos + sizeof(uint32_t) > args_len)
      {
        return fmt_append_cstr(out, off, cap, "<!trunc>");
      }
      uint32_t v;
      memcpy(&v, args + *apos, sizeof(v));
      *apos += sizeof(v);
      written = snprintf(tmp, sizeof(tmp), "%" PRIu32, v);
      break;
    }
    case PROTON_LOG_ARG_I64:
    {
      if (*apos + sizeof(int64_t) > args_len)
      {
        return fmt_append_cstr(out, off, cap, "<!trunc>");
      }
      int64_t v;
      memcpy(&v, args + *apos, sizeof(v));
      *apos += sizeof(v);
      written = snprintf(tmp, sizeof(tmp), "%" PRId64, v);
      break;
    }
    case PROTON_LOG_ARG_U64:
    {
      if (*apos + sizeof(uint64_t) > args_len)
      {
        return fmt_append_cstr(out, off, cap, "<!trunc>");
      }
      uint64_t v;
      memcpy(&v, args + *apos, sizeof(v));
      *apos += sizeof(v);
      written = snprintf(tmp, sizeof(tmp), "%" PRIu64, v);
      break;
    }
    case PROTON_LOG_ARG_F64:
    {
      if (*apos + sizeof(double) > args_len)
      {
        return fmt_append_cstr(out, off, cap, "<!trunc>");
      }
      double v;
      memcpy(&v, args + *apos, sizeof(v));
      *apos += sizeof(v);
      written = snprintf(tmp, sizeof(tmp), "%g", v);
      break;
    }
    case PROTON_LOG_ARG_STR:
    {
      if (*apos + sizeof(uint16_t) > args_len)
      {
        return fmt_append_cstr(out, off, cap, "<!trunc>");
      }
      uint16_t n;
      memcpy(&n, args + *apos, sizeof(n));
      *apos += sizeof(n);
      if (*apos + n > args_len)
      {
        return fmt_append_cstr(out, off, cap, "<!trunc>");
      }
      off = fmt_append(out, off, cap, (const char *)(args + *apos), n);
      *apos += n;
      return off;
    }
    case PROTON_LOG_ARG_PTR:
    {
      if (*apos + sizeof(uint64_t) > args_len)
      {
        return fmt_append_cstr(out, off, cap, "<!trunc>");
      }
      uint64_t v;
      memcpy(&v, args + *apos, sizeof(v));
      *apos += sizeof(v);
      written = snprintf(tmp, sizeof(tmp), "0x%" PRIx64, v);
      break;
    }
    case PROTON_LOG_ARG_CHR:
    {
      if (*apos + sizeof(char) > args_len)
      {
        return fmt_append_cstr(out, off, cap, "<!trunc>");
      }
      char v = (char)args[(*apos)++];
      return fmt_append(out, off, cap, &v, 1u);
    }
    default:
      return fmt_append_cstr(out, off, cap, "<!tag>");
  }

  if (written < 0)
  {
    return fmt_append_cstr(out, off, cap, "<!fmt>");
  }
  return fmt_append(out, off, cap, tmp, (size_t)written);
}

size_t proton_log_format_entry(
  const char * fmt, const uint8_t * args, size_t args_len, char * out, size_t out_cap)
{
  if (out == NULL || out_cap == 0)
  {
    return 0;
  }
  const char * p = (fmt != NULL) ? fmt : "";
  const uint8_t * a = (args != NULL) ? args : (const uint8_t *)"";
  size_t off = 0;
  size_t apos = 0;
  const size_t str_cap = out_cap - 1u;

  while (*p != '\0')
  {
    if (p[0] == '{' && p[1] == '{')
    {
      off = fmt_append(out, off, str_cap, "{", 1u);
      p += 2;
    }
    else if (p[0] == '}' && p[1] == '}')
    {
      off = fmt_append(out, off, str_cap, "}", 1u);
      p += 2;
    }
    else if (p[0] == '{' && p[1] == '}')
    {
      off = fmt_one_arg(out, off, str_cap, a, args_len, &apos);
      p += 2;
    }
    else
    {
      off = fmt_append(out, off, str_cap, p, 1u);
      p += 1;
    }
  }
  out[off] = '\0';
  return off;
}

proton_status_e proton_log_dispatch(proton_logger_t * logger, const proton_Log * log)
{
  if (logger == NULL || log == NULL)
  {
    return PROTON_NULL_PTR_ERROR;
  }
  if (logger->config.on_receive != NULL)
  {
    logger->config.on_receive(log, logger->config.on_receive_arg);
  }
  return PROTON_OK;
}
