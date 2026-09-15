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

#include <gtest/gtest.h>

#include <array>
#include <cstdint>
#include <cstring>

#include "proton/log.h"

#if PROTON_ENABLE_LOGGING

namespace
{

struct LockCounters
{
  int lock_calls = 0;
  int unlock_calls = 0;
  bool lock_ok = true;
  bool unlock_ok = true;
};

extern "C" bool test_lock_fn(void * arg)
{
  auto * c = static_cast<LockCounters *>(arg);
  c->lock_calls++;
  return c->lock_ok;
}

extern "C" bool test_unlock_fn(void * arg)
{
  auto * c = static_cast<LockCounters *>(arg);
  c->unlock_calls++;
  return c->unlock_ok;
}

proton_logger_config_t make_config(proton_Log * entries, size_t capacity)
{
  proton_logger_config_t cfg{};
  cfg.entries = entries;
  cfg.capacity = capacity;
  cfg.min_level = PROTON_LOG_LEVEL_TRACE;
  return cfg;
}

}  // namespace

TEST(LogRing, InitRejectsBadInputs)
{
  proton_logger_t logger;
  std::array<proton_Log, 4> entries{};

  EXPECT_EQ(proton_log_init(nullptr, nullptr), PROTON_NULL_PTR_ERROR);

  proton_logger_config_t cfg = make_config(nullptr, 4);
  EXPECT_EQ(proton_log_init(&logger, &cfg), PROTON_INSUFFICIENT_BUFFER_ERROR);

  cfg = make_config(entries.data(), 0);
  EXPECT_EQ(proton_log_init(&logger, &cfg), PROTON_INSUFFICIENT_BUFFER_ERROR);

  cfg = make_config(entries.data(), entries.size());
  cfg.lock = test_lock_fn;
  cfg.unlock = nullptr;
  EXPECT_EQ(proton_log_init(&logger, &cfg), PROTON_ERROR);
}

TEST(LogRing, PushPopRoundTrip)
{
  proton_logger_t logger;
  std::array<proton_Log, 4> entries{};
  auto cfg = make_config(entries.data(), entries.size());
  ASSERT_EQ(proton_log_init(&logger, &cfg), PROTON_OK);

  const char * text = "hello world";
  ASSERT_EQ(
    proton_log_push(&logger, PROTON_LOG_LEVEL_INFO, 12345, text, std::strlen(text)), PROTON_OK);

  proton_Log out{};
  ASSERT_EQ(proton_log_pop(&logger, &out), PROTON_OK);
  EXPECT_EQ(out.level, proton_Log_Level_LEVEL_INFO);
  EXPECT_EQ(out.timestamp_ms, 12345u);
  EXPECT_EQ(out.sequence, 0u);
  EXPECT_STREQ(out.text, text);
}

TEST(LogRing, PopEmptyReturnsEmpty)
{
  proton_logger_t logger;
  std::array<proton_Log, 2> entries{};
  auto cfg = make_config(entries.data(), entries.size());
  ASSERT_EQ(proton_log_init(&logger, &cfg), PROTON_OK);

  proton_Log out{};
  EXPECT_EQ(proton_log_pop(&logger, &out), PROTON_EMPTY);
}

TEST(LogRing, TruncatesOverlongText)
{
  proton_logger_t logger;
  std::array<proton_Log, 2> entries{};
  auto cfg = make_config(entries.data(), entries.size());
  ASSERT_EQ(proton_log_init(&logger, &cfg), PROTON_OK);

  const size_t max_text = sizeof(entries[0].text) - 1u;
  std::string long_text(max_text + 20, 'x');
  ASSERT_EQ(
    proton_log_push(&logger, PROTON_LOG_LEVEL_INFO, 0, long_text.data(), long_text.size()),
    PROTON_OK);

  proton_Log out{};
  ASSERT_EQ(proton_log_pop(&logger, &out), PROTON_OK);
  EXPECT_EQ(std::strlen(out.text), max_text);
}

TEST(LogRing, ReturnsErrorWhenFull)
{
  proton_logger_t logger;
  std::array<proton_Log, 2> entries{};
  auto cfg = make_config(entries.data(), entries.size());
  ASSERT_EQ(proton_log_init(&logger, &cfg), PROTON_OK);

  ASSERT_EQ(proton_log_push(&logger, 0, 1, "a", 1), PROTON_OK);
  ASSERT_EQ(proton_log_push(&logger, 0, 2, "b", 1), PROTON_OK);
  EXPECT_EQ(proton_log_push(&logger, 0, 3, "c", 1), PROTON_INSUFFICIENT_BUFFER_ERROR);

  proton_Log out{};
  ASSERT_EQ(proton_log_pop(&logger, &out), PROTON_OK);
  EXPECT_EQ(out.timestamp_ms, 1u);

  ASSERT_EQ(proton_log_push(&logger, 0, 5, "e", 1), PROTON_OK);

  ASSERT_EQ(proton_log_pop(&logger, &out), PROTON_OK);
  EXPECT_EQ(out.timestamp_ms, 2u);

  ASSERT_EQ(proton_log_pop(&logger, &out), PROTON_OK);
  EXPECT_EQ(out.timestamp_ms, 5u);
}

TEST(LogRing, WrapAroundPreservesOrder)
{
  proton_logger_t logger;
  std::array<proton_Log, 3> entries{};
  auto cfg = make_config(entries.data(), entries.size());
  ASSERT_EQ(proton_log_init(&logger, &cfg), PROTON_OK);

  for (uint64_t i = 1; i <= 3; ++i)
  {
    ASSERT_EQ(proton_log_push(&logger, 0, i, "x", 1), PROTON_OK);
  }

  proton_Log out{};
  ASSERT_EQ(proton_log_pop(&logger, &out), PROTON_OK);
  EXPECT_EQ(out.timestamp_ms, 1u);
  ASSERT_EQ(proton_log_pop(&logger, &out), PROTON_OK);
  EXPECT_EQ(out.timestamp_ms, 2u);

  ASSERT_EQ(proton_log_push(&logger, 0, 100, "x", 1), PROTON_OK);
  ASSERT_EQ(proton_log_push(&logger, 0, 101, "x", 1), PROTON_OK);

  ASSERT_EQ(proton_log_pop(&logger, &out), PROTON_OK);
  EXPECT_EQ(out.timestamp_ms, 3u);
  ASSERT_EQ(proton_log_pop(&logger, &out), PROTON_OK);
  EXPECT_EQ(out.timestamp_ms, 100u);
  ASSERT_EQ(proton_log_pop(&logger, &out), PROTON_OK);
  EXPECT_EQ(out.timestamp_ms, 101u);
  EXPECT_EQ(proton_log_pop(&logger, &out), PROTON_EMPTY);
}

TEST(LogRing, LockHooksInvokedOnPushAndPop)
{
  proton_logger_t logger;
  std::array<proton_Log, 2> entries{};
  LockCounters counters;
  auto cfg = make_config(entries.data(), entries.size());
  cfg.lock = test_lock_fn;
  cfg.unlock = test_unlock_fn;
  cfg.lock_arg = &counters;
  ASSERT_EQ(proton_log_init(&logger, &cfg), PROTON_OK);

  ASSERT_EQ(proton_log_push(&logger, 0, 1, "x", 1), PROTON_OK);
  EXPECT_EQ(counters.lock_calls, 1);
  EXPECT_EQ(counters.unlock_calls, 1);

  proton_Log out{};
  ASSERT_EQ(proton_log_pop(&logger, &out), PROTON_OK);
  EXPECT_EQ(counters.lock_calls, 2);
  EXPECT_EQ(counters.unlock_calls, 2);

  counters.lock_ok = false;
  EXPECT_EQ(proton_log_push(&logger, 0, 2, "x", 1), PROTON_MUTEX_ERROR);
}

TEST(LogRing, SetDefaultLoggerRoundTrip)
{
  proton_logger_t logger;
  std::array<proton_Log, 2> entries{};
  auto cfg = make_config(entries.data(), entries.size());
  ASSERT_EQ(proton_log_init(&logger, &cfg), PROTON_OK);

  proton_log_set_logger(&logger);
  EXPECT_EQ(proton_log_get_logger(), &logger);
  proton_log_set_logger(nullptr);
  EXPECT_EQ(proton_log_get_logger(), nullptr);
}

int main(int argc, char ** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

#endif  // PROTON_ENABLE_LOGGING
