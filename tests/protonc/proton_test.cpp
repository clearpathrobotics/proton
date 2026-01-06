/**
 * Software License Agreement (proprietary)
 *
 * @copyright Copyright (c) 2025 Clearpath Robotics, Inc., All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, is not permitted without the express permission of Clearpath
 * Robotics.
 *
 * @author Roni Kreinin (rkreinin@clearpathrobotics.com)
 */

#include "protonc/utils.hpp"

uint8_t read_buf[PROTON_MAX_MESSAGE_SIZE];
uint8_t write_buf[PROTON_MAX_MESSAGE_SIZE];
proton_buffer_t proton_producer_buffer = {write_buf, PROTON_MAX_MESSAGE_SIZE};
proton_buffer_t proton_consumer_buffer = {read_buf, PROTON_MAX_MESSAGE_SIZE};

TEST(PROTONC_Proton, InitBundle) {
  // Create structs to hold bundle and signals
  proton_bundle_handle_t test_bundle_handle;
  proton_signal_handle_t test_signal_handles[PROTON__BUNDLE__VALUE_TEST__SIGNAL__COUNT];
  proton_bundle_value_test_t test_bundle;

  proton_init_bundle(&test_bundle_handle, PROTON__BUNDLE__VALUE_TEST, test_signal_handles, PROTON__BUNDLE__VALUE_TEST__SIGNAL__COUNT, PROTON__BUNDLE__VALUE_TEST__PRODUCERS, PROTON__BUNDLE__VALUE_TEST__CONSUMERS);

  ASSERT_EQ(test_bundle_handle.bundle.id, PROTON__BUNDLE__VALUE_TEST);
  ASSERT_EQ(test_bundle_handle.bundle.signals, &test_bundle_handle.signals);
  ASSERT_EQ(test_bundle_handle.signals.data, test_signal_handles);
  ASSERT_EQ(test_bundle_handle.signals.length, PROTON__BUNDLE__VALUE_TEST__SIGNAL__COUNT);
  ASSERT_EQ(test_bundle_handle.signals.size, 0);
  ASSERT_EQ(test_bundle_handle.producers, PROTON__BUNDLE__VALUE_TEST__PRODUCERS);
  ASSERT_EQ(test_bundle_handle.consumers, PROTON__BUNDLE__VALUE_TEST__CONSUMERS);
}

TEST(PROTONC_Proton, InitNode) {
  proton_node_t producer_node = PROTON__NODE__PRODUCER__DEFAULT_VALUE;
  proton_peer_t producer_peers[PROTON__PEER__COUNT] = {PROTON__NODE__CONSUMER__PEER__DEFAULT_VALUE};

  ASSERT_EQ(proton_peer_consumer_init(&producer_peers[PROTON__PEER__CONSUMER], proton_consumer_buffer), PROTON_OK);

  ASSERT_EQ(proton_node_producer_init(
    &producer_node,
    producer_peers,
    proton_producer_buffer,
    nullptr),
  PROTON_OK);

  ASSERT_STREQ(producer_node.name, PROTON__NODE__PRODUCER__NAME);
  ASSERT_EQ(producer_node.heartbeat.enabled, PROTON__NODE__PRODUCER__HEARTBEAT__ENABLED);
  ASSERT_EQ(producer_node.heartbeat.period, PROTON__NODE__PRODUCER__HEARTBEAT__PERIOD);
  ASSERT_EQ(producer_node.peers[PROTON__PEER__CONSUMER].transport.connect, proton_node_consumer_transport_connect);
  ASSERT_EQ(producer_node.peers[PROTON__PEER__CONSUMER].transport.disconnect, proton_node_consumer_transport_disconnect);
  ASSERT_EQ(producer_node.peers[PROTON__PEER__CONSUMER].transport.read, proton_node_consumer_transport_read);
  ASSERT_EQ(producer_node.peers[PROTON__PEER__CONSUMER].transport.write, proton_node_consumer_transport_write);
  ASSERT_EQ(producer_node.peers[PROTON__PEER__CONSUMER].receive, proton_node_consumer_receive);
  ASSERT_EQ(producer_node.atomic_buffer.buffer.data, proton_producer_buffer.data);
  ASSERT_EQ(producer_node.atomic_buffer.buffer.len, proton_producer_buffer.len);
  ASSERT_EQ(producer_node.state, PROTON_NODE_ACTIVE);
}

TEST(PROTONC_Proton, Encode) {
  // Create structs to hold bundle and signals
  proton_bundle_handle_t test_bundle_handle;
  proton_signal_handle_t test_signal_handles[PROTON__BUNDLE__VALUE_TEST__SIGNAL__COUNT];
  proton_bundle_value_test_t test_bundle;

  // Create test values to test against
  double double_value = 1.234;
  float float_value = 1.23f;
  int32_t int32_value = -12;
  int64_t int64_value = -125;
  uint32_t uint32_value = 32;
  uint64_t uint64_value = 64;
  bool bool_value = true;
  char string_value[PROTON__BUNDLE__VALUE_TEST__SIGNAL__STRING_VALUE__CAPACITY] = "test";
  uint8_t bytes_value[PROTON__BUNDLE__VALUE_TEST__SIGNAL__BYTES_VALUE__CAPACITY] = {0, 1, 2, 3};
  double list_double_value[PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_DOUBLE_VALUE__LENGTH] = {0.1, 0.2};
  float list_float_value[PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_FLOAT_VALUE__LENGTH] = {0.12f, 0.23f};
  int32_t list_int32_value[PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_INT32_VALUE__LENGTH] = {1, 2};
  int64_t list_int64_value[PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_INT64_VALUE__LENGTH] = {3, 4};
  uint32_t list_uint32_value[PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_UINT32_VALUE__LENGTH] = {5, 6};
  uint64_t list_uint64_value[PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_UINT64_VALUE__LENGTH] = {7, 8};
  bool list_bool_value[PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_BOOL_VALUE__LENGTH] = {true, false};
  char list_string_value[PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_STRING_VALUE__LENGTH][PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_STRING_VALUE__CAPACITY] = {"test1", "test2"};
  uint8_t list_bytes_value[PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_BYTES_VALUE__LENGTH][PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_BYTES_VALUE__CAPACITY] = {{0, 1}, {2, 3}};

  // Copy test values into signals
  test_bundle.double_value = double_value;
  test_bundle.float_value = float_value;
  test_bundle.int32_value = int32_value;
  test_bundle.int64_value = int64_value;
  test_bundle.uint32_value = uint32_value;
  test_bundle.uint64_value = uint64_value;
  test_bundle.bool_value = bool_value;
  strncpy(test_bundle.string_value, string_value, PROTON__BUNDLE__VALUE_TEST__SIGNAL__STRING_VALUE__CAPACITY);
  memcpy(test_bundle.bytes_value, bytes_value, sizeof(bytes_value));
  memcpy(test_bundle.list_double_value, list_double_value, sizeof(list_double_value));
  memcpy(test_bundle.list_float_value, list_float_value, sizeof(list_float_value));
  memcpy(test_bundle.list_int32_value, list_int32_value, sizeof(list_int32_value));
  memcpy(test_bundle.list_int64_value, list_int64_value, sizeof(list_int64_value));
  memcpy(test_bundle.list_uint32_value, list_uint32_value, sizeof(list_uint32_value));
  memcpy(test_bundle.list_uint64_value, list_uint64_value, sizeof(list_uint64_value));
  memcpy(test_bundle.list_bool_value, list_bool_value, sizeof(list_bool_value));
  memcpy(test_bundle.list_string_value, list_string_value, sizeof(list_string_value));
  memcpy(test_bundle.list_bytes_value, list_bytes_value, sizeof(list_bytes_value));

  // Initialize signals
  EXPECT_EQ(proton_init_signal(&test_signal_handles[PROTON__BUNDLE__VALUE_TEST__SIGNAL__DOUBLE_VALUE], proton_Signal_double_value_tag, &test_bundle.double_value, 0, 0), PROTON_OK);
  EXPECT_EQ(proton_init_signal(&test_signal_handles[PROTON__BUNDLE__VALUE_TEST__SIGNAL__FLOAT_VALUE], proton_Signal_float_value_tag, &test_bundle.float_value, 0, 0), PROTON_OK);
  EXPECT_EQ(proton_init_signal(&test_signal_handles[PROTON__BUNDLE__VALUE_TEST__SIGNAL__INT32_VALUE], proton_Signal_int32_value_tag, &test_bundle.int32_value, 0, 0), PROTON_OK);
  EXPECT_EQ(proton_init_signal(&test_signal_handles[PROTON__BUNDLE__VALUE_TEST__SIGNAL__INT64_VALUE], proton_Signal_int64_value_tag, &test_bundle.int64_value, 0, 0), PROTON_OK);
  EXPECT_EQ(proton_init_signal(&test_signal_handles[PROTON__BUNDLE__VALUE_TEST__SIGNAL__UINT32_VALUE], proton_Signal_uint32_value_tag, &test_bundle.uint32_value, 0, 0), PROTON_OK);
  EXPECT_EQ(proton_init_signal(&test_signal_handles[PROTON__BUNDLE__VALUE_TEST__SIGNAL__UINT64_VALUE], proton_Signal_uint64_value_tag, &test_bundle.uint64_value, 0, 0), PROTON_OK);
  EXPECT_EQ(proton_init_signal(&test_signal_handles[PROTON__BUNDLE__VALUE_TEST__SIGNAL__BOOL_VALUE], proton_Signal_bool_value_tag, &test_bundle.bool_value, 0, 0), PROTON_OK);
  EXPECT_EQ(proton_init_signal(&test_signal_handles[PROTON__BUNDLE__VALUE_TEST__SIGNAL__STRING_VALUE], proton_Signal_string_value_tag, &test_bundle.string_value, 0, PROTON__BUNDLE__VALUE_TEST__SIGNAL__STRING_VALUE__CAPACITY), PROTON_OK);
  EXPECT_EQ(proton_init_signal(&test_signal_handles[PROTON__BUNDLE__VALUE_TEST__SIGNAL__BYTES_VALUE], proton_Signal_bytes_value_tag, &test_bundle.bytes_value, 0, PROTON__BUNDLE__VALUE_TEST__SIGNAL__BYTES_VALUE__CAPACITY), PROTON_OK);
  EXPECT_EQ(proton_init_signal(&test_signal_handles[PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_DOUBLE_VALUE], proton_Signal_list_double_value_tag, &test_bundle.list_double_value, PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_DOUBLE_VALUE__LENGTH, 0), PROTON_OK);
  EXPECT_EQ(proton_init_signal(&test_signal_handles[PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_FLOAT_VALUE], proton_Signal_list_float_value_tag, &test_bundle.list_float_value, PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_FLOAT_VALUE__LENGTH, 0), PROTON_OK);
  EXPECT_EQ(proton_init_signal(&test_signal_handles[PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_INT32_VALUE], proton_Signal_list_int32_value_tag, &test_bundle.list_int32_value, PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_INT32_VALUE__LENGTH, 0), PROTON_OK);
  EXPECT_EQ(proton_init_signal(&test_signal_handles[PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_INT64_VALUE], proton_Signal_list_int64_value_tag, &test_bundle.list_int64_value, PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_INT64_VALUE__LENGTH, 0), PROTON_OK);
  EXPECT_EQ(proton_init_signal(&test_signal_handles[PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_UINT32_VALUE], proton_Signal_list_uint32_value_tag, &test_bundle.list_uint32_value, PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_UINT32_VALUE__LENGTH, 0), PROTON_OK);
  EXPECT_EQ(proton_init_signal(&test_signal_handles[PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_UINT64_VALUE], proton_Signal_list_uint64_value_tag, &test_bundle.list_uint64_value, PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_UINT64_VALUE__LENGTH, 0), PROTON_OK);
  EXPECT_EQ(proton_init_signal(&test_signal_handles[PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_BOOL_VALUE], proton_Signal_list_bool_value_tag, &test_bundle.list_bool_value, PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_BOOL_VALUE__LENGTH, 0), PROTON_OK);
  EXPECT_EQ(proton_init_signal(&test_signal_handles[PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_STRING_VALUE], proton_Signal_list_string_value_tag, &test_bundle.list_string_value, PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_STRING_VALUE__LENGTH, PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_STRING_VALUE__CAPACITY), PROTON_OK);
  EXPECT_EQ(proton_init_signal(&test_signal_handles[PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_BYTES_VALUE], proton_Signal_list_bytes_value_tag, &test_bundle.list_bytes_value, PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_BYTES_VALUE__LENGTH, PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_BYTES_VALUE__CAPACITY), PROTON_OK);


  // Initialise bundle
  proton_status_e status = proton_init_bundle(&test_bundle_handle, PROTON__BUNDLE__VALUE_TEST, test_signal_handles, PROTON__BUNDLE__VALUE_TEST__SIGNAL__COUNT, 0, 0);
  EXPECT_EQ(status, PROTON_OK);

  // Encode bundle
  size_t bytes_encoded;
  status = proton_encode(&test_bundle_handle, proton_producer_buffer, &bytes_encoded);
  EXPECT_EQ(status, PROTON_OK);

  // More than 0 bytes written if encoding is successful
  EXPECT_GT(bytes_encoded, 0);
}

TEST(PROTONC_Proton, DecodeId) {
  // Create structs to hold bundle and signals
  proton_bundle_handle_t test_bundle_handle;
  proton_signal_handle_t test_signal_handle;
  proton_bundle_value_test_t test_bundle;

  // Initialise bundle with ID 0x123
  proton_status_e status = proton_init_bundle(&test_bundle_handle, 0x123, &test_signal_handle, 0, 0, 0);
  EXPECT_EQ(status, PROTON_OK);

  // Encode bundle
  size_t bytes_encoded;
  status = proton_encode(&test_bundle_handle, proton_producer_buffer, &bytes_encoded);
  EXPECT_EQ(status, PROTON_OK);

  // More than 0 bytes written if encoding is successful
  EXPECT_GT(bytes_encoded, 0);

  uint32_t id = 0;

  // Decoding should succeed
  EXPECT_EQ(proton_decode_id(proton_producer_buffer, &id), PROTON_OK);

  // ID should match
  ASSERT_EQ(id, 0x123);
}

TEST(PROTONC_Proton, Decode) {
  // Create structs to hold bundle and signals
  proton_bundle_handle_t test_bundle_handle;
  proton_signal_handle_t test_signal_handles[PROTON__BUNDLE__VALUE_TEST__SIGNAL__COUNT];
  proton_bundle_value_test_t test_bundle;

  // Create test values to test against
  double double_value = 1.234;
  float float_value = 1.23f;
  int32_t int32_value = -12;
  int64_t int64_value = -125;
  uint32_t uint32_value = 32;
  uint64_t uint64_value = 64;
  bool bool_value = true;
  char string_value[PROTON__BUNDLE__VALUE_TEST__SIGNAL__STRING_VALUE__CAPACITY] = "test";
  uint8_t bytes_value[PROTON__BUNDLE__VALUE_TEST__SIGNAL__BYTES_VALUE__CAPACITY] = {0, 1, 2, 3};
  double list_double_value[PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_DOUBLE_VALUE__LENGTH] = {0.1, 0.2};
  float list_float_value[PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_FLOAT_VALUE__LENGTH] = {0.12f, 0.23f};
  int32_t list_int32_value[PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_INT32_VALUE__LENGTH] = {1, 2};
  int64_t list_int64_value[PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_INT64_VALUE__LENGTH] = {3, 4};
  uint32_t list_uint32_value[PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_UINT32_VALUE__LENGTH] = {5, 6};
  uint64_t list_uint64_value[PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_UINT64_VALUE__LENGTH] = {7, 8};
  bool list_bool_value[PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_BOOL_VALUE__LENGTH] = {true, false};
  char list_string_value[PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_STRING_VALUE__LENGTH][PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_STRING_VALUE__CAPACITY] = {"test1", "test2"};
  uint8_t list_bytes_value[PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_BYTES_VALUE__LENGTH][PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_BYTES_VALUE__CAPACITY] = {{0, 1}, {2, 3}};

  // Copy test values into signals
  test_bundle.double_value = double_value;
  test_bundle.float_value = float_value;
  test_bundle.int32_value = int32_value;
  test_bundle.int64_value = int64_value;
  test_bundle.uint32_value = uint32_value;
  test_bundle.uint64_value = uint64_value;
  test_bundle.bool_value = bool_value;
  strncpy(test_bundle.string_value, string_value, PROTON__BUNDLE__VALUE_TEST__SIGNAL__STRING_VALUE__CAPACITY);
  memcpy(test_bundle.bytes_value, bytes_value, sizeof(bytes_value));
  memcpy(test_bundle.list_double_value, list_double_value, sizeof(list_double_value));
  memcpy(test_bundle.list_float_value, list_float_value, sizeof(list_float_value));
  memcpy(test_bundle.list_int32_value, list_int32_value, sizeof(list_int32_value));
  memcpy(test_bundle.list_int64_value, list_int64_value, sizeof(list_int64_value));
  memcpy(test_bundle.list_uint32_value, list_uint32_value, sizeof(list_uint32_value));
  memcpy(test_bundle.list_uint64_value, list_uint64_value, sizeof(list_uint64_value));
  memcpy(test_bundle.list_bool_value, list_bool_value, sizeof(list_bool_value));
  memcpy(test_bundle.list_string_value, list_string_value, sizeof(list_string_value));
  memcpy(test_bundle.list_bytes_value, list_bytes_value, sizeof(list_bytes_value));

  // Initialize signals
  EXPECT_EQ(proton_init_signal(&test_signal_handles[PROTON__BUNDLE__VALUE_TEST__SIGNAL__DOUBLE_VALUE], proton_Signal_double_value_tag, &test_bundle.double_value, 0, 0), PROTON_OK);
  EXPECT_EQ(proton_init_signal(&test_signal_handles[PROTON__BUNDLE__VALUE_TEST__SIGNAL__FLOAT_VALUE], proton_Signal_float_value_tag, &test_bundle.float_value, 0, 0), PROTON_OK);
  EXPECT_EQ(proton_init_signal(&test_signal_handles[PROTON__BUNDLE__VALUE_TEST__SIGNAL__INT32_VALUE], proton_Signal_int32_value_tag, &test_bundle.int32_value, 0, 0), PROTON_OK);
  EXPECT_EQ(proton_init_signal(&test_signal_handles[PROTON__BUNDLE__VALUE_TEST__SIGNAL__INT64_VALUE], proton_Signal_int64_value_tag, &test_bundle.int64_value, 0, 0), PROTON_OK);
  EXPECT_EQ(proton_init_signal(&test_signal_handles[PROTON__BUNDLE__VALUE_TEST__SIGNAL__UINT32_VALUE], proton_Signal_uint32_value_tag, &test_bundle.uint32_value, 0, 0), PROTON_OK);
  EXPECT_EQ(proton_init_signal(&test_signal_handles[PROTON__BUNDLE__VALUE_TEST__SIGNAL__UINT64_VALUE], proton_Signal_uint64_value_tag, &test_bundle.uint64_value, 0, 0), PROTON_OK);
  EXPECT_EQ(proton_init_signal(&test_signal_handles[PROTON__BUNDLE__VALUE_TEST__SIGNAL__BOOL_VALUE], proton_Signal_bool_value_tag, &test_bundle.bool_value, 0, 0), PROTON_OK);
  EXPECT_EQ(proton_init_signal(&test_signal_handles[PROTON__BUNDLE__VALUE_TEST__SIGNAL__STRING_VALUE], proton_Signal_string_value_tag, &test_bundle.string_value, 0, PROTON__BUNDLE__VALUE_TEST__SIGNAL__STRING_VALUE__CAPACITY), PROTON_OK);
  EXPECT_EQ(proton_init_signal(&test_signal_handles[PROTON__BUNDLE__VALUE_TEST__SIGNAL__BYTES_VALUE], proton_Signal_bytes_value_tag, &test_bundle.bytes_value, 0, PROTON__BUNDLE__VALUE_TEST__SIGNAL__BYTES_VALUE__CAPACITY), PROTON_OK);
  EXPECT_EQ(proton_init_signal(&test_signal_handles[PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_DOUBLE_VALUE], proton_Signal_list_double_value_tag, &test_bundle.list_double_value, PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_DOUBLE_VALUE__LENGTH, 0), PROTON_OK);
  EXPECT_EQ(proton_init_signal(&test_signal_handles[PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_FLOAT_VALUE], proton_Signal_list_float_value_tag, &test_bundle.list_float_value, PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_FLOAT_VALUE__LENGTH, 0), PROTON_OK);
  EXPECT_EQ(proton_init_signal(&test_signal_handles[PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_INT32_VALUE], proton_Signal_list_int32_value_tag, &test_bundle.list_int32_value, PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_INT32_VALUE__LENGTH, 0), PROTON_OK);
  EXPECT_EQ(proton_init_signal(&test_signal_handles[PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_INT64_VALUE], proton_Signal_list_int64_value_tag, &test_bundle.list_int64_value, PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_INT64_VALUE__LENGTH, 0), PROTON_OK);
  EXPECT_EQ(proton_init_signal(&test_signal_handles[PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_UINT32_VALUE], proton_Signal_list_uint32_value_tag, &test_bundle.list_uint32_value, PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_UINT32_VALUE__LENGTH, 0), PROTON_OK);
  EXPECT_EQ(proton_init_signal(&test_signal_handles[PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_UINT64_VALUE], proton_Signal_list_uint64_value_tag, &test_bundle.list_uint64_value, PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_UINT64_VALUE__LENGTH, 0), PROTON_OK);
  EXPECT_EQ(proton_init_signal(&test_signal_handles[PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_BOOL_VALUE], proton_Signal_list_bool_value_tag, &test_bundle.list_bool_value, PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_BOOL_VALUE__LENGTH, 0), PROTON_OK);
  EXPECT_EQ(proton_init_signal(&test_signal_handles[PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_STRING_VALUE], proton_Signal_list_string_value_tag, &test_bundle.list_string_value, PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_STRING_VALUE__LENGTH, PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_STRING_VALUE__CAPACITY), PROTON_OK);
  EXPECT_EQ(proton_init_signal(&test_signal_handles[PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_BYTES_VALUE], proton_Signal_list_bytes_value_tag, &test_bundle.list_bytes_value, PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_BYTES_VALUE__LENGTH, PROTON__BUNDLE__VALUE_TEST__SIGNAL__LIST_BYTES_VALUE__CAPACITY), PROTON_OK);


  // Initialise bundle
  proton_status_e status = proton_init_bundle(&test_bundle_handle, PROTON__BUNDLE__VALUE_TEST, test_signal_handles, PROTON__BUNDLE__VALUE_TEST__SIGNAL__COUNT, 0, 0);
  EXPECT_EQ(status, PROTON_OK);

  // Encode bundle
  size_t bytes_encoded;
  status = proton_encode(&test_bundle_handle, proton_producer_buffer, &bytes_encoded);
  EXPECT_EQ(status, PROTON_OK);

  // More than 0 bytes written if encoding is successful
  EXPECT_GT(bytes_encoded, 0);

  uint32_t id = 0;

  // Decode ID
  EXPECT_EQ(proton_decode_id(proton_producer_buffer, &id), PROTON_OK);

  // ID should be the same
  EXPECT_EQ(id, PROTON__BUNDLE__VALUE_TEST);

  // Decode bundle in place
  status = proton_decode(&test_bundle_handle, proton_producer_buffer, bytes_encoded);

  // Check that all bytes were successfully decoded
  EXPECT_EQ(status, PROTON_OK);

  // Data in test struct should be decoded correctly
  EXPECT_EQ(test_bundle.double_value, double_value);
  EXPECT_EQ(test_bundle.float_value, float_value);
  EXPECT_EQ(test_bundle.int32_value, int32_value);
  EXPECT_EQ(test_bundle.int64_value, int64_value);
  EXPECT_EQ(test_bundle.uint32_value, uint32_value);
  EXPECT_EQ(test_bundle.uint64_value, uint64_value);
  EXPECT_EQ(test_bundle.bool_value, bool_value);
  EXPECT_STREQ(test_bundle.string_value, string_value);
  EXPECT_EQ(memcmp(test_bundle.bytes_value, bytes_value, sizeof(bytes_value)), 0);
  EXPECT_EQ(memcmp(test_bundle.list_double_value, list_double_value, sizeof(list_double_value)), 0);
  EXPECT_EQ(memcmp(test_bundle.list_float_value, list_float_value, sizeof(list_float_value)), 0);
  EXPECT_EQ(memcmp(test_bundle.list_int32_value, list_int32_value, sizeof(list_int32_value)), 0);
  EXPECT_EQ(memcmp(test_bundle.list_int64_value, list_int64_value, sizeof(list_int64_value)), 0);
  EXPECT_EQ(memcmp(test_bundle.list_uint32_value, list_uint32_value, sizeof(list_uint32_value)), 0);
  EXPECT_EQ(memcmp(test_bundle.list_uint64_value, list_uint64_value, sizeof(list_uint64_value)), 0);
  EXPECT_EQ(memcmp(test_bundle.list_bool_value, list_bool_value, sizeof(list_bool_value)), 0);
  EXPECT_EQ(memcmp(test_bundle.list_string_value, list_string_value, sizeof(list_string_value)), 0);
  EXPECT_EQ(memcmp(test_bundle.list_bytes_value, list_bytes_value, sizeof(list_bytes_value)), 0);
}

// TEST(PROTONC_Proton, SpinOnce)
// {
//   // Null node should return error
//   ASSERT_EQ(PROTON_SpinOnce(NULL), PROTON_ERROR);

//   proton_node_t node = proton_node_default;

//   // Uninitialised node should report an error
//   ASSERT_EQ(PROTON_SpinOnce(&node), PROTON_ERROR);

//   // Set node to connected state
//   node.connected = true;

//   proton_transport_read_t read_func = [](uint8_t *buf, size_t len)->size_t { return 1; };
//   proton_receive_t receive_func = [](const uint8_t *buf, size_t len)->bool { return false; };
//   node.transport.read = read_func;

//   // Initialise node without defined buffers
//   proton_configure(&node, node.transport, receive_func, proton_buffer_default, proton_buffer_default);

//   ASSERT_EQ(PROTON_SpinOnce(&node), PROTON_ERROR);

//   uint8_t buffer[256];
//   proton_buffer_t read_buffer = {buffer, sizeof(buffer)};

//   // Initialise node with a read buffer
//   proton_configure(&node, node.transport, receive_func, read_buffer, proton_buffer_default);

//   ASSERT_EQ(PROTON_SpinOnce(&node), PROTON_READ_ERROR);
// }


int main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
