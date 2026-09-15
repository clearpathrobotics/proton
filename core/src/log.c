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

#include <string.h>

#include "pb_encode.h"
#include "proton/generated/proton.pb.h"

static proton_logger_t * s_default_logger = NULL;

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
  if (config->entries == NULL || config->capacity == 0)
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

proton_status_e proton_log_push(
  proton_logger_t * logger, uint8_t level, uint64_t timestamp_ms, const char * text,
  size_t text_len)
{
  if (logger == NULL || (text_len > 0 && text == NULL))
  {
    return PROTON_NULL_PTR_ERROR;
  }

  proton_status_e s = lock_logger(&logger->config);
  if (s != PROTON_OK)
  {
    return s;
  }

  if (logger->count >= logger->config.capacity)
  {
    (void)unlock_logger(&logger->config);
    return PROTON_INSUFFICIENT_BUFFER_ERROR;
  }

  proton_Log * slot = &logger->config.entries[logger->head];
  const size_t max_text = sizeof(slot->text) - 1u;
  const size_t copy_len = (text_len > max_text) ? max_text : text_len;

  slot->level = (proton_Log_Level)level;
  slot->timestamp_ms = timestamp_ms;
  slot->sequence = logger->sequence++;
  if (copy_len > 0)
  {
    memcpy(slot->text, text, copy_len);
  }
  slot->text[copy_len] = '\0';

  logger->head = (logger->head + 1u) % logger->config.capacity;
  logger->count++;

  return unlock_logger(&logger->config);
}

proton_status_e proton_log_pop(proton_logger_t * logger, proton_Log * out)
{
  if (logger == NULL || out == NULL)
  {
    return PROTON_NULL_PTR_ERROR;
  }

  proton_status_e s = lock_logger(&logger->config);
  if (s != PROTON_OK)
  {
    return s;
  }

  if (logger->count == 0)
  {
    (void)unlock_logger(&logger->config);
    return PROTON_EMPTY;
  }

  *out = logger->config.entries[logger->tail];
  logger->tail = (logger->tail + 1u) % logger->config.capacity;
  logger->count--;

  return unlock_logger(&logger->config);
}

proton_status_e proton_log_encode_next(
  proton_logger_t * logger, uint8_t * buffer, size_t buffer_len, size_t * out_len)
{
  if (logger == NULL || buffer == NULL || out_len == NULL)
  {
    return PROTON_NULL_PTR_ERROR;
  }

  proton_Proton msg = proton_Proton_init_zero;
  msg.which_operation = proton_Proton_log_tag;
  proton_status_e s = proton_log_pop(logger, &msg.operation.log);
  if (s != PROTON_OK)
  {
    return s;
  }

  pb_ostream_t stream = pb_ostream_from_buffer((pb_byte_t *)buffer, buffer_len);
  if (!pb_encode(&stream, proton_Proton_fields, &msg))
  {
    return PROTON_SERIALIZATION_ERROR;
  }
  *out_len = stream.bytes_written;
  return PROTON_OK;
}
