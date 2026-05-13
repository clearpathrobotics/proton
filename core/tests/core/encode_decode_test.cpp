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

#include "proton/encode_decode.h"
#include <gtest/gtest.h>
#include <cstring>
#include "proton/registry.h"

static constexpr size_t BUFFER_SIZE = 1024;

// Helpers

static proton_buffer_t make_buffer(uint8_t * data, size_t len)
{
  proton_buffer_t buf;
  buf.data = data;
  buf.len = len;
  return buf;
}

// -----------------------------------------------------------------------
// Encode
// -----------------------------------------------------------------------

TEST(EncodeDecode, EncodeDefaultValueTestBundle)
{
  uint8_t raw[BUFFER_SIZE];
  proton_buffer_t buf = make_buffer(raw, sizeof(raw));
  size_t bytes_encoded = 0;

  proton_status_e status =
    proton_encode_bundle(PROTON_BUNDLE_DEFAULT_VALUE_TEST_ID, buf, &bytes_encoded);

  EXPECT_EQ(status, PROTON_OK);
  EXPECT_GT(bytes_encoded, 0);
}

TEST(EncodeDecode, EncodeNullBufferReturnsError)
{
  proton_buffer_t buf;
  buf.data = nullptr;
  buf.len = BUFFER_SIZE;
  size_t bytes_encoded = 0;

  EXPECT_EQ(
    proton_encode_bundle(PROTON_BUNDLE_DEFAULT_VALUE_TEST_ID, buf, &bytes_encoded),
    PROTON_NULL_PTR_ERROR);
}

TEST(EncodeDecode, EncodeNullBytesEncodedReturnsError)
{
  uint8_t raw[BUFFER_SIZE];
  proton_buffer_t buf = make_buffer(raw, sizeof(raw));

  EXPECT_EQ(
    proton_encode_bundle(PROTON_BUNDLE_DEFAULT_VALUE_TEST_ID, buf, nullptr), PROTON_NULL_PTR_ERROR);
}

TEST(EncodeDecode, EncodeInvalidBundleIdReturnsError)
{
  uint8_t raw[BUFFER_SIZE];
  proton_buffer_t buf = make_buffer(raw, sizeof(raw));
  size_t bytes_encoded = 0;

  EXPECT_EQ(proton_encode_bundle(0xDEAD, buf, &bytes_encoded), PROTON_ERROR);
}

// -----------------------------------------------------------------------
// Decode
// -----------------------------------------------------------------------

TEST(EncodeDecode, DecodeNullBufferReturnsError)
{
  proton_buffer_t buf;
  buf.data = nullptr;
  buf.len = BUFFER_SIZE;

  EXPECT_EQ(proton_decode_bundle(buf), PROTON_NULL_PTR_ERROR);
}

TEST(EncodeDecode, DecodeGarbageReturnsError)
{
  uint8_t raw[BUFFER_SIZE];
  memset(raw, 0xFF, sizeof(raw));
  proton_buffer_t buf = make_buffer(raw, sizeof(raw));

  EXPECT_EQ(proton_decode_bundle(buf), PROTON_SERIALIZATION_ERROR);
}

// -----------------------------------------------------------------------
// Round-trip
// -----------------------------------------------------------------------

TEST(EncodeDecode, RoundTripDefaultValues)
{
  uint8_t raw[BUFFER_SIZE];
  proton_buffer_t buf = make_buffer(raw, sizeof(raw));
  size_t bytes_encoded = 0;

  // Encode using values already in registry
  ASSERT_EQ(
    proton_encode_bundle(PROTON_BUNDLE_DEFAULT_VALUE_TEST_ID, buf, &bytes_encoded), PROTON_OK);
  ASSERT_GT(bytes_encoded, 0);

  buf.len = bytes_encoded;
  ASSERT_EQ(proton_decode_bundle(buf), PROTON_OK);

  // Verify registry values stayed default
  double decoded_double = 0.0;
  size_t len = sizeof(decoded_double);
  proton_signal_type_e type;
  EXPECT_TRUE(
    proton_signal_get_value(PROTON_SIGNAL_DEFAULT_DOUBLE_ID, &decoded_double, &len, &type));
  EXPECT_DOUBLE_EQ(decoded_double, 3.14159);
  EXPECT_EQ(type, PROTON_DOUBLE);

  char decoded_string[16] = {'\0'};
  len = sizeof(decoded_string);
  EXPECT_TRUE(
    proton_signal_get_value(PROTON_SIGNAL_DEFAULT_STRING_ID, decoded_string, &len, &type));
  EXPECT_STREQ(decoded_string, "foo");
  EXPECT_EQ(type, PROTON_STRING);

  uint8_t decoded_bytes[3] = {};
  len = sizeof(decoded_bytes);
  EXPECT_TRUE(proton_signal_get_value(PROTON_SIGNAL_DEFAULT_BYTES_ID, decoded_bytes, &len, &type));
  EXPECT_EQ(decoded_bytes[0], 0);
  EXPECT_EQ(decoded_bytes[1], 1);
  EXPECT_EQ(decoded_bytes[2], 2);
  EXPECT_EQ(type, PROTON_BYTES);
}

TEST(EncodeDecode, RoundTripMutatedDoubleValue)
{
  // Mutate the double signal
  double new_value = 2.71828;
  ASSERT_TRUE(
    proton_signal_set_value(PROTON_SIGNAL_DEFAULT_DOUBLE_ID, &new_value, sizeof(new_value)));

  uint8_t raw[BUFFER_SIZE];
  proton_buffer_t buf = make_buffer(raw, sizeof(raw));
  size_t bytes_encoded = 0;

  ASSERT_EQ(
    proton_encode_bundle(PROTON_BUNDLE_DEFAULT_VALUE_TEST_ID, buf, &bytes_encoded), PROTON_OK);
  ASSERT_GT(bytes_encoded, 0u);

  // Zero out the signal in the registry before decoding to confirm decode actually writes it
  double zero = 0.0;
  ASSERT_TRUE(proton_signal_set_value(PROTON_SIGNAL_DEFAULT_DOUBLE_ID, &zero, sizeof(zero)));

  buf.len = bytes_encoded;
  ASSERT_EQ(proton_decode_bundle(buf), PROTON_OK);

  double decoded = 0.0;
  size_t len = sizeof(decoded);
  proton_signal_type_e type;
  EXPECT_TRUE(proton_signal_get_value(PROTON_SIGNAL_DEFAULT_DOUBLE_ID, &decoded, &len, &type));
  EXPECT_DOUBLE_EQ(decoded, new_value);
}

int main(int argc, char ** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
