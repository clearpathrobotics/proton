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

#ifndef PROTONCPP_LOG_ACCESS_HPP
#define PROTONCPP_LOG_ACCESS_HPP

#include <array>
#include <cstddef>
#include <cstdint>

#include "proton/log.h"

#if __cplusplus >= 201703L
#include <optional>
#include <string_view>
#endif

#if __cplusplus >= 202002L
#include <span>
#endif

#if PROTON_ENABLE_LOGGING

namespace proton
{

template <std::size_t Capacity>
class Logger
{
public:
  static_assert(Capacity >= 1u, "Logger capacity must be at least 1");

  explicit Logger(proton_logger_config_t config = {}) noexcept
  {
    config.entries = entries_.data();
    config.capacity = entries_.size();
    (void)proton_log_init(&logger_, &config);
  }

  Logger(const Logger &) = delete;
  Logger & operator=(const Logger &) = delete;
  Logger(Logger &&) = delete;
  Logger & operator=(Logger &&) = delete;

  proton_logger_t * raw() noexcept { return &logger_; }
  const proton_logger_t * raw() const noexcept { return &logger_; }

  void set_global_logger() noexcept { proton_log_set_logger(&logger_); }

  proton_status_e set_min_level(proton_log_level_e level) noexcept
  {
    return proton_log_set_min_level(&logger_, level);
  }

  proton_status_e push(
    std::uint8_t level, std::uint64_t timestamp_ms, const char * text,
    std::size_t text_len) noexcept
  {
    return proton_log_push(&logger_, level, timestamp_ms, text, text_len);
  }

#if __cplusplus >= 201703L
  proton_status_e push(
    proton_log_level_e level, std::string_view text, std::uint64_t timestamp_ms = 0u) noexcept
  {
    return proton_log_push(
      &logger_, static_cast<std::uint8_t>(level), timestamp_ms, text.data(), text.size());
  }
#endif

  proton_status_e pop(proton_Log & out) noexcept { return proton_log_pop(&logger_, &out); }

#if __cplusplus >= 201703L
  std::optional<proton_Log> pop() noexcept
  {
    proton_Log out{};
    if (proton_log_pop(&logger_, &out) != PROTON_OK)
    {
      return std::nullopt;
    }
    return out;
  }
#endif

  proton_status_e encode_next(
    std::uint8_t * buffer, std::size_t buffer_len, std::size_t & out_len) noexcept
  {
    return proton_log_encode_next(&logger_, buffer, buffer_len, &out_len);
  }

#if __cplusplus >= 202002L
  std::optional<std::size_t> encode_next(std::span<std::uint8_t> buffer) noexcept
  {
    std::size_t out_len = 0;
    if (proton_log_encode_next(&logger_, buffer.data(), buffer.size(), &out_len) != PROTON_OK)
    {
      return std::nullopt;
    }
    return out_len;
  }
#endif

private:
  proton_logger_t logger_{};
  std::array<proton_Log, Capacity> entries_{};
};

}  // namespace proton

#endif  // PROTON_ENABLE_LOGGING

#endif  // PROTONCPP_LOG_ACCESS_HPP
