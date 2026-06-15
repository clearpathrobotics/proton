/*
 * Copyright SIG_DEFAULT_BOOL_ID6 Rockwell Automation Technologies, Inc., All rights reserved.
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

// Signal IDs used for signals with default values
static constexpr uint32_t SIG_DEFAULT_DOUBLE_ID = 200;
static constexpr uint32_t SIG_DEFAULT_FLOAT_ID = 201;
static constexpr uint32_t SIG_DEFAULT_INT32_ID = 202;
static constexpr uint32_t SIG_DEFAULT_INT64_ID = 203;
static constexpr uint32_t SIG_DEFAULT_UINT32_ID = 204;
static constexpr uint32_t SIG_DEFAULT_UINT64_ID = 205;
static constexpr uint32_t SIG_DEFAULT_BOOL_ID = 206;
static constexpr uint32_t SIG_DEFAULT_STRING_ID = 207;
static constexpr uint32_t SIG_DEFAULT_BYTES_ID = 208;

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

// ============================================================================
// Default Value Tests
// ============================================================================

Config create_default_values_config()
{
  Config config;

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

  ConnectionConfig conn_ab;
  conn_ab.first = {0, "node_a"};
  conn_ab.second = {0, "node_b"};
  config.connections.push_back(conn_ab);

  // Signal with default double value
  SignalConfig sig_double("default_double", SIG_DEFAULT_DOUBLE_ID, "double");
  sig_double.has_default_value = true;
  sig_double.value = 3.14159;
  config.signals.push_back(sig_double);

  // Signal with default float value
  SignalConfig sig_float("default_float", SIG_DEFAULT_FLOAT_ID, "float");
  sig_float.has_default_value = true;
  sig_float.value = 1.141;
  config.signals.push_back(sig_float);

  // Signal with default int32 value
  SignalConfig sig_int32("default_int32", SIG_DEFAULT_INT32_ID, "int32");
  sig_int32.has_default_value = true;
  sig_int32.value = int32_t{-42};
  config.signals.push_back(sig_int32);

  // Signal with default int64 value
  SignalConfig sig_int64("default_int64", SIG_DEFAULT_INT64_ID, "int64");
  sig_int64.has_default_value = true;
  sig_int64.value = int64_t{-64};
  config.signals.push_back(sig_int64);

  // Signal with default uint32 value
  SignalConfig sig_uint32("default_uint32", SIG_DEFAULT_UINT32_ID, "uint32");
  sig_uint32.has_default_value = true;
  sig_uint32.value = uint32_t{42};
  config.signals.push_back(sig_uint32);

  // Signal with default bool value
  SignalConfig sig_bool("default_bool", SIG_DEFAULT_BOOL_ID, "bool");
  sig_bool.has_default_value = true;
  sig_bool.value = true;
  config.signals.push_back(sig_bool);

  // Signal with default uint64 value
  SignalConfig sig_uint64("default_uint64", SIG_DEFAULT_UINT64_ID, "uint64");
  sig_uint64.has_default_value = true;
  sig_uint64.value = uint64_t{0xDEADBEEFCAFEBABEULL};
  config.signals.push_back(sig_uint64);

  // Signal with default string value
  SignalConfig sig_string("default_string", SIG_DEFAULT_STRING_ID, "string", 64);
  sig_string.has_default_value = true;
  sig_string.value = std::string("hello_default");
  config.signals.push_back(sig_string);

  // Signal with default bytes value
  SignalConfig sig_bytes("default_bytes", SIG_DEFAULT_BYTES_ID, "bytes", 64);
  sig_bytes.has_default_value = true;
  sig_bytes.value = ConfigSequence{ConfigValue(0), ConfigValue(1), ConfigValue(2)};
  config.signals.push_back(sig_bytes);

  BundleConfig bundle;
  bundle.name = "defaults_bundle";
  bundle.id = 20;
  bundle.period_ms = 100;
  bundle.producers = {"node_a"};
  bundle.consumers = {"node_b"};
  bundle.signals = {
    SIG_DEFAULT_DOUBLE_ID, SIG_DEFAULT_FLOAT_ID,  SIG_DEFAULT_INT32_ID,
    SIG_DEFAULT_INT64_ID,  SIG_DEFAULT_UINT32_ID, SIG_DEFAULT_UINT64_ID,
    SIG_DEFAULT_BOOL_ID,   SIG_DEFAULT_STRING_ID, SIG_DEFAULT_BYTES_ID,
  };
  config.bundles.push_back(bundle);

  return config;
}

TEST(DefaultValuesTest, DoubleDefaultValue)
{
  Config config = create_default_values_config();
  GeneratedNode node(config, "node_a");

  double value = 0.0;
  ASSERT_EQ(proton_signal_get_double(node.registry(), SIG_DEFAULT_DOUBLE_ID, &value), PROTON_OK);
  EXPECT_DOUBLE_EQ(value, 3.14159);
}

TEST(DefaultValuesTest, FloatDefaultValue)
{
  Config config = create_default_values_config();
  GeneratedNode node(config, "node_a");

  float value = 0.0;
  ASSERT_EQ(proton_signal_get_float(node.registry(), SIG_DEFAULT_FLOAT_ID, &value), PROTON_OK);
  EXPECT_FLOAT_EQ(value, 1.141);
}

TEST(DefaultValuesTest, Int32DefaultValue)
{
  Config config = create_default_values_config();
  GeneratedNode node(config, "node_a");

  int32_t value = 0;
  ASSERT_EQ(proton_signal_get_int32(node.registry(), SIG_DEFAULT_INT32_ID, &value), PROTON_OK);
  EXPECT_EQ(value, -42);
}

TEST(DefaultValuesTest, Int64DefaultValue)
{
  Config config = create_default_values_config();
  GeneratedNode node(config, "node_a");

  int64_t value = 0;
  ASSERT_EQ(proton_signal_get_int64(node.registry(), SIG_DEFAULT_INT64_ID, &value), PROTON_OK);
  EXPECT_EQ(value, -64);
}

TEST(DefaultValuesTest, UInt32DefaultValue)
{
  Config config = create_default_values_config();
  GeneratedNode node(config, "node_a");

  uint32_t value = 0;
  ASSERT_EQ(proton_signal_get_uint32(node.registry(), SIG_DEFAULT_UINT32_ID, &value), PROTON_OK);
  EXPECT_EQ(value, 42);
}

TEST(DefaultValuesTest, BoolDefaultValue)
{
  Config config = create_default_values_config();
  GeneratedNode node(config, "node_a");

  bool value = false;
  ASSERT_EQ(proton_signal_get_bool(node.registry(), SIG_DEFAULT_BOOL_ID, &value), PROTON_OK);
  EXPECT_TRUE(value);
}

TEST(DefaultValuesTest, Uint64DefaultValue)
{
  Config config = create_default_values_config();
  GeneratedNode node(config, "node_a");

  uint64_t value = 0;
  ASSERT_EQ(proton_signal_get_uint64(node.registry(), SIG_DEFAULT_UINT64_ID, &value), PROTON_OK);
  EXPECT_EQ(value, 0xDEADBEEFCAFEBABEULL);
}

TEST(DefaultValuesTest, StringDefaultValue)
{
  Config config = create_default_values_config();
  GeneratedNode node(config, "node_a");

  char buf[64] = {0};
  size_t len = 0;
  ASSERT_EQ(
    proton_signal_get_string(node.registry(), SIG_DEFAULT_STRING_ID, buf, sizeof(buf), &len),
    PROTON_OK);
  EXPECT_STREQ(buf, "hello_default");
}

TEST(DefaultValuesTest, BytesDefaultValue)
{
  Config config = create_default_values_config();
  GeneratedNode node(config, "node_a");

  uint8_t buf[64] = {0};
  size_t len = 0;
  ASSERT_EQ(
    proton_signal_get_bytes(node.registry(), SIG_DEFAULT_BYTES_ID, buf, sizeof(buf), &len),
    PROTON_OK);
  EXPECT_EQ(len, 3);
  EXPECT_EQ(buf[0], 0);
  EXPECT_EQ(buf[1], 1);
  EXPECT_EQ(buf[2], 2);
}

TEST(DefaultValuesTest, RoundTripPreservesDefaults)
{
  Config config = create_default_values_config();
  GeneratedNode node(config, "node_a");

  uint8_t buffer[BUFFER_SIZE];
  size_t bytes_encoded = 0;

  // Encode with default values
  ASSERT_EQ(
    proton_encode_bundle(node.registry(), 20, buffer, sizeof(buffer), &bytes_encoded), PROTON_OK);
  ASSERT_GT(bytes_encoded, 0u);

  // Create a second node to decode into
  GeneratedNode node2(config, "node_a");

  // Modify values before decode
  ASSERT_EQ(proton_signal_set_double(node2.registry(), SIG_DEFAULT_DOUBLE_ID, 999.0), PROTON_OK);
  ASSERT_EQ(proton_signal_set_int32(node2.registry(), SIG_DEFAULT_INT32_ID, 999), PROTON_OK);

  proton_Proton decoded_msg = proton_Proton_init_zero;
  ASSERT_EQ(proton_decode(node2.registry(), buffer, bytes_encoded, &decoded_msg), PROTON_OK);

  // Verify decoded values match original defaults
  double double_val = 0.0;
  ASSERT_EQ(
    proton_signal_get_double(node2.registry(), SIG_DEFAULT_DOUBLE_ID, &double_val), PROTON_OK);
  EXPECT_DOUBLE_EQ(double_val, 3.14159);

  int32_t int32_val = 0;
  ASSERT_EQ(proton_signal_get_int32(node2.registry(), SIG_DEFAULT_INT32_ID, &int32_val), PROTON_OK);
  EXPECT_EQ(int32_val, -42);
}

// ============================================================================
// Invalid signal type Tests
// ============================================================================

TEST(InvalidSignalType, InvalidSignalTypeThrows)
{
  Config config;

  NodeConfig node_a;
  node_a.name = "node_a";
  node_a.id = 1;
  node_a.endpoints[0] = EndpointConfig{0, "serial", "/dev/ttyUSB0", "", 0};

  NodeConfig node_b;
  node_b.name = "node_b";
  node_b.id = 2;
  node_b.endpoints[0] = EndpointConfig{0, "serial", "/dev/ttyUSB1", "", 0};

  config.nodes["node_a"] = node_a;
  config.nodes["node_b"] = node_b;

  ConnectionConfig conn;
  conn.first = {0, "node_a"};
  conn.second = {0, "node_b"};
  config.connections.push_back(conn);

  SignalConfig sig("test_signal", 100, "invalid_signal_type");
  config.signals.push_back(sig);

  BundleConfig bundle;
  bundle.name = "test_bundle";
  bundle.id = 10;
  bundle.period_ms = 100;
  bundle.producers = {"node_a"};
  bundle.consumers = {"node_b"};
  bundle.signals = {100};
  config.bundles.push_back(bundle);

  EXPECT_THROW(GeneratedNode(config, "node_a"), NodeBuilderException);
}

// ============================================================================
// Invalid node name Tests
// ============================================================================

TEST(InvalidNodeName, InvalidNodeNameThrows)
{
  Config config;

  NodeConfig node_a;
  node_a.name = "node_a";
  node_a.id = 1;
  node_a.endpoints[0] = EndpointConfig{0, "serial", "/dev/ttyUSB0", "", 0};

  NodeConfig node_b;
  node_b.name = "node_b";
  node_b.id = 2;
  node_b.endpoints[0] = EndpointConfig{0, "serial", "/dev/ttyUSB1", "", 0};

  config.nodes["node_a"] = node_a;
  config.nodes["node_b"] = node_b;

  ConnectionConfig conn;
  conn.first = {0, "node_a"};
  conn.second = {0, "node_b"};
  config.connections.push_back(conn);

  SignalConfig sig("test_signal", 100, "invalid_signal_type");
  config.signals.push_back(sig);

  BundleConfig bundle;
  bundle.name = "test_bundle";
  bundle.id = 10;
  bundle.period_ms = 100;
  bundle.producers = {"node_a"};
  bundle.consumers = {"node_b"};
  bundle.signals = {100};
  config.bundles.push_back(bundle);

  EXPECT_THROW(GeneratedNode(config, "node_notanode"), NodeBuilderException);
}

// ============================================================================
// Invalid Transport Tests
// ============================================================================

TEST(InvalidTransportTest, InvalidTransportTypeThrows)
{
  Config config;

  NodeConfig node_a;
  node_a.name = "node_a";
  node_a.id = 1;
  node_a.endpoints[0] = EndpointConfig{0, "invalid_transport", "", "192.168.1.1", 5000};

  NodeConfig node_b;
  node_b.name = "node_b";
  node_b.id = 2;
  node_b.endpoints[0] = EndpointConfig{0, "udp4", "", "192.168.1.2", 5000};

  config.nodes["node_a"] = node_a;
  config.nodes["node_b"] = node_b;

  ConnectionConfig conn;
  conn.first = {0, "node_a"};
  conn.second = {0, "node_b"};
  config.connections.push_back(conn);

  // Should throw when generating node_b (which tries to create endpoint for node_a with invalid transport)
  EXPECT_THROW(GeneratedNode(config, "node_b"), NodeBuilderException);
}

TEST(InvalidTransportTest, SerialTransportIsValid)
{
  Config config;

  NodeConfig node_a;
  node_a.name = "node_a";
  node_a.id = 1;
  node_a.endpoints[0] = EndpointConfig{0, "serial", "/dev/ttyUSB0", "", 0};

  NodeConfig node_b;
  node_b.name = "node_b";
  node_b.id = 2;
  node_b.endpoints[0] = EndpointConfig{0, "serial", "/dev/ttyUSB1", "", 0};

  config.nodes["node_a"] = node_a;
  config.nodes["node_b"] = node_b;

  ConnectionConfig conn;
  conn.first = {0, "node_a"};
  conn.second = {0, "node_b"};
  config.connections.push_back(conn);

  SignalConfig sig("test_signal", 100, "int32");
  config.signals.push_back(sig);

  BundleConfig bundle;
  bundle.name = "test_bundle";
  bundle.id = 10;
  bundle.period_ms = 100;
  bundle.producers = {"node_a"};
  bundle.consumers = {"node_b"};
  bundle.signals = {100};
  config.bundles.push_back(bundle);

  // Should not throw - serial is a valid transport
  EXPECT_NO_THROW(GeneratedNode(config, "node_a"));
}

// ============================================================================
// Node Filtering Tests
// ============================================================================

Config create_multi_node_config()
{
  Config config;

  // Create 4 nodes in a chain: A <-> B <-> C <-> D
  NodeConfig node_a;
  node_a.name = "node_a";
  node_a.id = 1;
  node_a.endpoints[0] = EndpointConfig{0, "serial", "/dev/ttyUSB0", "", 0};

  NodeConfig node_b;
  node_b.name = "node_b";
  node_b.id = 2;
  node_b.endpoints[0] = EndpointConfig{0, "serial", "/dev/ttyUSB1", "", 0};

  NodeConfig node_c;
  node_c.name = "node_c";
  node_c.id = 3;
  node_c.endpoints[0] = EndpointConfig{0, "serial", "/dev/ttyUSB2", "", 0};

  NodeConfig node_d;
  node_d.name = "node_d";
  node_d.id = 4;
  node_d.endpoints[0] = EndpointConfig{0, "serial", "/dev/ttyUSB3", "", 0};

  config.nodes["node_a"] = node_a;
  config.nodes["node_b"] = node_b;
  config.nodes["node_c"] = node_c;
  config.nodes["node_d"] = node_d;

  // Connections: A <-> B, B <-> C, C <-> D
  ConnectionConfig conn_ab;
  conn_ab.first = {0, "node_a"};
  conn_ab.second = {0, "node_b"};
  config.connections.push_back(conn_ab);

  ConnectionConfig conn_bc;
  conn_bc.first = {0, "node_b"};
  conn_bc.second = {0, "node_c"};
  config.connections.push_back(conn_bc);

  ConnectionConfig conn_cd;
  conn_cd.first = {0, "node_c"};
  conn_cd.second = {0, "node_d"};
  config.connections.push_back(conn_cd);

  // Signals
  config.signals.push_back(SignalConfig("signal_ab", 100, "int32"));
  config.signals.push_back(SignalConfig("signal_bc", 101, "int32"));
  config.signals.push_back(SignalConfig("signal_cd", 102, "int32"));

  // Bundles
  BundleConfig bundle_ab;
  bundle_ab.name = "bundle_ab";
  bundle_ab.id = 10;
  bundle_ab.period_ms = 100;
  bundle_ab.producers = {"node_a"};
  bundle_ab.consumers = {"node_b"};
  bundle_ab.signals = {100};
  config.bundles.push_back(bundle_ab);

  BundleConfig bundle_bc;
  bundle_bc.name = "bundle_bc";
  bundle_bc.id = 11;
  bundle_bc.period_ms = 100;
  bundle_bc.producers = {"node_b"};
  bundle_bc.consumers = {"node_c"};
  bundle_bc.signals = {101};
  config.bundles.push_back(bundle_bc);

  BundleConfig bundle_cd;
  bundle_cd.name = "bundle_cd";
  bundle_cd.id = 12;
  bundle_cd.period_ms = 100;
  bundle_cd.producers = {"node_c"};
  bundle_cd.consumers = {"node_d"};
  bundle_cd.signals = {102};
  config.bundles.push_back(bundle_cd);

  return config;
}

TEST(NodeFilteringTest, FilteredNodeOnlySeesRelevantSignals)
{
  Config config = create_multi_node_config();
  Config filtered_config = filter_for_target(config, "node_a");

  // Node A should only see signal 100 (bundle_ab)
  GeneratedNode node_a(filtered_config, "node_a");
  EXPECT_NE(proton_registry_get_signal(node_a.registry(), 100, nullptr), nullptr);
  EXPECT_EQ(proton_registry_get_signal(node_a.registry(), 101, nullptr), nullptr);
  EXPECT_EQ(proton_registry_get_signal(node_a.registry(), 102, nullptr), nullptr);
}

TEST(NodeFilteringTest, FilteredNodeOnlySeesRelevantBundles)
{
  Config config = create_multi_node_config();
  Config filtered_config = filter_for_target(config, "node_a");

  // Node A should only see bundle 10 (bundle_ab)
  GeneratedNode node_a(filtered_config, "node_a");
  EXPECT_NE(proton_registry_get_bundle(node_a.registry(), 10, nullptr), nullptr);
  EXPECT_EQ(proton_registry_get_bundle(node_a.registry(), 11, nullptr), nullptr);
  EXPECT_EQ(proton_registry_get_bundle(node_a.registry(), 12, nullptr), nullptr);
}

TEST(NodeFilteringTest, MiddleNodeSeesAdjacentBundles)
{
  Config config = create_multi_node_config();
  Config filtered_config = filter_for_target(config, "node_b");

  // Node B should see bundles 10 and 11
  GeneratedNode node_b(filtered_config, "node_b");
  EXPECT_NE(proton_registry_get_bundle(node_b.registry(), 10, nullptr), nullptr);
  EXPECT_NE(proton_registry_get_bundle(node_b.registry(), 11, nullptr), nullptr);
  EXPECT_EQ(proton_registry_get_bundle(node_b.registry(), 12, nullptr), nullptr);
}

TEST(NodeFilteringTest, MiddleNodeSeesAdjacentSignals)
{
  Config config = create_multi_node_config();
  Config filtered_config = filter_for_target(config, "node_b");

  // Node B should see signals 100 and 101
  GeneratedNode node_b(filtered_config, "node_b");
  EXPECT_NE(proton_registry_get_signal(node_b.registry(), 100, nullptr), nullptr);
  EXPECT_NE(proton_registry_get_signal(node_b.registry(), 101, nullptr), nullptr);
  EXPECT_EQ(proton_registry_get_signal(node_b.registry(), 102, nullptr), nullptr);
}

TEST(NodeFilteringTest, EndNodeOnlySeesItsBundle)
{
  Config config = create_multi_node_config();
  Config filtered_config = filter_for_target(config, "node_d");

  // Node D should only see bundle 12 (bundle_cd)
  GeneratedNode node_d(filtered_config, "node_d");
  EXPECT_EQ(proton_registry_get_bundle(node_d.registry(), 10, nullptr), nullptr);
  EXPECT_EQ(proton_registry_get_bundle(node_d.registry(), 11, nullptr), nullptr);
  EXPECT_NE(proton_registry_get_bundle(node_d.registry(), 12, nullptr), nullptr);
}

TEST(NodeFilteringTest, NodeOnlyHasPeersItConnectsTo)
{
  Config config = create_multi_node_config();

  // Node A should only have node B as a peer (1 peer)
  Config node_a_config = filter_for_target(config, "node_a");
  GeneratedNode node_a(node_a_config, "node_a");
  EXPECT_EQ(node_a.node()->num_peers, 1);

  // Node B should have nodes A and C as peers (2 peers)
  Config node_b_config = filter_for_target(config, "node_b");
  GeneratedNode node_b(node_b_config, "node_b");
  EXPECT_EQ(node_b.node()->num_peers, 2);

  // Node D should only have node C as a peer (1 peer)
  Config node_d_config = filter_for_target(config, "node_d");
  GeneratedNode node_d(node_d_config, "node_d");
  EXPECT_EQ(node_d.node()->num_peers, 1);
}

TEST(NodeFilteringTest, InvalidTargetNodeThrows)
{
  Config config = create_multi_node_config();

  EXPECT_THROW(filter_for_target(config, "nonexistent_node"), NodeBuilderException);
}

TEST(NodeFilteringTest, FilterForTargetFunction)
{
  Config config = create_multi_node_config();

  Config filtered = filter_for_target(config, "node_a");

  // Should have node_a and node_b (connected peer)
  EXPECT_TRUE(filtered.nodes.contains("node_a"));
  EXPECT_TRUE(filtered.nodes.contains("node_b"));
  EXPECT_FALSE(filtered.nodes.contains("node_c"));
  EXPECT_FALSE(filtered.nodes.contains("node_d"));

  // Should have 1 connection (a <-> b)
  EXPECT_EQ(filtered.connections.size(), 1);

  // Should have 1 bundle (bundle_ab)
  EXPECT_EQ(filtered.bundles.size(), 1);
  EXPECT_EQ(filtered.bundles[0].id, 10);

  // Should have 1 signal (signal_ab)
  EXPECT_EQ(filtered.signals.size(), 1);
  EXPECT_EQ(filtered.signals[0].id, 100);
}

int main(int argc, char ** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
