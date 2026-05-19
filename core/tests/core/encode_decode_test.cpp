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
#include "target_registry_ids.h"
#include "target_registry_sizes.h"
#include "utils.hpp"

static constexpr size_t BUFFER_SIZE = 1024;

extern proton_registry_t g_proton_registry;

// -----------------------------------------------------------------------
// Encode
// -----------------------------------------------------------------------

TEST(EncodeDecode, EncodeDefaultValueTestBundle)
{
  uint8_t raw[BUFFER_SIZE];
  size_t bytes_encoded = 0;

  proton_registry_t registry = copy_default_registry(&g_proton_registry);

  proton_status_e status = proton_encode_bundle(
    &registry, PROTON_BUNDLE_DEFAULT_VALUE_TEST_ID, raw, sizeof(raw), &bytes_encoded);

  EXPECT_EQ(status, PROTON_OK);
  EXPECT_GT(bytes_encoded, 0);
  free(registry.signal_registry);
}

TEST(EncodeDecode, EncodeNullBufferReturnsError)
{
  uint8_t * buf = nullptr;
  size_t bytes_encoded = 0;

  proton_registry_t registry = copy_default_registry(&g_proton_registry);

  EXPECT_EQ(
    proton_encode_bundle(
      &registry, PROTON_BUNDLE_DEFAULT_VALUE_TEST_ID, buf, BUFFER_SIZE, &bytes_encoded),
    PROTON_NULL_PTR_ERROR);
  free(registry.signal_registry);
}

TEST(EncodeDecode, EncodeNullBytesEncodedReturnsError)
{
  uint8_t raw[BUFFER_SIZE];

  proton_registry_t registry = copy_default_registry(&g_proton_registry);

  EXPECT_EQ(
    proton_encode_bundle(&registry, PROTON_BUNDLE_DEFAULT_VALUE_TEST_ID, raw, BUFFER_SIZE, nullptr),
    PROTON_NULL_PTR_ERROR);
  free(registry.signal_registry);
}

TEST(EncodeDecode, EncodeInvalidBundleIdReturnsError)
{
  uint8_t raw[BUFFER_SIZE];
  size_t bytes_encoded = 0;

  proton_registry_t registry = copy_default_registry(&g_proton_registry);

  EXPECT_EQ(
    proton_encode_bundle(&registry, 0xDEAD, raw, BUFFER_SIZE, &bytes_encoded), PROTON_ERROR);
  free(registry.signal_registry);
}

// -----------------------------------------------------------------------
// Decode
// -----------------------------------------------------------------------

TEST(EncodeDecode, DecodeNullBufferReturnsError)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  EXPECT_EQ(proton_decode(&registry, nullptr, BUFFER_SIZE), PROTON_NULL_PTR_ERROR);
  free(registry.signal_registry);
}

TEST(EncodeDecode, DecodeGarbageReturnsError)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  uint8_t raw[BUFFER_SIZE];
  memset(raw, 0xFF, sizeof(raw));

  EXPECT_EQ(proton_decode(&registry, raw, sizeof(raw)), PROTON_SERIALIZATION_ERROR);
  free(registry.signal_registry);
}

// -----------------------------------------------------------------------
// Round-trip
// -----------------------------------------------------------------------

TEST(EncodeDecode, RoundTripDefaultValues)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  uint8_t raw[BUFFER_SIZE];
  size_t bytes_encoded = 0;

  // Encode using values already in registry
  ASSERT_EQ(
    proton_encode_bundle(
      &registry, PROTON_BUNDLE_DEFAULT_VALUE_TEST_ID, raw, sizeof(raw), &bytes_encoded),
    PROTON_OK);
  ASSERT_GT(bytes_encoded, 0);

  ASSERT_EQ(proton_decode(&registry, raw, bytes_encoded), PROTON_OK);

  // Verify registry values stayed default
  double decoded_double = 0.0;
  size_t len = sizeof(decoded_double);
  proton_status_e status =
    proton_signal_get_double(&registry, PROTON_SIGNAL_DEFAULT_DOUBLE_ID, &decoded_double);
  ASSERT_EQ(status, PROTON_OK);
  EXPECT_DOUBLE_EQ(decoded_double, 3.14159);

  char decoded_string[16] = {'\0'};
  len = sizeof(decoded_string);
  status = proton_signal_get_string(
    &registry, PROTON_SIGNAL_DEFAULT_STRING_ID, decoded_string, sizeof(decoded_string), &len);
  ASSERT_EQ(status, PROTON_OK);
  EXPECT_STREQ(decoded_string, "foo");

  uint8_t decoded_bytes[3] = {};
  len = sizeof(decoded_bytes);
  status = proton_signal_get_bytes(
    &registry, PROTON_SIGNAL_DEFAULT_BYTES_ID, decoded_bytes, sizeof(decoded_bytes), &len);
  ASSERT_EQ(status, PROTON_OK);
  EXPECT_EQ(decoded_bytes[0], 0);
  EXPECT_EQ(decoded_bytes[1], 1);
  EXPECT_EQ(decoded_bytes[2], 2);
  free(registry.signal_registry);
}

TEST(EncodeDecode, RoundTripMutatedDoubleValue)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);

  // Mutate the double signal
  double new_value = 2.71828;
  proton_status_e status =
    proton_signal_set_double(&registry, PROTON_SIGNAL_DEFAULT_DOUBLE_ID, new_value);
  ASSERT_EQ(status, PROTON_OK);

  uint8_t raw[BUFFER_SIZE];
  size_t bytes_encoded = 0;

  ASSERT_EQ(
    proton_encode_bundle(
      &registry, PROTON_BUNDLE_DEFAULT_VALUE_TEST_ID, raw, BUFFER_SIZE, &bytes_encoded),
    PROTON_OK);
  ASSERT_GT(bytes_encoded, 0u);

  // Zero out the signal in the registry before decoding to confirm decode actually writes it
  double zero = 0.0;
  status = proton_signal_set_double(&registry, PROTON_SIGNAL_DEFAULT_DOUBLE_ID, zero);
  ASSERT_EQ(status, PROTON_OK);

  ASSERT_EQ(proton_decode(&registry, raw, bytes_encoded), PROTON_OK);

  double decoded = 0.0;
  size_t len = sizeof(decoded);
  proton_signal_type_e type;
  status = proton_signal_get_double(&registry, PROTON_SIGNAL_DEFAULT_DOUBLE_ID, &decoded);
  ASSERT_EQ(status, PROTON_OK);
  EXPECT_DOUBLE_EQ(decoded, new_value);
  free(registry.signal_registry);
}

TEST(EncodeDecode, SignalSharedBetweenBundles)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);

  // Mutate the shared signal
  int32_t first_value = 42;
  int32_t second_value = 84;
  proton_status_e status =
    proton_signal_set_int32(&registry, PROTON_SIGNAL_SHARED_SIGNAL_ID, first_value);
  ASSERT_EQ(status, PROTON_OK);

  uint8_t raw[BUFFER_SIZE];
  size_t bytes_encoded = 0;

  // Encode the first bundle that contains the shared signal
  ASSERT_EQ(
    proton_encode_bundle(&registry, PROTON_BUNDLE_SHARED_1_ID, raw, BUFFER_SIZE, &bytes_encoded),
    PROTON_OK);
  ASSERT_GT(bytes_encoded, 0u);

  // Mutate the shared signal again, so that encoding a second bundle with the same signal will have a different value
  status = proton_signal_set_int32(&registry, PROTON_SIGNAL_SHARED_SIGNAL_ID, second_value);
  ASSERT_EQ(status, PROTON_OK);

  // Encode a second bundle that contains the second value of the shared signal
  uint8_t raw2[BUFFER_SIZE];
  size_t bytes_encoded2 = 0;
  ASSERT_EQ(
    proton_encode_bundle(&registry, PROTON_BUNDLE_SHARED_2_ID, raw2, BUFFER_SIZE, &bytes_encoded2),
    PROTON_OK);
  ASSERT_GT(bytes_encoded2, 0u);

  // Decode the first bundle and verify that the shared signal is the first value
  ASSERT_EQ(proton_decode(&registry, raw, bytes_encoded), PROTON_OK);

  int32_t decoded = 0;
  size_t len = sizeof(decoded);
  status = proton_signal_get_int32(&registry, PROTON_SIGNAL_SHARED_SIGNAL_ID, &decoded);
  ASSERT_EQ(status, PROTON_OK);
  EXPECT_EQ(decoded, first_value);

  // Decode the second bundle and verify that the shared signal is the second value
  ASSERT_EQ(proton_decode(&registry, raw2, bytes_encoded2), PROTON_OK);

  status = proton_signal_get_int32(&registry, PROTON_SIGNAL_SHARED_SIGNAL_ID, &decoded);
  ASSERT_EQ(status, PROTON_OK);
  EXPECT_EQ(decoded, second_value);

  free(registry.signal_registry);
}

TEST(EncodeDecode, BundleDecodeCallback)
{
  bool cb_called = false;
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  proton_registry_set_bundle_callback(
    &registry, PROTON_BUNDLE_DEFAULT_VALUE_TEST_ID, test_bundle_callback, (void *)&cb_called);

  uint8_t raw[BUFFER_SIZE];
  size_t bytes_encoded = 0;

  ASSERT_EQ(
    proton_encode_bundle(
      &registry, PROTON_BUNDLE_DEFAULT_VALUE_TEST_ID, raw, sizeof(raw), &bytes_encoded),
    PROTON_OK);
  ASSERT_GT(bytes_encoded, 0);

  ASSERT_EQ(proton_decode(&registry, raw, bytes_encoded), PROTON_OK);

  EXPECT_TRUE(cb_called);
}

int main(int argc, char ** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
