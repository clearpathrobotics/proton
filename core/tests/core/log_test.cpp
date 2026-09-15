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

#include "pb_decode.h"
#include "proton/generated/proton.pb.h"
#include "proton/log.h"

#if PROTON_ENABLE_LOGGING

namespace
{

uint64_t g_now_ms = 0;

extern "C" uint64_t test_now_ms(void) { return g_now_ms; }

proton_logger_config_t make_config(proton_Log * entries, size_t capacity)
{
  proton_logger_config_t cfg{};
  cfg.entries = entries;
  cfg.capacity = capacity;
  cfg.min_level = PROTON_LOG_LEVEL_TRACE;
  cfg.now_ms = test_now_ms;
  return cfg;
}

}  // namespace

TEST(LogMacro, PushesFormattedAndDrainsRoundTrip)
{
  proton_logger_t logger;
  std::array<proton_Log, 4> entries{};
  auto cfg = make_config(entries.data(), entries.size());
  ASSERT_EQ(proton_log_init(&logger, &cfg), PROTON_OK);
  proton_log_set_logger(&logger);

  g_now_ms = 9999;
  PROTON_LOG_INFO("motor %u at %d rpm", 7u, -1234);

  std::array<uint8_t, 256> wire{};
  size_t wire_len = 0;
  ASSERT_EQ(proton_log_encode_next(&logger, wire.data(), wire.size(), &wire_len), PROTON_OK);
  ASSERT_GT(wire_len, 0u);

  proton_Proton msg = proton_Proton_init_zero;
  pb_istream_t is = pb_istream_from_buffer(wire.data(), wire_len);
  ASSERT_TRUE(pb_decode(&is, proton_Proton_fields, &msg));
  ASSERT_EQ(msg.which_operation, (pb_size_t)proton_Proton_log_tag);

  EXPECT_EQ(msg.operation.log.level, proton_Log_Level_LEVEL_INFO);
  EXPECT_EQ(msg.operation.log.timestamp_ms, 9999u);
  EXPECT_EQ(msg.operation.log.sequence, 0u);
  EXPECT_STREQ(msg.operation.log.text, "motor 7 at -1234 rpm");

  proton_log_set_logger(nullptr);
}

TEST(LogMacro, RespectsRuntimeMinLevel)
{
  proton_logger_t logger;
  std::array<proton_Log, 4> entries{};
  auto cfg = make_config(entries.data(), entries.size());
  cfg.min_level = PROTON_LOG_LEVEL_WARN;
  ASSERT_EQ(proton_log_init(&logger, &cfg), PROTON_OK);
  proton_log_set_logger(&logger);

  PROTON_LOG_INFO("suppressed");
  PROTON_LOG_ERROR("kept %u", 42u);

  std::array<uint8_t, 256> wire{};
  size_t wire_len = 0;
  ASSERT_EQ(proton_log_encode_next(&logger, wire.data(), wire.size(), &wire_len), PROTON_OK);

  proton_Proton msg = proton_Proton_init_zero;
  pb_istream_t is = pb_istream_from_buffer(wire.data(), wire_len);
  ASSERT_TRUE(pb_decode(&is, proton_Proton_fields, &msg));
  EXPECT_STREQ(msg.operation.log.text, "kept 42");

  EXPECT_EQ(proton_log_encode_next(&logger, wire.data(), wire.size(), &wire_len), PROTON_EMPTY);
  proton_log_set_logger(nullptr);
}

TEST(LogMacro, ZeroArgsWorks)
{
  proton_logger_t logger;
  std::array<proton_Log, 4> entries{};
  auto cfg = make_config(entries.data(), entries.size());
  ASSERT_EQ(proton_log_init(&logger, &cfg), PROTON_OK);
  proton_log_set_logger(&logger);

  PROTON_LOG_DEBUG("no args here");

  std::array<uint8_t, 256> wire{};
  size_t wire_len = 0;
  ASSERT_EQ(proton_log_encode_next(&logger, wire.data(), wire.size(), &wire_len), PROTON_OK);

  proton_Proton msg = proton_Proton_init_zero;
  pb_istream_t is = pb_istream_from_buffer(wire.data(), wire_len);
  ASSERT_TRUE(pb_decode(&is, proton_Proton_fields, &msg));
  EXPECT_STREQ(msg.operation.log.text, "no args here");

  proton_log_set_logger(nullptr);
}

TEST(LogMacro, TruncatesOverlongFormattedText)
{
  proton_logger_t logger;
  std::array<proton_Log, 2> entries{};
  auto cfg = make_config(entries.data(), entries.size());
  ASSERT_EQ(proton_log_init(&logger, &cfg), PROTON_OK);
  proton_log_set_logger(&logger);

  std::string big(PROTON_LOG_MAX_MESSAGE_SIZE + 40u, 'x');
  PROTON_LOG_INFO("%s", big.c_str());

  proton_Log out{};
  ASSERT_EQ(proton_log_pop(&logger, &out), PROTON_OK);
  EXPECT_EQ(std::strlen(out.text), PROTON_LOG_MAX_MESSAGE_SIZE - 1u);

  proton_log_set_logger(nullptr);
}

int main(int argc, char ** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

#endif  // PROTON_ENABLE_LOGGING
