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
#include <vector>

#include "proton/log.h"

namespace
{

constexpr size_t kHeaderSize = 28;

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

proton_logger_config_t make_config(uint8_t * ring, size_t ring_size)
{
  proton_logger_config_t cfg{};
  cfg.ring_buffer = ring;
  cfg.ring_buffer_size = ring_size;
  cfg.min_level = PROTON_LOG_LEVEL_TRACE;
  return cfg;
}

}  // namespace

TEST(LogRing, InitRejectsBadInputs)
{
  proton_logger_t logger;
  std::array<uint8_t, 128> buf{};

  EXPECT_EQ(proton_log_init(nullptr, nullptr), PROTON_NULL_PTR_ERROR);

  proton_logger_config_t cfg = make_config(nullptr, 128);
  EXPECT_EQ(proton_log_init(&logger, &cfg), PROTON_INSUFFICIENT_BUFFER_ERROR);

  cfg = make_config(buf.data(), 4);
  EXPECT_EQ(proton_log_init(&logger, &cfg), PROTON_INSUFFICIENT_BUFFER_ERROR);

  cfg = make_config(buf.data(), buf.size());
  cfg.lock = test_lock_fn;
  cfg.unlock = nullptr;
  EXPECT_EQ(proton_log_init(&logger, &cfg), PROTON_ERROR);
}

TEST(LogRing, PushPopRoundTrip)
{
  proton_logger_t logger;
  std::array<uint8_t, 256> buf{};
  auto cfg = make_config(buf.data(), buf.size());
  ASSERT_EQ(proton_log_init(&logger, &cfg), PROTON_OK);

  const char * fmt = "hello %d";
  const uint8_t args[] = {0x01, 0x02, 0x03, 0x04};
  ASSERT_EQ(
    proton_log_push_raw(&logger, PROTON_LOG_LEVEL_INFO, 12345, fmt, args, sizeof(args)), PROTON_OK);

  proton_log_entry_header_t hdr{};
  uint8_t args_out[16] = {};
  size_t args_len = 0;
  ASSERT_EQ(proton_log_pop_raw(&logger, &hdr, args_out, sizeof(args_out), &args_len), PROTON_OK);

  EXPECT_EQ(hdr.level, PROTON_LOG_LEVEL_INFO);
  EXPECT_EQ(hdr.timestamp_ms, 12345u);
  EXPECT_EQ(hdr.sequence, 0u);
  EXPECT_EQ(hdr.dropped_since_last, 0u);
  EXPECT_EQ(hdr.fmt_ref, static_cast<const void *>(fmt));
  ASSERT_EQ(args_len, sizeof(args));
  EXPECT_EQ(std::memcmp(args_out, args, sizeof(args)), 0);
}

TEST(LogRing, PopEmptyReturnsEmpty)
{
  proton_logger_t logger;
  std::array<uint8_t, 128> buf{};
  auto cfg = make_config(buf.data(), buf.size());
  ASSERT_EQ(proton_log_init(&logger, &cfg), PROTON_OK);

  proton_log_entry_header_t hdr{};
  uint8_t args_out[4] = {};
  size_t args_len = 0;
  EXPECT_EQ(proton_log_pop_raw(&logger, &hdr, args_out, sizeof(args_out), &args_len), PROTON_EMPTY);
}

TEST(LogRing, InsufficientArgsBufferDoesNotConsume)
{
  proton_logger_t logger;
  std::array<uint8_t, 128> buf{};
  auto cfg = make_config(buf.data(), buf.size());
  ASSERT_EQ(proton_log_init(&logger, &cfg), PROTON_OK);

  const uint8_t args[8] = {1, 2, 3, 4, 5, 6, 7, 8};
  ASSERT_EQ(
    proton_log_push_raw(&logger, PROTON_LOG_LEVEL_WARN, 42, nullptr, args, sizeof(args)),
    PROTON_OK);

  proton_log_entry_header_t hdr{};
  uint8_t small_buf[4] = {};
  size_t args_len = 0;
  EXPECT_EQ(
    proton_log_pop_raw(&logger, &hdr, small_buf, sizeof(small_buf), &args_len),
    PROTON_INSUFFICIENT_BUFFER_ERROR);
  EXPECT_EQ(args_len, sizeof(args));

  uint8_t big_buf[16] = {};
  EXPECT_EQ(proton_log_pop_raw(&logger, &hdr, big_buf, sizeof(big_buf), &args_len), PROTON_OK);
  EXPECT_EQ(std::memcmp(big_buf, args, sizeof(args)), 0);
}

TEST(LogRing, DropWhenFullBumpsDroppedCounter)
{
  proton_logger_t logger;
  std::array<uint8_t, 64> buf{};
  auto cfg = make_config(buf.data(), buf.size());
  ASSERT_EQ(proton_log_init(&logger, &cfg), PROTON_OK);

  // Entries are kHeaderSize (28) + args each. With 64 bytes and args_len=4 → 32 per entry.
  // So exactly 2 entries fit; the 3rd should drop.
  const uint8_t args[4] = {0xAA, 0xBB, 0xCC, 0xDD};
  ASSERT_EQ(proton_log_push_raw(&logger, 0, 1, nullptr, args, sizeof(args)), PROTON_OK);
  ASSERT_EQ(proton_log_push_raw(&logger, 0, 2, nullptr, args, sizeof(args)), PROTON_OK);
  EXPECT_EQ(
    proton_log_push_raw(&logger, 0, 3, nullptr, args, sizeof(args)),
    PROTON_INSUFFICIENT_BUFFER_ERROR);
  EXPECT_EQ(
    proton_log_push_raw(&logger, 0, 4, nullptr, args, sizeof(args)),
    PROTON_INSUFFICIENT_BUFFER_ERROR);

  // Pop one → frees space → next push carries dropped_since_last = 2.
  proton_log_entry_header_t hdr{};
  uint8_t args_out[16] = {};
  size_t args_len = 0;
  ASSERT_EQ(proton_log_pop_raw(&logger, &hdr, args_out, sizeof(args_out), &args_len), PROTON_OK);
  EXPECT_EQ(hdr.timestamp_ms, 1u);
  EXPECT_EQ(hdr.dropped_since_last, 0u);

  ASSERT_EQ(proton_log_push_raw(&logger, 0, 5, nullptr, args, sizeof(args)), PROTON_OK);

  ASSERT_EQ(proton_log_pop_raw(&logger, &hdr, args_out, sizeof(args_out), &args_len), PROTON_OK);
  EXPECT_EQ(hdr.timestamp_ms, 2u);
  EXPECT_EQ(hdr.dropped_since_last, 0u);

  ASSERT_EQ(proton_log_pop_raw(&logger, &hdr, args_out, sizeof(args_out), &args_len), PROTON_OK);
  EXPECT_EQ(hdr.timestamp_ms, 5u);
  EXPECT_EQ(hdr.dropped_since_last, 2u);
}

TEST(LogRing, WrapAroundPreservesOrder)
{
  proton_logger_t logger;
  std::array<uint8_t, 96> buf{};
  auto cfg = make_config(buf.data(), buf.size());
  ASSERT_EQ(proton_log_init(&logger, &cfg), PROTON_OK);

  const uint8_t args[4] = {0x11, 0x22, 0x33, 0x44};
  proton_log_entry_header_t hdr{};
  uint8_t args_out[16] = {};
  size_t args_len = 0;

  // Each entry = 32 bytes. Ring = 96 bytes → 3 entries fit.
  // Push 3, pop 2, push 2, pop remaining 3 → wrap exercised.
  for (uint64_t i = 0; i < 3; ++i)
  {
    ASSERT_EQ(proton_log_push_raw(&logger, 0, i + 1, nullptr, args, sizeof(args)), PROTON_OK);
  }

  ASSERT_EQ(proton_log_pop_raw(&logger, &hdr, args_out, sizeof(args_out), &args_len), PROTON_OK);
  EXPECT_EQ(hdr.timestamp_ms, 1u);
  ASSERT_EQ(proton_log_pop_raw(&logger, &hdr, args_out, sizeof(args_out), &args_len), PROTON_OK);
  EXPECT_EQ(hdr.timestamp_ms, 2u);

  for (uint64_t i = 0; i < 2; ++i)
  {
    ASSERT_EQ(proton_log_push_raw(&logger, 0, 100 + i, nullptr, args, sizeof(args)), PROTON_OK);
  }

  ASSERT_EQ(proton_log_pop_raw(&logger, &hdr, args_out, sizeof(args_out), &args_len), PROTON_OK);
  EXPECT_EQ(hdr.timestamp_ms, 3u);
  ASSERT_EQ(proton_log_pop_raw(&logger, &hdr, args_out, sizeof(args_out), &args_len), PROTON_OK);
  EXPECT_EQ(hdr.timestamp_ms, 100u);
  ASSERT_EQ(proton_log_pop_raw(&logger, &hdr, args_out, sizeof(args_out), &args_len), PROTON_OK);
  EXPECT_EQ(hdr.timestamp_ms, 101u);
  EXPECT_EQ(proton_log_pop_raw(&logger, &hdr, args_out, sizeof(args_out), &args_len), PROTON_EMPTY);
}

TEST(LogRing, VariableSizeEntriesWithWrap)
{
  proton_logger_t logger;
  std::array<uint8_t, 128> buf{};
  auto cfg = make_config(buf.data(), buf.size());
  ASSERT_EQ(proton_log_init(&logger, &cfg), PROTON_OK);

  auto push = [&](uint64_t ts, size_t n)
  {
    std::vector<uint8_t> args(n);
    for (size_t i = 0; i < n; ++i) args[i] = static_cast<uint8_t>(ts + i);
    ASSERT_EQ(proton_log_push_raw(&logger, 0, ts, nullptr, args.data(), n), PROTON_OK);
  };

  auto pop_expect = [&](uint64_t ts, size_t n)
  {
    proton_log_entry_header_t hdr{};
    uint8_t out[64] = {};
    size_t len = 0;
    ASSERT_EQ(proton_log_pop_raw(&logger, &hdr, out, sizeof(out), &len), PROTON_OK);
    EXPECT_EQ(hdr.timestamp_ms, ts);
    ASSERT_EQ(len, n);
    for (size_t i = 0; i < n; ++i) EXPECT_EQ(out[i], static_cast<uint8_t>(ts + i));
  };

  push(10, 8);   // entry size 36
  push(20, 16);  // entry size 44 → total 80
  pop_expect(10, 8);
  push(30, 12);  // entry size 40 → wraps
  push(40, 4);   // entry size 32
  pop_expect(20, 16);
  pop_expect(30, 12);
  pop_expect(40, 4);
  proton_log_entry_header_t hdr{};
  uint8_t out[4];
  size_t len;
  EXPECT_EQ(proton_log_pop_raw(&logger, &hdr, out, sizeof(out), &len), PROTON_EMPTY);
}

TEST(LogRing, LockHooksInvokedOnPushAndPop)
{
  proton_logger_t logger;
  std::array<uint8_t, 128> buf{};
  LockCounters counters;
  auto cfg = make_config(buf.data(), buf.size());
  cfg.lock = test_lock_fn;
  cfg.unlock = test_unlock_fn;
  cfg.lock_arg = &counters;
  ASSERT_EQ(proton_log_init(&logger, &cfg), PROTON_OK);

  const uint8_t args[2] = {0xAB, 0xCD};
  ASSERT_EQ(proton_log_push_raw(&logger, 0, 1, nullptr, args, sizeof(args)), PROTON_OK);
  EXPECT_EQ(counters.lock_calls, 1);
  EXPECT_EQ(counters.unlock_calls, 1);

  proton_log_entry_header_t hdr{};
  uint8_t out[8] = {};
  size_t len = 0;
  ASSERT_EQ(proton_log_pop_raw(&logger, &hdr, out, sizeof(out), &len), PROTON_OK);
  EXPECT_EQ(counters.lock_calls, 2);
  EXPECT_EQ(counters.unlock_calls, 2);

  counters.lock_ok = false;
  EXPECT_EQ(proton_log_push_raw(&logger, 0, 2, nullptr, args, sizeof(args)), PROTON_MUTEX_ERROR);
}

TEST(LogRing, SetDefaultLoggerRoundTrip)
{
  proton_logger_t logger;
  std::array<uint8_t, 64> buf{};
  auto cfg = make_config(buf.data(), buf.size());
  ASSERT_EQ(proton_log_init(&logger, &cfg), PROTON_OK);

  proton_log_set_default(&logger);
  EXPECT_EQ(proton_log_default(), &logger);
  proton_log_set_default(nullptr);
  EXPECT_EQ(proton_log_default(), nullptr);
}

int main(int argc, char ** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
