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
#include <cstring>
#include <span>
#include "protoncpp/transport/core_udp4.hpp"

using namespace proton::transport::udp4;

// -----------------------------------------------------------------------
// fill_header
// -----------------------------------------------------------------------

TEST(FillHeader, SetsCurrentVersion)
{
  Header header = {};
  ASSERT_EQ(fill_header(header, 0, 0), PROTON_OK);
  EXPECT_EQ(header.version, CURRENT_VERSION);
}

TEST(FillHeader, SetsNodeId)
{
  Header header = {};
  ASSERT_EQ(fill_header(header, 42, 0), PROTON_OK);
  EXPECT_EQ(header.node_id, 42);
}

TEST(FillHeader, SetsFlags)
{
  Header header = {};
  ASSERT_EQ(fill_header(header, 0, 0xAB), PROTON_OK);
  EXPECT_EQ(header.flags, 0xAB);
}

TEST(FillHeader, ReservedByteIsZero)
{
  Header header = {};
  header.reserved = 0xFF;  // pre-dirty the field
  ASSERT_EQ(fill_header(header, 0, 0), PROTON_OK);
  EXPECT_EQ(header.reserved, 0x00);
}

TEST(FillHeader, ZeroNodeIdAndFlagsIsValid)
{
  Header header = {};
  EXPECT_EQ(fill_header(header, 0, 0), PROTON_OK);
  EXPECT_EQ(header.node_id, 0);
  EXPECT_EQ(header.flags, 0);
}

// -----------------------------------------------------------------------
// check_payload
// -----------------------------------------------------------------------

TEST(CheckPayload, NullPayloadReturnsError)
{
  Header out = {};
  EXPECT_EQ(check_payload(nullptr, 4, out), PROTON_NULL_PTR_ERROR);
}

TEST(CheckPayload, PayloadShorterThanHeaderReturnsError)
{
  const uint8_t payload[2] = {CURRENT_VERSION, 0};
  Header out = {};
  EXPECT_EQ(check_payload(payload, sizeof(payload), out), PROTON_INSUFFICIENT_BUFFER_ERROR);
}

TEST(CheckPayload, EmptyPayloadReturnsError)
{
  const uint8_t payload[4] = {};
  Header out = {};
  EXPECT_EQ(check_payload(payload, 0, out), PROTON_INSUFFICIENT_BUFFER_ERROR);
}

TEST(CheckPayload, ValidV2PayloadCopiesHeader)
{
  // Build a raw payload whose first 4 bytes are a valid v2 header
  uint8_t payload[8] = {};
  payload[0] = (uint8_t)UDP4_VERSION_2;
  payload[1] = 7;     // node_id
  payload[2] = 0x0F;  // flags
  payload[3] = 0;     // reserved

  Header out = {};
  ASSERT_EQ(check_payload(payload, sizeof(payload), out), PROTON_OK);
  EXPECT_EQ(out.version, (uint8_t)UDP4_VERSION_2);
  EXPECT_EQ(out.node_id, 7);
  EXPECT_EQ(out.flags, 0x0F);
  EXPECT_EQ(out.reserved, 0);
}

TEST(CheckPayload, V1PayloadReturnsVersion1WithZeroedFields)
{
  // Version 1 had no header — version byte will be 0 (UDP4_VERSION_1)
  uint8_t payload[8] = {};
  payload[0] = (uint8_t)UDP4_VERSION_1;
  payload[1] = 0xFF;  // garbage that should not appear in out_header
  payload[2] = 0xFF;
  payload[3] = 0xFF;

  Header out = {};
  ASSERT_EQ(check_payload(payload, sizeof(payload), out), PROTON_OK);
  EXPECT_EQ(out.version, (uint8_t)UDP4_VERSION_1);
  EXPECT_EQ(out.node_id, 0);
  EXPECT_EQ(out.flags, 0);
  EXPECT_EQ(out.reserved, 0);
}

TEST(CheckPayload, UnknownVersionReturnsVersion1WithZeroedFields)
{
  uint8_t payload[8] = {};
  payload[0] = 0xFF;  // unknown version

  Header out = {};
  ASSERT_EQ(check_payload(payload, sizeof(payload), out), PROTON_OK);
  EXPECT_EQ(out.version, (uint8_t)UDP4_VERSION_1);
  EXPECT_EQ(out.node_id, 0);
  EXPECT_EQ(out.flags, 0);
}

TEST(CheckPayload, ExactlyHeaderSizePayloadIsAccepted)
{
  uint8_t payload[sizeof(Header)] = {};
  payload[0] = (uint8_t)UDP4_VERSION_2;
  payload[1] = 3;

  Header out = {};
  ASSERT_EQ(check_payload(payload, sizeof(Header), out), PROTON_OK);
  EXPECT_EQ(out.version, (uint8_t)UDP4_VERSION_2);
  EXPECT_EQ(out.node_id, 3);
}

// -----------------------------------------------------------------------
// Round-trip: fill_header -> check_payload
// -----------------------------------------------------------------------

TEST(Udp4RoundTrip, FilledHeaderParsesBack)
{
  Header filled = {};
  ASSERT_EQ(fill_header(filled, 12, 0x03), PROTON_OK);

  // Place the header at the front of a payload buffer
  uint8_t payload[16] = {};
  memcpy(payload, &filled, sizeof(filled));

  Header parsed = {};
  ASSERT_EQ(check_payload(payload, sizeof(payload), parsed), PROTON_OK);

  EXPECT_EQ(parsed.version, filled.version);
  EXPECT_EQ(parsed.node_id, filled.node_id);
  EXPECT_EQ(parsed.flags, filled.flags);
  EXPECT_EQ(parsed.reserved, filled.reserved);
}

// -----------------------------------------------------------------------
// check_payload span overload
// -----------------------------------------------------------------------

TEST(CheckPayloadSpan, PayloadShorterThanHeaderReturnsError)
{
  const std::array<uint8_t, 2> payload = {CURRENT_VERSION, 0};
  Header out = {};
  EXPECT_EQ(check_payload(payload, out), PROTON_INSUFFICIENT_BUFFER_ERROR);
}

TEST(CheckPayloadSpan, ValidV2PayloadCopiesHeader)
{
  // Build a raw payload whose first 4 bytes are a valid v2 header
  std::array<uint8_t, 8> payload = {};
  payload[0] = (uint8_t)UDP4_VERSION_2;
  payload[1] = 7;     // node_id
  payload[2] = 0x0F;  // flags
  payload[3] = 0;     // reserved

  Header out = {};
  ASSERT_EQ(check_payload(payload, out), PROTON_OK);
  EXPECT_EQ(out.version, (uint8_t)UDP4_VERSION_2);
  EXPECT_EQ(out.node_id, 7);
  EXPECT_EQ(out.flags, 0x0F);
  EXPECT_EQ(out.reserved, 0);
}

TEST(CheckPayloadSpan, V1PayloadReturnsVersion1WithZeroedFields)
{
  // Version 1 had no header — version byte will be 0 (UDP4_VERSION_1)
  std::array<uint8_t, 8> payload = {};
  payload[0] = (uint8_t)UDP4_VERSION_1;
  payload[1] = 0xFF;  // garbage that should not appear in out_header
  payload[2] = 0xFF;
  payload[3] = 0xFF;

  Header out = {};
  ASSERT_EQ(check_payload(payload, out), PROTON_OK);
  EXPECT_EQ(out.version, (uint8_t)UDP4_VERSION_1);
  EXPECT_EQ(out.node_id, 0);
  EXPECT_EQ(out.flags, 0);
  EXPECT_EQ(out.reserved, 0);
}

TEST(CheckPayloadSpan, UnknownVersionReturnsVersion1WithZeroedFields)
{
  std::array<uint8_t, 8> payload = {};
  payload[0] = 0xFF;  // unknown version

  Header out = {};
  ASSERT_EQ(check_payload(payload, out), PROTON_OK);
  EXPECT_EQ(out.version, (uint8_t)UDP4_VERSION_1);
  EXPECT_EQ(out.node_id, 0);
  EXPECT_EQ(out.flags, 0);
}

TEST(CheckPayloadSpan, ExactlyHeaderSizePayloadIsAccepted)
{
  std::array<uint8_t, sizeof(Header)> payload = {};
  payload[0] = (uint8_t)UDP4_VERSION_2;
  payload[1] = 3;

  Header out = {};
  ASSERT_EQ(check_payload(payload, out), PROTON_OK);
  EXPECT_EQ(out.version, (uint8_t)UDP4_VERSION_2);
  EXPECT_EQ(out.node_id, 3);
}

int main(int argc, char ** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
