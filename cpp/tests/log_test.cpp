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

#include <string>

#include <protoncpp/log_access.hpp>

#include <gtest/gtest.h>

using namespace proton;

// -----------------------------------------------------------------------
// Logger<Capacity>
// -----------------------------------------------------------------------

TEST(LoggerTest, PushPopRoundTrip)
{
  Logger<4> logger;
  ASSERT_EQ(logger.push(PROTON_LOG_LEVEL_INFO, "hello", 42u), PROTON_OK);

  auto out = logger.pop();
  ASSERT_TRUE(out.has_value());
  EXPECT_EQ(out->level, proton_Log_Level_LEVEL_INFO);
  EXPECT_EQ(out->timestamp_ms, 42u);
  EXPECT_STREQ(out->text, "hello");
}

TEST(LoggerTest, PopEmptyReturnsNullopt)
{
  Logger<2> logger;
  EXPECT_FALSE(logger.pop().has_value());
}

TEST(LoggerTest, RawExposesUnderlyingLogger)
{
  Logger<2> logger;
  ASSERT_NE(logger.raw(), nullptr);
  ASSERT_EQ(proton_log_push(logger.raw(), PROTON_LOG_LEVEL_ERROR, 1u, "raw", 3u), PROTON_OK);

  proton_Log out{};
  ASSERT_EQ(logger.pop(out), PROTON_OK);
  EXPECT_STREQ(out.text, "raw");
}

TEST(LoggerTest, SetAsDefaultRoutesMacros)
{
  Logger<4> logger;
  logger.set_global_logger();
  EXPECT_EQ(proton_log_get_logger(), logger.raw());
  PROTON_LOG_INFO("hi %u", 7u);
  auto out = logger.pop();
  ASSERT_TRUE(out.has_value());
  EXPECT_STREQ(out->text, "hi 7");
  proton_log_set_logger(nullptr);
}

TEST(LoggerTest, EncodeNextRoundTripsThroughDecode)
{
  Logger<2> logger;
  ASSERT_EQ(logger.push(PROTON_LOG_LEVEL_WARN, "danger", 99u), PROTON_OK);

  std::array<std::uint8_t, 256> buf{};
  auto n = logger.encode_next(std::span<std::uint8_t>{buf});
  ASSERT_TRUE(n.has_value());
  EXPECT_GT(*n, 0u);
}

int main(int argc, char ** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
