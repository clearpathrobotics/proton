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
#include <string>

#include "pb_decode.h"
#include "proton/generated/proton.pb.h"
#include "proton/log.h"

namespace
{

uint64_t g_now_ms = 0;

extern "C" uint64_t test_now_ms(void) { return g_now_ms; }

proton_logger_config_t make_config(uint8_t * ring, size_t ring_size)
{
  proton_logger_config_t cfg{};
  cfg.ring_buffer = ring;
  cfg.ring_buffer_size = ring_size;
  cfg.min_level = PROTON_LOG_LEVEL_TRACE;
  cfg.now_ms = test_now_ms;
  return cfg;
}

std::string format_with(const char * fmt, const uint8_t * args, size_t args_len)
{
  char out[256] = {};
  size_t n = proton_log_format_entry(fmt, args, args_len, out, sizeof(out));
  return std::string(out, n);
}

}  // namespace

// ---- Format walker ---------------------------------------------------------

TEST(LogFormat, LiteralPassthrough)
{
  EXPECT_EQ(format_with("hello world", nullptr, 0), "hello world");
}

TEST(LogFormat, EscapedBraces) { EXPECT_EQ(format_with("{{ and }}", nullptr, 0), "{ and }"); }

TEST(LogFormat, EmptyFormatWithArgsIgnoresArgs)
{
  uint8_t buf[64];
  size_t off = 0;
  off = proton_log_enc_i32_(buf, off, sizeof(buf), 42);
  EXPECT_EQ(format_with("", buf, off), "");
}

TEST(LogFormat, MixedTypes)
{
  uint8_t buf[64];
  size_t off = 0;
  off = proton_log_enc_i32_(buf, off, sizeof(buf), -7);
  off = proton_log_enc_u32_(buf, off, sizeof(buf), 123u);
  off = proton_log_enc_str_(buf, off, sizeof(buf), "abc");
  off = proton_log_enc_f64_(buf, off, sizeof(buf), 3.5);
  EXPECT_EQ(format_with("i={} u={} s={} f={}", buf, off), "i=-7 u=123 s=abc f=3.5");
}

TEST(LogFormat, MissingArgProducesPlaceholder)
{
  EXPECT_EQ(format_with("x={}", nullptr, 0), "x=<!args>");
}

TEST(LogFormat, TruncatesToOutputCapacity)
{
  char out[8] = {};
  uint8_t buf[16];
  size_t off = proton_log_enc_i32_(buf, 0, sizeof(buf), 123456);
  size_t n = proton_log_format_entry("val={}", buf, off, out, sizeof(out));
  EXPECT_EQ(n, sizeof(out) - 1u);
  EXPECT_EQ(std::string(out), "val=123");
}

// ---- _Generic macro --------------------------------------------------------

TEST(LogEncode, GenericDispatchesByType)
{
  uint8_t buf[128];
  size_t off = 0;
  int32_t si = -1;
  uint32_t ui = 2u;
  int64_t sl = -3;
  uint64_t ul = 4u;
  double d = 5.5;
  const char * s = "hi";

  off = PROTON_LOG_ENCODE_ARG(buf, off, sizeof(buf), si);
  off = PROTON_LOG_ENCODE_ARG(buf, off, sizeof(buf), ui);
  off = PROTON_LOG_ENCODE_ARG(buf, off, sizeof(buf), sl);
  off = PROTON_LOG_ENCODE_ARG(buf, off, sizeof(buf), ul);
  off = PROTON_LOG_ENCODE_ARG(buf, off, sizeof(buf), d);
  off = PROTON_LOG_ENCODE_ARG(buf, off, sizeof(buf), s);

  ASSERT_NE(off, SIZE_MAX);
  EXPECT_EQ(format_with("{}/{}/{}/{}/{}/{}", buf, off), "-1/2/-3/4/5.5/hi");
}

TEST(LogEncode, OverflowSignaledBySizeMax)
{
  uint8_t buf[4];
  size_t off = PROTON_LOG_ENCODE_ARG(buf, 0u, sizeof(buf), (int64_t)1);
  EXPECT_EQ(off, SIZE_MAX);
}

// ---- Macro end-to-end ------------------------------------------------------

TEST(LogMacro, PushesFormattedAndDrainsRoundTrip)
{
  proton_logger_t logger;
  std::array<uint8_t, 512> ring{};
  auto cfg = make_config(ring.data(), ring.size());
  ASSERT_EQ(proton_log_init(&logger, &cfg), PROTON_OK);
  proton_log_set_default(&logger);

  g_now_ms = 9999;
  PROTON_LOG_INFO("motor {} at {} rpm", (uint32_t)7, (int32_t)-1234);

  std::array<uint8_t, 256> wire{};
  size_t wire_len = 0;
  ASSERT_EQ(proton_log_drain(&logger, wire.data(), wire.size(), &wire_len), PROTON_OK);
  ASSERT_GT(wire_len, 0u);

  proton_Proton msg = proton_Proton_init_zero;
  pb_istream_t is = pb_istream_from_buffer(wire.data(), wire_len);
  ASSERT_TRUE(pb_decode(&is, proton_Proton_fields, &msg));
  ASSERT_EQ(msg.which_operation, (pb_size_t)proton_Proton_log_tag);

  EXPECT_EQ(msg.operation.log.level, proton_Log_Level_LEVEL_INFO);
  EXPECT_EQ(msg.operation.log.timestamp_ms, 9999u);
  EXPECT_EQ(msg.operation.log.sequence, 0u);
  EXPECT_STREQ(msg.operation.log.text, "motor 7 at -1234 rpm");

  proton_log_set_default(nullptr);
}

TEST(LogMacro, RespectsRuntimeMinLevel)
{
  proton_logger_t logger;
  std::array<uint8_t, 256> ring{};
  auto cfg = make_config(ring.data(), ring.size());
  cfg.min_level = PROTON_LOG_LEVEL_WARN;
  ASSERT_EQ(proton_log_init(&logger, &cfg), PROTON_OK);
  proton_log_set_default(&logger);

  PROTON_LOG_INFO("suppressed");
  PROTON_LOG_ERROR("kept {}", (uint32_t)42);

  std::array<uint8_t, 256> wire{};
  size_t wire_len = 0;
  ASSERT_EQ(proton_log_drain(&logger, wire.data(), wire.size(), &wire_len), PROTON_OK);

  proton_Proton msg = proton_Proton_init_zero;
  pb_istream_t is = pb_istream_from_buffer(wire.data(), wire_len);
  ASSERT_TRUE(pb_decode(&is, proton_Proton_fields, &msg));
  EXPECT_STREQ(msg.operation.log.text, "kept 42");

  EXPECT_EQ(proton_log_drain(&logger, wire.data(), wire.size(), &wire_len), PROTON_EMPTY);
  proton_log_set_default(nullptr);
}

TEST(LogMacro, ZeroArgsWorks)
{
  proton_logger_t logger;
  std::array<uint8_t, 256> ring{};
  auto cfg = make_config(ring.data(), ring.size());
  ASSERT_EQ(proton_log_init(&logger, &cfg), PROTON_OK);
  proton_log_set_default(&logger);

  PROTON_LOG_DEBUG("no args here");

  std::array<uint8_t, 256> wire{};
  size_t wire_len = 0;
  ASSERT_EQ(proton_log_drain(&logger, wire.data(), wire.size(), &wire_len), PROTON_OK);

  proton_Proton msg = proton_Proton_init_zero;
  pb_istream_t is = pb_istream_from_buffer(wire.data(), wire_len);
  ASSERT_TRUE(pb_decode(&is, proton_Proton_fields, &msg));
  EXPECT_STREQ(msg.operation.log.text, "no args here");

  proton_log_set_default(nullptr);
}

TEST(LogMacro, DispatchesDroppedCounterAcrossEntries)
{
  proton_logger_t logger;
  std::array<uint8_t, 128> ring{};
  auto cfg = make_config(ring.data(), ring.size());
  ASSERT_EQ(proton_log_init(&logger, &cfg), PROTON_OK);
  proton_log_set_default(&logger);

  // Two entries fit; the third gets dropped due to ring pressure.
  PROTON_LOG_INFO("first");
  PROTON_LOG_INFO("second");
  PROTON_LOG_INFO("third has a longer body to force a drop xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");

  std::array<uint8_t, 256> wire{};
  size_t wire_len = 0;
  ASSERT_EQ(proton_log_drain(&logger, wire.data(), wire.size(), &wire_len), PROTON_OK);
  ASSERT_EQ(proton_log_drain(&logger, wire.data(), wire.size(), &wire_len), PROTON_OK);

  proton_Proton msg = proton_Proton_init_zero;
  pb_istream_t is = pb_istream_from_buffer(wire.data(), wire_len);
  ASSERT_TRUE(pb_decode(&is, proton_Proton_fields, &msg));
  EXPECT_STREQ(msg.operation.log.text, "second");
  EXPECT_GE(msg.operation.log.dropped_since_last, 0u);

  proton_log_set_default(nullptr);
}

TEST(LogNoteDrop, BumpsDroppedCounterOnNextPush)
{
  proton_logger_t logger;
  std::array<uint8_t, 256> ring{};
  auto cfg = make_config(ring.data(), ring.size());
  ASSERT_EQ(proton_log_init(&logger, &cfg), PROTON_OK);

  ASSERT_EQ(proton_log_note_drop(&logger), PROTON_OK);
  ASSERT_EQ(proton_log_note_drop(&logger), PROTON_OK);

  const char * fmt = "x";
  ASSERT_EQ(proton_log_push_raw(&logger, PROTON_LOG_LEVEL_INFO, 0u, fmt, nullptr, 0u), PROTON_OK);

  proton_log_entry_header_t hdr{};
  uint8_t args[4] = {};
  size_t args_len = 0;
  ASSERT_EQ(proton_log_pop_raw(&logger, &hdr, args, sizeof(args), &args_len), PROTON_OK);
  EXPECT_EQ(hdr.dropped_since_last, 2u);
}
