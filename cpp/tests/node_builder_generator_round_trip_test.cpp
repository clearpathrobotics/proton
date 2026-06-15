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
#include <cstring>
#include <limits>
#include <string>

#include "proton/encode_decode.h"
#include "proton/registry.h"
#include "protoncpp/node_builder/generator.hpp"

using namespace proton::node_builder;

static constexpr size_t BUFFER_SIZE = 1024;

// Signal IDs used in tests
static constexpr uint32_t SIG_DOUBLE_ID = 100;
static constexpr uint32_t SIG_FLOAT_ID = 101;
static constexpr uint32_t SIG_INT32_ID = 102;
static constexpr uint32_t SIG_INT64_ID = 103;
static constexpr uint32_t SIG_UINT32_ID = 104;
static constexpr uint32_t SIG_UINT64_ID = 105;
static constexpr uint32_t SIG_BOOL_ID = 106;
static constexpr uint32_t SIG_STRING_ID = 107;
static constexpr uint32_t SIG_BYTES_ID = 108;

static constexpr uint32_t BUNDLE_ALL_TYPES_ID = 10;
static constexpr uint32_t BUNDLE_NUMERIC_ID = 11;
static constexpr uint32_t BUNDLE_STRING_BYTES_ID = 12;

static constexpr size_t STRING_CAPACITY = 64;
static constexpr size_t BYTES_CAPACITY = 32;

// Helper to create a config with all signal types for round-trip testing
Config create_round_trip_config()
{
  Config config;

  // Add two nodes
  NodeConfig node_a;
  node_a.name = "node_a";
  node_a.id = 1;
  node_a.endpoints[0] = EndpointConfig{0, "udp4", "", "192.168.1.1", 5000};

  NodeConfig node_b;
  node_b.name = "node_b";
  node_b.id = 2;
  node_b.endpoints[0] = EndpointConfig{0, "udp4", "", "192.168.1.2", 5000};

  config.nodes["node_a"] = node_a;
  config.nodes["node_b"] = node_b;

  // Add connection
  ConnectionConfig conn_ab;
  conn_ab.first = {0, "node_a"};
  conn_ab.second = {0, "node_b"};
  config.connections.push_back(conn_ab);

  // Add signals of all types
  config.signals.push_back(SignalConfig("double_signal", SIG_DOUBLE_ID, "double"));
  config.signals.push_back(SignalConfig("float_signal", SIG_FLOAT_ID, "float"));
  config.signals.push_back(SignalConfig("int32_signal", SIG_INT32_ID, "int32"));
  config.signals.push_back(SignalConfig("int64_signal", SIG_INT64_ID, "int64"));
  config.signals.push_back(SignalConfig("uint32_signal", SIG_UINT32_ID, "uint32"));
  config.signals.push_back(SignalConfig("uint64_signal", SIG_UINT64_ID, "uint64"));
  config.signals.push_back(SignalConfig("bool_signal", SIG_BOOL_ID, "bool"));
  config.signals.push_back(SignalConfig("string_signal", SIG_STRING_ID, "string", STRING_CAPACITY));
  config.signals.push_back(SignalConfig("bytes_signal", SIG_BYTES_ID, "bytes", BYTES_CAPACITY));

  // Bundle with all signal types
  BundleConfig bundle_all;
  bundle_all.name = "bundle_all_types";
  bundle_all.id = BUNDLE_ALL_TYPES_ID;
  bundle_all.period_ms = 100;
  bundle_all.producers = {"node_a"};
  bundle_all.consumers = {"node_b"};
  bundle_all.signals = {SIG_DOUBLE_ID, SIG_FLOAT_ID, SIG_INT32_ID,  SIG_INT64_ID, SIG_UINT32_ID,
                        SIG_UINT64_ID, SIG_BOOL_ID,  SIG_STRING_ID, SIG_BYTES_ID};
  config.bundles.push_back(bundle_all);

  // Bundle with only numeric types
  BundleConfig bundle_numeric;
  bundle_numeric.name = "bundle_numeric";
  bundle_numeric.id = BUNDLE_NUMERIC_ID;
  bundle_numeric.period_ms = 50;
  bundle_numeric.producers = {"node_a"};
  bundle_numeric.consumers = {"node_b"};
  bundle_numeric.signals = {SIG_DOUBLE_ID, SIG_FLOAT_ID,  SIG_INT32_ID,
                            SIG_INT64_ID,  SIG_UINT32_ID, SIG_UINT64_ID};
  config.bundles.push_back(bundle_numeric);

  // Bundle with string and bytes only
  BundleConfig bundle_string_bytes;
  bundle_string_bytes.name = "bundle_string_bytes";
  bundle_string_bytes.id = BUNDLE_STRING_BYTES_ID;
  bundle_string_bytes.period_ms = 200;
  bundle_string_bytes.producers = {"node_a"};
  bundle_string_bytes.consumers = {"node_b"};
  bundle_string_bytes.signals = {SIG_STRING_ID, SIG_BYTES_ID};
  config.bundles.push_back(bundle_string_bytes);

  return config;
}

// ============================================================================
// GeneratedNode Round-Trip Tests
// ============================================================================

class GeneratedNodeRoundTripTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    config_ = create_round_trip_config();
    generated_node_ = std::make_unique<GeneratedNode>(config_, "node_a");
  }

  void TearDown() override { generated_node_.reset(); }

  Config config_;
  std::unique_ptr<GeneratedNode> generated_node_;
};

TEST_F(GeneratedNodeRoundTripTest, NodeAndRegistryAccessorsReturnValidPointers)
{
  ASSERT_NE(generated_node_->node(), nullptr);
  ASSERT_NE(generated_node_->registry(), nullptr);
  EXPECT_EQ(generated_node_->node()->id, 1);
  EXPECT_EQ(generated_node_->node()->registry, generated_node_->registry());
}

TEST_F(GeneratedNodeRoundTripTest, RegistryContainsExpectedSignals)
{
  proton_registry_t * registry = generated_node_->registry();

  // Verify all signals are present
  EXPECT_NE(proton_registry_get_signal(registry, SIG_DOUBLE_ID, nullptr), nullptr);
  EXPECT_NE(proton_registry_get_signal(registry, SIG_FLOAT_ID, nullptr), nullptr);
  EXPECT_NE(proton_registry_get_signal(registry, SIG_INT32_ID, nullptr), nullptr);
  EXPECT_NE(proton_registry_get_signal(registry, SIG_INT64_ID, nullptr), nullptr);
  EXPECT_NE(proton_registry_get_signal(registry, SIG_UINT32_ID, nullptr), nullptr);
  EXPECT_NE(proton_registry_get_signal(registry, SIG_UINT64_ID, nullptr), nullptr);
  EXPECT_NE(proton_registry_get_signal(registry, SIG_BOOL_ID, nullptr), nullptr);
  EXPECT_NE(proton_registry_get_signal(registry, SIG_STRING_ID, nullptr), nullptr);
  EXPECT_NE(proton_registry_get_signal(registry, SIG_BYTES_ID, nullptr), nullptr);
}

TEST_F(GeneratedNodeRoundTripTest, RegistryContainsExpectedBundles)
{
  proton_registry_t * registry = generated_node_->registry();

  EXPECT_NE(proton_registry_get_bundle(registry, BUNDLE_ALL_TYPES_ID, nullptr), nullptr);
  EXPECT_NE(proton_registry_get_bundle(registry, BUNDLE_NUMERIC_ID, nullptr), nullptr);
  EXPECT_NE(proton_registry_get_bundle(registry, BUNDLE_STRING_BYTES_ID, nullptr), nullptr);
}

TEST_F(GeneratedNodeRoundTripTest, RoundTripDouble)
{
  proton_registry_t * registry = generated_node_->registry();
  uint8_t buffer[BUFFER_SIZE];
  size_t bytes_encoded = 0;

  const double test_value = 3.14159265358979;
  ASSERT_EQ(proton_signal_set_double(registry, SIG_DOUBLE_ID, test_value), PROTON_OK);

  ASSERT_EQ(
    proton_encode_bundle(registry, BUNDLE_NUMERIC_ID, buffer, sizeof(buffer), &bytes_encoded),
    PROTON_OK);
  ASSERT_GT(bytes_encoded, 0u);

  // Zero out the signal before decode
  ASSERT_EQ(proton_signal_set_double(registry, SIG_DOUBLE_ID, 0.0), PROTON_OK);

  proton_Proton decoded_msg = proton_Proton_init_zero;
  ASSERT_EQ(proton_decode(registry, buffer, bytes_encoded, &decoded_msg), PROTON_OK);

  double decoded_value = 0.0;
  ASSERT_EQ(proton_signal_get_double(registry, SIG_DOUBLE_ID, &decoded_value), PROTON_OK);
  EXPECT_DOUBLE_EQ(decoded_value, test_value);
}

TEST_F(GeneratedNodeRoundTripTest, RoundTripFloat)
{
  proton_registry_t * registry = generated_node_->registry();
  uint8_t buffer[BUFFER_SIZE];
  size_t bytes_encoded = 0;

  const float test_value = 2.71828f;
  ASSERT_EQ(proton_signal_set_float(registry, SIG_FLOAT_ID, test_value), PROTON_OK);

  ASSERT_EQ(
    proton_encode_bundle(registry, BUNDLE_NUMERIC_ID, buffer, sizeof(buffer), &bytes_encoded),
    PROTON_OK);
  ASSERT_GT(bytes_encoded, 0u);

  ASSERT_EQ(proton_signal_set_float(registry, SIG_FLOAT_ID, 0.0f), PROTON_OK);

  proton_Proton decoded_msg = proton_Proton_init_zero;
  ASSERT_EQ(proton_decode(registry, buffer, bytes_encoded, &decoded_msg), PROTON_OK);

  float decoded_value = 0.0f;
  ASSERT_EQ(proton_signal_get_float(registry, SIG_FLOAT_ID, &decoded_value), PROTON_OK);
  EXPECT_FLOAT_EQ(decoded_value, test_value);
}

TEST_F(GeneratedNodeRoundTripTest, RoundTripInt32)
{
  proton_registry_t * registry = generated_node_->registry();
  uint8_t buffer[BUFFER_SIZE];
  size_t bytes_encoded = 0;

  const int32_t test_value = -2147483647;
  ASSERT_EQ(proton_signal_set_int32(registry, SIG_INT32_ID, test_value), PROTON_OK);

  ASSERT_EQ(
    proton_encode_bundle(registry, BUNDLE_NUMERIC_ID, buffer, sizeof(buffer), &bytes_encoded),
    PROTON_OK);
  ASSERT_GT(bytes_encoded, 0u);

  ASSERT_EQ(proton_signal_set_int32(registry, SIG_INT32_ID, 0), PROTON_OK);

  proton_Proton decoded_msg = proton_Proton_init_zero;
  ASSERT_EQ(proton_decode(registry, buffer, bytes_encoded, &decoded_msg), PROTON_OK);

  int32_t decoded_value = 0;
  ASSERT_EQ(proton_signal_get_int32(registry, SIG_INT32_ID, &decoded_value), PROTON_OK);
  EXPECT_EQ(decoded_value, test_value);
}

TEST_F(GeneratedNodeRoundTripTest, RoundTripInt64)
{
  proton_registry_t * registry = generated_node_->registry();
  uint8_t buffer[BUFFER_SIZE];
  size_t bytes_encoded = 0;

  const int64_t test_value = -9223372036854775807LL;
  ASSERT_EQ(proton_signal_set_int64(registry, SIG_INT64_ID, test_value), PROTON_OK);

  ASSERT_EQ(
    proton_encode_bundle(registry, BUNDLE_NUMERIC_ID, buffer, sizeof(buffer), &bytes_encoded),
    PROTON_OK);
  ASSERT_GT(bytes_encoded, 0u);

  ASSERT_EQ(proton_signal_set_int64(registry, SIG_INT64_ID, 0), PROTON_OK);

  proton_Proton decoded_msg = proton_Proton_init_zero;
  ASSERT_EQ(proton_decode(registry, buffer, bytes_encoded, &decoded_msg), PROTON_OK);

  int64_t decoded_value = 0;
  ASSERT_EQ(proton_signal_get_int64(registry, SIG_INT64_ID, &decoded_value), PROTON_OK);
  EXPECT_EQ(decoded_value, test_value);
}

TEST_F(GeneratedNodeRoundTripTest, RoundTripUint32)
{
  proton_registry_t * registry = generated_node_->registry();
  uint8_t buffer[BUFFER_SIZE];
  size_t bytes_encoded = 0;

  const uint32_t test_value = 0xDEADBEEF;
  ASSERT_EQ(proton_signal_set_uint32(registry, SIG_UINT32_ID, test_value), PROTON_OK);

  ASSERT_EQ(
    proton_encode_bundle(registry, BUNDLE_NUMERIC_ID, buffer, sizeof(buffer), &bytes_encoded),
    PROTON_OK);
  ASSERT_GT(bytes_encoded, 0u);

  ASSERT_EQ(proton_signal_set_uint32(registry, SIG_UINT32_ID, 0), PROTON_OK);

  proton_Proton decoded_msg = proton_Proton_init_zero;
  ASSERT_EQ(proton_decode(registry, buffer, bytes_encoded, &decoded_msg), PROTON_OK);

  uint32_t decoded_value = 0;
  ASSERT_EQ(proton_signal_get_uint32(registry, SIG_UINT32_ID, &decoded_value), PROTON_OK);
  EXPECT_EQ(decoded_value, test_value);
}

TEST_F(GeneratedNodeRoundTripTest, RoundTripUint64)
{
  proton_registry_t * registry = generated_node_->registry();
  uint8_t buffer[BUFFER_SIZE];
  size_t bytes_encoded = 0;

  const uint64_t test_value = 0xDEADBEEFCAFEBABEULL;
  ASSERT_EQ(proton_signal_set_uint64(registry, SIG_UINT64_ID, test_value), PROTON_OK);

  ASSERT_EQ(
    proton_encode_bundle(registry, BUNDLE_NUMERIC_ID, buffer, sizeof(buffer), &bytes_encoded),
    PROTON_OK);
  ASSERT_GT(bytes_encoded, 0u);

  ASSERT_EQ(proton_signal_set_uint64(registry, SIG_UINT64_ID, 0), PROTON_OK);

  proton_Proton decoded_msg = proton_Proton_init_zero;
  ASSERT_EQ(proton_decode(registry, buffer, bytes_encoded, &decoded_msg), PROTON_OK);

  uint64_t decoded_value = 0;
  ASSERT_EQ(proton_signal_get_uint64(registry, SIG_UINT64_ID, &decoded_value), PROTON_OK);
  EXPECT_EQ(decoded_value, test_value);
}

TEST_F(GeneratedNodeRoundTripTest, RoundTripBool)
{
  proton_registry_t * registry = generated_node_->registry();
  uint8_t buffer[BUFFER_SIZE];
  size_t bytes_encoded = 0;

  // Test with true
  ASSERT_EQ(proton_signal_set_bool(registry, SIG_BOOL_ID, true), PROTON_OK);

  ASSERT_EQ(
    proton_encode_bundle(registry, BUNDLE_ALL_TYPES_ID, buffer, sizeof(buffer), &bytes_encoded),
    PROTON_OK);
  ASSERT_GT(bytes_encoded, 0u);

  ASSERT_EQ(proton_signal_set_bool(registry, SIG_BOOL_ID, false), PROTON_OK);

  proton_Proton decoded_msg = proton_Proton_init_zero;
  ASSERT_EQ(proton_decode(registry, buffer, bytes_encoded, &decoded_msg), PROTON_OK);

  bool decoded_value = false;
  ASSERT_EQ(proton_signal_get_bool(registry, SIG_BOOL_ID, &decoded_value), PROTON_OK);
  EXPECT_TRUE(decoded_value);

  // Test with false
  ASSERT_EQ(proton_signal_set_bool(registry, SIG_BOOL_ID, false), PROTON_OK);

  ASSERT_EQ(
    proton_encode_bundle(registry, BUNDLE_ALL_TYPES_ID, buffer, sizeof(buffer), &bytes_encoded),
    PROTON_OK);

  ASSERT_EQ(proton_signal_set_bool(registry, SIG_BOOL_ID, true), PROTON_OK);

  decoded_msg = proton_Proton_init_zero;
  ASSERT_EQ(proton_decode(registry, buffer, bytes_encoded, &decoded_msg), PROTON_OK);

  ASSERT_EQ(proton_signal_get_bool(registry, SIG_BOOL_ID, &decoded_value), PROTON_OK);
  EXPECT_FALSE(decoded_value);
}

TEST_F(GeneratedNodeRoundTripTest, RoundTripString)
{
  proton_registry_t * registry = generated_node_->registry();
  uint8_t buffer[BUFFER_SIZE];
  size_t bytes_encoded = 0;

  const char * test_value = "Hello, Proton!";
  ASSERT_EQ(
    proton_signal_set_string(registry, SIG_STRING_ID, test_value, strlen(test_value) + 1),
    PROTON_OK);

  ASSERT_EQ(
    proton_encode_bundle(registry, BUNDLE_STRING_BYTES_ID, buffer, sizeof(buffer), &bytes_encoded),
    PROTON_OK);
  ASSERT_GT(bytes_encoded, 0u);

  // Clear the string signal
  // TODO this breaks the test later on
  ASSERT_EQ(proton_signal_set_string(registry, SIG_STRING_ID, "", 1), PROTON_OK);

  proton_Proton decoded_msg = proton_Proton_init_zero;
  ASSERT_EQ(proton_decode(registry, buffer, bytes_encoded, &decoded_msg), PROTON_OK);

  char decoded_string[STRING_CAPACITY] = {0};
  size_t decoded_len = 0;
  ASSERT_EQ(
    proton_signal_get_string(
      registry, SIG_STRING_ID, decoded_string, sizeof(decoded_string), &decoded_len),
    PROTON_OK);
  EXPECT_STREQ(decoded_string, test_value);
}

TEST_F(GeneratedNodeRoundTripTest, RoundTripBytes)
{
  proton_registry_t * registry = generated_node_->registry();
  uint8_t buffer[BUFFER_SIZE];
  size_t bytes_encoded = 0;

  const uint8_t test_value[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, 0xBA, 0xBE};
  ASSERT_EQ(
    proton_signal_set_bytes(registry, SIG_BYTES_ID, test_value, sizeof(test_value)), PROTON_OK);

  ASSERT_EQ(
    proton_encode_bundle(registry, BUNDLE_STRING_BYTES_ID, buffer, sizeof(buffer), &bytes_encoded),
    PROTON_OK);
  ASSERT_GT(bytes_encoded, 0u);

  // Clear the bytes signal
  const uint8_t empty_bytes[] = {0};
  ASSERT_EQ(proton_signal_set_bytes(registry, SIG_BYTES_ID, empty_bytes, 0), PROTON_OK);

  proton_Proton decoded_msg = proton_Proton_init_zero;
  ASSERT_EQ(proton_decode(registry, buffer, bytes_encoded, &decoded_msg), PROTON_OK);

  uint8_t decoded_bytes[BYTES_CAPACITY] = {0};
  size_t decoded_len = 0;
  ASSERT_EQ(
    proton_signal_get_bytes(
      registry, SIG_BYTES_ID, decoded_bytes, sizeof(decoded_bytes), &decoded_len),
    PROTON_OK);
  EXPECT_EQ(decoded_len, sizeof(test_value));
  EXPECT_EQ(memcmp(decoded_bytes, test_value, sizeof(test_value)), 0);
}

TEST_F(GeneratedNodeRoundTripTest, RoundTripAllTypesBundle)
{
  proton_registry_t * registry = generated_node_->registry();
  uint8_t buffer[BUFFER_SIZE];
  size_t bytes_encoded = 0;

  // Set all signal values
  const double double_val = 1.23456789;
  const float float_val = 9.87654f;
  const int32_t int32_val = -42;
  const int64_t int64_val = -1234567890123LL;
  const uint32_t uint32_val = 0xABCD1234;
  const uint64_t uint64_val = 0x123456789ABCDEFULL;
  const bool bool_val = true;
  const char * string_val = "test_string";
  const uint8_t bytes_val[] = {1, 2, 3, 4, 5};

  ASSERT_EQ(proton_signal_set_double(registry, SIG_DOUBLE_ID, double_val), PROTON_OK);
  ASSERT_EQ(proton_signal_set_float(registry, SIG_FLOAT_ID, float_val), PROTON_OK);
  ASSERT_EQ(proton_signal_set_int32(registry, SIG_INT32_ID, int32_val), PROTON_OK);
  ASSERT_EQ(proton_signal_set_int64(registry, SIG_INT64_ID, int64_val), PROTON_OK);
  ASSERT_EQ(proton_signal_set_uint32(registry, SIG_UINT32_ID, uint32_val), PROTON_OK);
  ASSERT_EQ(proton_signal_set_uint64(registry, SIG_UINT64_ID, uint64_val), PROTON_OK);
  ASSERT_EQ(proton_signal_set_bool(registry, SIG_BOOL_ID, bool_val), PROTON_OK);
  ASSERT_EQ(
    proton_signal_set_string(registry, SIG_STRING_ID, string_val, strlen(string_val) + 1),
    PROTON_OK);
  ASSERT_EQ(
    proton_signal_set_bytes(registry, SIG_BYTES_ID, bytes_val, sizeof(bytes_val)), PROTON_OK);

  // Encode bundle with all types
  ASSERT_EQ(
    proton_encode_bundle(registry, BUNDLE_ALL_TYPES_ID, buffer, sizeof(buffer), &bytes_encoded),
    PROTON_OK);
  ASSERT_GT(bytes_encoded, 0u);

  // Clear all signals
  ASSERT_EQ(proton_signal_set_double(registry, SIG_DOUBLE_ID, 0.0), PROTON_OK);
  ASSERT_EQ(proton_signal_set_float(registry, SIG_FLOAT_ID, 0.0f), PROTON_OK);
  ASSERT_EQ(proton_signal_set_int32(registry, SIG_INT32_ID, 0), PROTON_OK);
  ASSERT_EQ(proton_signal_set_int64(registry, SIG_INT64_ID, 0), PROTON_OK);
  ASSERT_EQ(proton_signal_set_uint32(registry, SIG_UINT32_ID, 0), PROTON_OK);
  ASSERT_EQ(proton_signal_set_uint64(registry, SIG_UINT64_ID, 0), PROTON_OK);
  ASSERT_EQ(proton_signal_set_bool(registry, SIG_BOOL_ID, false), PROTON_OK);
  ASSERT_EQ(proton_signal_set_string(registry, SIG_STRING_ID, "", 1), PROTON_OK);
  const uint8_t empty_bytes[] = {0};
  ASSERT_EQ(proton_signal_set_bytes(registry, SIG_BYTES_ID, empty_bytes, 0), PROTON_OK);

  // Decode
  proton_Proton decoded_msg = proton_Proton_init_zero;
  ASSERT_EQ(proton_decode(registry, buffer, bytes_encoded, &decoded_msg), PROTON_OK);

  // Verify all values
  double decoded_double = 0.0;
  ASSERT_EQ(proton_signal_get_double(registry, SIG_DOUBLE_ID, &decoded_double), PROTON_OK);
  EXPECT_DOUBLE_EQ(decoded_double, double_val);

  float decoded_float = 0.0f;
  ASSERT_EQ(proton_signal_get_float(registry, SIG_FLOAT_ID, &decoded_float), PROTON_OK);
  EXPECT_FLOAT_EQ(decoded_float, float_val);

  int32_t decoded_int32 = 0;
  ASSERT_EQ(proton_signal_get_int32(registry, SIG_INT32_ID, &decoded_int32), PROTON_OK);
  EXPECT_EQ(decoded_int32, int32_val);

  int64_t decoded_int64 = 0;
  ASSERT_EQ(proton_signal_get_int64(registry, SIG_INT64_ID, &decoded_int64), PROTON_OK);
  EXPECT_EQ(decoded_int64, int64_val);

  uint32_t decoded_uint32 = 0;
  ASSERT_EQ(proton_signal_get_uint32(registry, SIG_UINT32_ID, &decoded_uint32), PROTON_OK);
  EXPECT_EQ(decoded_uint32, uint32_val);

  uint64_t decoded_uint64 = 0;
  ASSERT_EQ(proton_signal_get_uint64(registry, SIG_UINT64_ID, &decoded_uint64), PROTON_OK);
  EXPECT_EQ(decoded_uint64, uint64_val);

  bool decoded_bool = false;
  ASSERT_EQ(proton_signal_get_bool(registry, SIG_BOOL_ID, &decoded_bool), PROTON_OK);
  EXPECT_EQ(decoded_bool, bool_val);

  char decoded_string[STRING_CAPACITY] = {0};
  size_t string_len = 0;
  ASSERT_EQ(
    proton_signal_get_string(
      registry, SIG_STRING_ID, decoded_string, sizeof(decoded_string), &string_len),
    PROTON_OK);
  EXPECT_STREQ(decoded_string, string_val);

  uint8_t decoded_bytes[BYTES_CAPACITY] = {0};
  size_t bytes_len = 0;
  ASSERT_EQ(
    proton_signal_get_bytes(
      registry, SIG_BYTES_ID, decoded_bytes, sizeof(decoded_bytes), &bytes_len),
    PROTON_OK);
  EXPECT_EQ(bytes_len, sizeof(bytes_val));
  EXPECT_EQ(memcmp(decoded_bytes, bytes_val, sizeof(bytes_val)), 0);
}

TEST_F(GeneratedNodeRoundTripTest, RoundTripBoundaryValues)
{
  proton_registry_t * registry = generated_node_->registry();
  uint8_t buffer[BUFFER_SIZE];
  size_t bytes_encoded = 0;

  // Test boundary values for numeric types
  ASSERT_EQ(
    proton_signal_set_int32(registry, SIG_INT32_ID, std::numeric_limits<int32_t>::min()),
    PROTON_OK);
  ASSERT_EQ(
    proton_signal_set_int64(registry, SIG_INT64_ID, std::numeric_limits<int64_t>::min()),
    PROTON_OK);
  ASSERT_EQ(
    proton_signal_set_uint32(registry, SIG_UINT32_ID, std::numeric_limits<uint32_t>::max()),
    PROTON_OK);
  ASSERT_EQ(
    proton_signal_set_uint64(registry, SIG_UINT64_ID, std::numeric_limits<uint64_t>::max()),
    PROTON_OK);

  ASSERT_EQ(
    proton_encode_bundle(registry, BUNDLE_NUMERIC_ID, buffer, sizeof(buffer), &bytes_encoded),
    PROTON_OK);

  // Reset values
  ASSERT_EQ(proton_signal_set_int32(registry, SIG_INT32_ID, 0), PROTON_OK);
  ASSERT_EQ(proton_signal_set_int64(registry, SIG_INT64_ID, 0), PROTON_OK);
  ASSERT_EQ(proton_signal_set_uint32(registry, SIG_UINT32_ID, 0), PROTON_OK);
  ASSERT_EQ(proton_signal_set_uint64(registry, SIG_UINT64_ID, 0), PROTON_OK);

  proton_Proton decoded_msg = proton_Proton_init_zero;
  ASSERT_EQ(proton_decode(registry, buffer, bytes_encoded, &decoded_msg), PROTON_OK);

  int32_t decoded_int32 = 0;
  ASSERT_EQ(proton_signal_get_int32(registry, SIG_INT32_ID, &decoded_int32), PROTON_OK);
  EXPECT_EQ(decoded_int32, std::numeric_limits<int32_t>::min());

  int64_t decoded_int64 = 0;
  ASSERT_EQ(proton_signal_get_int64(registry, SIG_INT64_ID, &decoded_int64), PROTON_OK);
  EXPECT_EQ(decoded_int64, std::numeric_limits<int64_t>::min());

  uint32_t decoded_uint32 = 0;
  ASSERT_EQ(proton_signal_get_uint32(registry, SIG_UINT32_ID, &decoded_uint32), PROTON_OK);
  EXPECT_EQ(decoded_uint32, std::numeric_limits<uint32_t>::max());

  uint64_t decoded_uint64 = 0;
  ASSERT_EQ(proton_signal_get_uint64(registry, SIG_UINT64_ID, &decoded_uint64), PROTON_OK);
  EXPECT_EQ(decoded_uint64, std::numeric_limits<uint64_t>::max());
}

TEST_F(GeneratedNodeRoundTripTest, RoundTripMultipleEncodesDecodes)
{
  proton_registry_t * registry = generated_node_->registry();
  uint8_t buffer1[BUFFER_SIZE];
  uint8_t buffer2[BUFFER_SIZE];
  size_t bytes_encoded1 = 0;
  size_t bytes_encoded2 = 0;

  // First encode with value 1
  const int32_t value1 = 100;
  ASSERT_EQ(proton_signal_set_int32(registry, SIG_INT32_ID, value1), PROTON_OK);
  ASSERT_EQ(
    proton_encode_bundle(registry, BUNDLE_NUMERIC_ID, buffer1, sizeof(buffer1), &bytes_encoded1),
    PROTON_OK);

  // Second encode with value 2
  const int32_t value2 = 200;
  ASSERT_EQ(proton_signal_set_int32(registry, SIG_INT32_ID, value2), PROTON_OK);
  ASSERT_EQ(
    proton_encode_bundle(registry, BUNDLE_NUMERIC_ID, buffer2, sizeof(buffer2), &bytes_encoded2),
    PROTON_OK);

  // Decode buffer1 - should restore value1
  proton_Proton decoded_msg = proton_Proton_init_zero;
  ASSERT_EQ(proton_decode(registry, buffer1, bytes_encoded1, &decoded_msg), PROTON_OK);

  int32_t decoded_value = 0;
  ASSERT_EQ(proton_signal_get_int32(registry, SIG_INT32_ID, &decoded_value), PROTON_OK);
  EXPECT_EQ(decoded_value, value1);

  // Decode buffer2 - should restore value2
  decoded_msg = proton_Proton_init_zero;
  ASSERT_EQ(proton_decode(registry, buffer2, bytes_encoded2, &decoded_msg), PROTON_OK);

  ASSERT_EQ(proton_signal_get_int32(registry, SIG_INT32_ID, &decoded_value), PROTON_OK);
  EXPECT_EQ(decoded_value, value2);
}

TEST_F(GeneratedNodeRoundTripTest, RoundTripEmptyString)
{
  proton_registry_t * registry = generated_node_->registry();
  uint8_t buffer[BUFFER_SIZE];
  size_t bytes_encoded = 0;

  // Set empty string (just null terminator)
  ASSERT_EQ(proton_signal_set_string(registry, SIG_STRING_ID, "", 1), PROTON_OK);

  ASSERT_EQ(
    proton_encode_bundle(registry, BUNDLE_STRING_BYTES_ID, buffer, sizeof(buffer), &bytes_encoded),
    PROTON_OK);
  ASSERT_GT(bytes_encoded, 0u);

  // Set a non-empty value before decode
  ASSERT_EQ(proton_signal_set_string(registry, SIG_STRING_ID, "not_empty", 10), PROTON_OK);

  proton_Proton decoded_msg = proton_Proton_init_zero;
  ASSERT_EQ(proton_decode(registry, buffer, bytes_encoded, &decoded_msg), PROTON_OK);

  char decoded_string[STRING_CAPACITY] = {0};
  size_t decoded_len = 0;
  ASSERT_EQ(
    proton_signal_get_string(
      registry, SIG_STRING_ID, decoded_string, sizeof(decoded_string), &decoded_len),
    PROTON_OK);
  EXPECT_STREQ(decoded_string, "");
}

TEST_F(GeneratedNodeRoundTripTest, MoveConstructorPreservesState)
{
  proton_registry_t * registry = generated_node_->registry();

  // Set a value
  const double test_value = 42.0;
  ASSERT_EQ(proton_signal_set_double(registry, SIG_DOUBLE_ID, test_value), PROTON_OK);

  // Move the node
  GeneratedNode moved_node = std::move(*generated_node_);

  // Verify the moved node still has valid state
  proton_registry_t * moved_registry = moved_node.registry();
  ASSERT_NE(moved_registry, nullptr);

  double decoded_value = 0.0;
  ASSERT_EQ(proton_signal_get_double(moved_registry, SIG_DOUBLE_ID, &decoded_value), PROTON_OK);
  EXPECT_DOUBLE_EQ(decoded_value, test_value);
}

int main(int argc, char ** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
