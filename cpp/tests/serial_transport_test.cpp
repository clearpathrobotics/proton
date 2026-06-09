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
#include <span>
#include "protoncpp/transport/core_serial.hpp"

using namespace proton::transport::serial;

// -----------------------------------------------------------------------
// fill_frame_header
// -----------------------------------------------------------------------

TEST(FillFrameHeader, NullHeaderReturnsError)
{
  EXPECT_EQ(fill_frame_header(nullptr, 10), PROTON_NULL_PTR_ERROR);
}

TEST(FillFrameHeader, MagicBytesAreCorrect)
{
  uint8_t header[4] = {};
  ASSERT_EQ(fill_frame_header(header, 0), PROTON_OK);
  EXPECT_EQ(header[0], FRAME_HEADER_MAGIC_BYTE_0);
  EXPECT_EQ(header[1], FRAME_HEADER_MAGIC_BYTE_1);
}

TEST(FillFrameHeader, LengthEncodedLittleEndian)
{
  uint8_t header[4] = {};
  ASSERT_EQ(fill_frame_header(header, 0x0203), PROTON_OK);
  EXPECT_EQ(header[2], 0x03);  // low byte
  EXPECT_EQ(header[3], 0x02);  // high byte
}

TEST(FillFrameHeader, ZeroLength)
{
  uint8_t header[4] = {};
  ASSERT_EQ(fill_frame_header(header, 0), PROTON_OK);
  EXPECT_EQ(header[2], 0x00);
  EXPECT_EQ(header[3], 0x00);
}

TEST(FillFrameHeader, MaxLength)
{
  uint8_t header[4] = {};
  ASSERT_EQ(fill_frame_header(header, 0xFFFF), PROTON_OK);
  EXPECT_EQ(header[2], 0xFF);
  EXPECT_EQ(header[3], 0xFF);
}

// -----------------------------------------------------------------------
// fill_crc16
// -----------------------------------------------------------------------

TEST(FillCrc16, NullPayloadReturnsError)
{
  uint8_t crc[2] = {};
  EXPECT_EQ(fill_crc16(nullptr, 0, crc), PROTON_NULL_PTR_ERROR);
}

TEST(FillCrc16, NullCrcOutputReturnsError)
{
  const uint8_t payload[] = {0x01};
  EXPECT_EQ(fill_crc16(payload, 1, nullptr), PROTON_NULL_PTR_ERROR);
}

TEST(FillCrc16, EmptyPayloadCrcIsAllOnes)
{
  // CRC initialises to 0xFFFF with no bytes processed
  const uint8_t dummy[1] = {};
  uint8_t crc[2] = {};
  ASSERT_EQ(fill_crc16(dummy, 0, crc), PROTON_OK);
  EXPECT_EQ(crc[0], 0xFF);  // low byte
  EXPECT_EQ(crc[1], 0xFF);  // high byte
}

TEST(FillCrc16, SingleByteKnownValue)
{
  // CRC16-CCITT of {0xAB} starting at 0xFFFF = 0xE571
  const uint8_t payload[] = {0xAB};
  uint8_t crc[2] = {};
  ASSERT_EQ(fill_crc16(payload, 1, crc), PROTON_OK);
  EXPECT_EQ(crc[0], 0x71);  // low byte
  EXPECT_EQ(crc[1], 0xE5);  // high byte
}

TEST(FillCrc16, DeterministicOutput)
{
  const uint8_t payload[] = {0x01, 0x02, 0x03};
  uint8_t crc_a[2] = {};
  uint8_t crc_b[2] = {};
  ASSERT_EQ(fill_crc16(payload, sizeof(payload), crc_a), PROTON_OK);
  ASSERT_EQ(fill_crc16(payload, sizeof(payload), crc_b), PROTON_OK);
  EXPECT_EQ(crc_a[0], crc_b[0]);
  EXPECT_EQ(crc_a[1], crc_b[1]);
}

TEST(FillCrc16, ByteOrderAffectsCrc)
{
  // Same bytes in different order should produce different CRCs
  const uint8_t payload_a[] = {0x01, 0x02};
  const uint8_t payload_b[] = {0x02, 0x01};
  uint8_t crc_a[2] = {};
  uint8_t crc_b[2] = {};
  ASSERT_EQ(fill_crc16(payload_a, sizeof(payload_a), crc_a), PROTON_OK);
  ASSERT_EQ(fill_crc16(payload_b, sizeof(payload_b), crc_b), PROTON_OK);
  EXPECT_FALSE(crc_a[0] == crc_b[0] && crc_a[1] == crc_b[1]);
}

// -----------------------------------------------------------------------
// check_framed_payload
// -----------------------------------------------------------------------

TEST(CheckFramedPayload, NullPayloadReturnsError)
{
  EXPECT_EQ(check_framed_payload(nullptr, 0, 0xFFFF), PROTON_NULL_PTR_ERROR);
}

TEST(CheckFramedPayload, CorrectCrcReturnsOk)
{
  // Known: CRC16-CCITT of {0xAB} = 0xE571
  const uint8_t payload[] = {0xAB};
  EXPECT_EQ(check_framed_payload(payload, 1, 0xE571), PROTON_OK);
}

TEST(CheckFramedPayload, WrongCrcReturnsCrcError)
{
  const uint8_t payload[] = {0xAB};
  EXPECT_EQ(check_framed_payload(payload, 1, 0x0000), PROTON_CRC16_ERROR);
}

TEST(CheckFramedPayload, EmptyPayloadCorrectCrc)
{
  const uint8_t dummy[1] = {};
  EXPECT_EQ(check_framed_payload(dummy, 0, 0xFFFF), PROTON_OK);
}

TEST(CheckFramedPayload, EmptyPayloadWrongCrc)
{
  const uint8_t dummy[1] = {};
  EXPECT_EQ(check_framed_payload(dummy, 0, 0x0000), PROTON_CRC16_ERROR);
}

// -----------------------------------------------------------------------
// get_framed_payload_length
// -----------------------------------------------------------------------

TEST(GetFramedPayloadLength, NullBufReturnsError)
{
  uint16_t length = 0;
  EXPECT_EQ(get_framed_payload_length(nullptr, &length), PROTON_NULL_PTR_ERROR);
}

TEST(GetFramedPayloadLength, NullLengthOutputReturnsError)
{
  const uint8_t frame[] = {FRAME_HEADER_MAGIC_BYTE_0, FRAME_HEADER_MAGIC_BYTE_1, 0x05, 0x00};
  EXPECT_EQ(get_framed_payload_length(frame, nullptr), PROTON_NULL_PTR_ERROR);
}

TEST(GetFramedPayloadLength, WrongFirstMagicByteReturnsError)
{
  const uint8_t frame[] = {0x00, FRAME_HEADER_MAGIC_BYTE_1, 0x05, 0x00};
  uint16_t length = 0;
  EXPECT_EQ(get_framed_payload_length(frame, &length), PROTON_INVALID_HEADER_ERROR);
}

TEST(GetFramedPayloadLength, WrongSecondMagicByteReturnsError)
{
  const uint8_t frame[] = {FRAME_HEADER_MAGIC_BYTE_0, 0x00, 0x05, 0x00};
  uint16_t length = 0;
  EXPECT_EQ(get_framed_payload_length(frame, &length), PROTON_INVALID_HEADER_ERROR);
}

TEST(GetFramedPayloadLength, ValidHeaderReturnsCorrectLength)
{
  const uint8_t frame[] = {FRAME_HEADER_MAGIC_BYTE_0, FRAME_HEADER_MAGIC_BYTE_1, 0x2A, 0x00};
  uint16_t length = 0;
  ASSERT_EQ(get_framed_payload_length(frame, &length), PROTON_OK);
  EXPECT_EQ(length, 42);
}

TEST(GetFramedPayloadLength, LengthDecodedLittleEndian)
{
  const uint8_t frame[] = {FRAME_HEADER_MAGIC_BYTE_0, FRAME_HEADER_MAGIC_BYTE_1, 0x03, 0x02};
  uint16_t length = 0;
  ASSERT_EQ(get_framed_payload_length(frame, &length), PROTON_OK);
  EXPECT_EQ(length, 0x0203);
}

TEST(GetFramedPayloadLength, ZeroLength)
{
  const uint8_t frame[] = {FRAME_HEADER_MAGIC_BYTE_0, FRAME_HEADER_MAGIC_BYTE_1, 0x00, 0x00};
  uint16_t length = 0;
  ASSERT_EQ(get_framed_payload_length(frame, &length), PROTON_OK);
  EXPECT_EQ(length, 0);
}

// -----------------------------------------------------------------------
// Round-trip integration tests
// -----------------------------------------------------------------------

TEST(SerialFraming, FillHeaderThenGetLength_RoundTrip)
{
  uint8_t header[4] = {};
  const uint16_t expected_len = 137;
  ASSERT_EQ(fill_frame_header(header, expected_len), PROTON_OK);

  uint16_t decoded_len = 0;
  ASSERT_EQ(get_framed_payload_length(header, &decoded_len), PROTON_OK);
  EXPECT_EQ(decoded_len, expected_len);
}

TEST(SerialFraming, FillCrcThenCheckCrc_RoundTrip)
{
  const uint8_t payload[] = {0xDE, 0xAD, 0xBE, 0xEF};
  uint8_t crc_bytes[2] = {};
  ASSERT_EQ(fill_crc16(payload, sizeof(payload), crc_bytes), PROTON_OK);

  const uint16_t crc_val = (uint16_t)crc_bytes[0] | ((uint16_t)crc_bytes[1] << 8);
  EXPECT_EQ(check_framed_payload(payload, sizeof(payload), crc_val), PROTON_OK);
}

TEST(SerialFraming, ModifiedPayloadFailsCrcCheck)
{
  uint8_t payload[] = {0xDE, 0xAD, 0xBE, 0xEF};
  uint8_t crc_bytes[2] = {};
  ASSERT_EQ(fill_crc16(payload, sizeof(payload), crc_bytes), PROTON_OK);

  const uint16_t crc_val = (uint16_t)crc_bytes[0] | ((uint16_t)crc_bytes[1] << 8);
  payload[0] ^= 0x01;  // flip one bit
  EXPECT_EQ(check_framed_payload(payload, sizeof(payload), crc_val), PROTON_CRC16_ERROR);
}

// -----------------------------------------------------------------------
// Span tests (C++20)
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
// fill_crc16 (span overload)
// -----------------------------------------------------------------------

TEST(FillCrc16Span, EmptyPayloadCrcIsAllOnes)
{
  const std::array<uint8_t, 1> dummy = {};
  std::array<uint8_t, 2> crc = {};
  ASSERT_EQ(fill_crc16(std::span{dummy.data(), 0}, crc), PROTON_OK);
  EXPECT_EQ(crc[0], 0xFF);
  EXPECT_EQ(crc[1], 0xFF);
}

TEST(FillCrc16Span, SingleByteKnownValue)
{
  const std::array<uint8_t, 1> payload = {0xAB};
  std::array<uint8_t, 2> crc = {};
  ASSERT_EQ(fill_crc16(payload, crc), PROTON_OK);
  EXPECT_EQ(crc[0], 0x71);
  EXPECT_EQ(crc[1], 0xE5);
}

TEST(FillCrc16Span, DeterministicOutput)
{
  const std::array<uint8_t, 3> payload = {0x01, 0x02, 0x03};
  std::array<uint8_t, 2> crc_a = {};
  std::array<uint8_t, 2> crc_b = {};
  ASSERT_EQ(fill_crc16(payload, crc_a), PROTON_OK);
  ASSERT_EQ(fill_crc16(payload, crc_b), PROTON_OK);
  EXPECT_EQ(crc_a, crc_b);
}

TEST(FillCrc16Span, ByteOrderAffectsCrc)
{
  const std::array<uint8_t, 2> payload_a = {0x01, 0x02};
  const std::array<uint8_t, 2> payload_b = {0x02, 0x01};
  std::array<uint8_t, 2> crc_a = {};
  std::array<uint8_t, 2> crc_b = {};
  ASSERT_EQ(fill_crc16(payload_a, crc_a), PROTON_OK);
  ASSERT_EQ(fill_crc16(payload_b, crc_b), PROTON_OK);
  EXPECT_NE(crc_a, crc_b);
}

// -----------------------------------------------------------------------
// check_framed_payload (span overload)
// -----------------------------------------------------------------------

TEST(CheckFramedPayloadSpan, CorrectCrcReturnsOk)
{
  const std::array<uint8_t, 1> payload = {0xAB};
  EXPECT_EQ(check_framed_payload(payload, 0xE571), PROTON_OK);
}

TEST(CheckFramedPayloadSpan, WrongCrcReturnsCrcError)
{
  const std::array<uint8_t, 1> payload = {0xAB};
  EXPECT_EQ(check_framed_payload(payload, 0x0000), PROTON_CRC16_ERROR);
}

TEST(CheckFramedPayloadSpan, EmptyPayloadCorrectCrc)
{
  const std::array<uint8_t, 1> dummy = {};
  EXPECT_EQ(check_framed_payload(std::span{dummy.data(), 0}, 0xFFFF), PROTON_OK);
}

TEST(CheckFramedPayloadSpan, EmptyPayloadWrongCrc)
{
  const std::array<uint8_t, 1> dummy = {};
  EXPECT_EQ(check_framed_payload(std::span{dummy.data(), 0}, 0x0000), PROTON_CRC16_ERROR);
}

// -----------------------------------------------------------------------
// get_framed_payload_length (span overload)
// -----------------------------------------------------------------------

TEST(GetFramedPayloadLengthSpan, ValidHeaderReturnsCorrectLength)
{
  const std::array<uint8_t, 4> frame = {
    FRAME_HEADER_MAGIC_BYTE_0, FRAME_HEADER_MAGIC_BYTE_1, 0x2A, 0x00};
  uint16_t length = 0;
  ASSERT_EQ(get_framed_payload_length(frame, length), PROTON_OK);
  EXPECT_EQ(length, 42);
}

TEST(GetFramedPayloadLengthSpan, LengthDecodedLittleEndian)
{
  const std::array<uint8_t, 4> frame = {
    FRAME_HEADER_MAGIC_BYTE_0, FRAME_HEADER_MAGIC_BYTE_1, 0x03, 0x02};
  uint16_t length = 0;
  ASSERT_EQ(get_framed_payload_length(frame, length), PROTON_OK);
  EXPECT_EQ(length, 0x0203);
}

TEST(GetFramedPayloadLengthSpan, ZeroLength)
{
  const std::array<uint8_t, 4> frame = {
    FRAME_HEADER_MAGIC_BYTE_0, FRAME_HEADER_MAGIC_BYTE_1, 0x00, 0x00};
  uint16_t length = 0;
  ASSERT_EQ(get_framed_payload_length(frame, length), PROTON_OK);
  EXPECT_EQ(length, 0);
}

TEST(GetFramedPayloadLengthSpan, WrongFirstMagicByteReturnsError)
{
  const std::array<uint8_t, 4> frame = {0x00, FRAME_HEADER_MAGIC_BYTE_1, 0x05, 0x00};
  uint16_t length = 0;
  EXPECT_EQ(get_framed_payload_length(frame, length), PROTON_INVALID_HEADER_ERROR);
}

TEST(GetFramedPayloadLengthSpan, WrongSecondMagicByteReturnsError)
{
  const std::array<uint8_t, 4> frame = {FRAME_HEADER_MAGIC_BYTE_0, 0x00, 0x05, 0x00};
  uint16_t length = 0;
  EXPECT_EQ(get_framed_payload_length(frame, length), PROTON_INVALID_HEADER_ERROR);
}

// -----------------------------------------------------------------------
// Round-trip integration tests (span)
// -----------------------------------------------------------------------

TEST(SerialFramingSpan, FillCrcThenCheckCrc_RoundTrip)
{
  const std::array<uint8_t, 4> payload = {0xDE, 0xAD, 0xBE, 0xEF};
  std::array<uint8_t, 2> crc_bytes = {};
  ASSERT_EQ(fill_crc16(payload, crc_bytes), PROTON_OK);

  const uint16_t crc_val =
    static_cast<uint16_t>(crc_bytes[0]) | (static_cast<uint16_t>(crc_bytes[1]) << 8);
  EXPECT_EQ(check_framed_payload(payload, crc_val), PROTON_OK);
}

TEST(SerialFramingSpan, ModifiedPayloadFailsCrcCheck)
{
  std::array<uint8_t, 4> payload = {0xDE, 0xAD, 0xBE, 0xEF};
  std::array<uint8_t, 2> crc_bytes = {};
  ASSERT_EQ(fill_crc16(payload, crc_bytes), PROTON_OK);

  const uint16_t crc_val =
    static_cast<uint16_t>(crc_bytes[0]) | (static_cast<uint16_t>(crc_bytes[1]) << 8);
  payload[0] ^= 0x01;  // flip one bit
  EXPECT_EQ(check_framed_payload(payload, crc_val), PROTON_CRC16_ERROR);
}

int main(int argc, char ** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
