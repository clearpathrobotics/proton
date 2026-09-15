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

static proton_logger_t * s_default_logger = NULL;

proton_status_e proton_log_init(proton_logger_t * logger, const proton_logger_config_t * config)
{
  if (logger == NULL || config == NULL)
  {
    return PROTON_NULL_PTR_ERROR;
  }
  if (config->ring_buffer == NULL || config->ring_buffer_size == 0)
  {
    return PROTON_NULL_PTR_ERROR;
  }
  // lock/unlock must be paired.
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
  (void)logger;
  (void)level;
  (void)timestamp_ms;
  (void)fmt_ref;
  (void)args_blob;
  (void)args_len;
  return PROTON_UNSUPPORTED_OPERATION_ERROR;
}

proton_status_e proton_log_drain(
  proton_logger_t * logger, uint8_t * buffer, size_t buffer_len, size_t * out_len)
{
  (void)logger;
  (void)buffer;
  (void)buffer_len;
  (void)out_len;
  return PROTON_UNSUPPORTED_OPERATION_ERROR;
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
