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

TEST(PROTONC_Proton, InitBundle) {
  // Create structs to hold bundle and signals
  proton_bundle_handle_t test_bundle_handle;
  proton_signal_handle_t test_signal_handles[PROTON_SIGNALS__VALUE_TEST_COUNT];
  PROTON_BUNDLE__value_test_t test_bundle;

  PROTON_InitBundle(&test_bundle_handle, PROTON_BUNDLE__VALUE_TEST, test_signal_handles, PROTON_SIGNALS__VALUE_TEST_COUNT);

  ASSERT_EQ(test_bundle_handle.bundle.id, PROTON_BUNDLE__VALUE_TEST);
  ASSERT_EQ(test_bundle_handle.bundle.signals, &test_bundle_handle.arg);
  ASSERT_EQ(test_bundle_handle.arg.data, test_signal_handles);
  ASSERT_EQ(test_bundle_handle.arg.length, PROTON_SIGNALS__VALUE_TEST_COUNT);
  ASSERT_EQ(test_bundle_handle.arg.size, 0);
}

TEST(PROTONC_Proton, InitNode) {
  proton_transport_t producer_transport;
  producer_transport.connect = PROTON_TRANSPORT__ProducerConnect;
  producer_transport.disconnect = PROTON_TRANSPORT__ProducerDisconnect;
  producer_transport.read = PROTON_TRANSPORT__ProducerRead;
  producer_transport.write = PROTON_TRANSPORT__ProducerWrite;

  PROTON_InitNode(&producer_node, producer_transport, PROTON_BUNDLE_Receive, proton_producer_read_buffer, proton_producer_write_buffer);

  ASSERT_EQ(producer_node.transport.connect, PROTON_TRANSPORT__ProducerConnect);
  ASSERT_EQ(producer_node.transport.disconnect, PROTON_TRANSPORT__ProducerDisconnect);
  ASSERT_EQ(producer_node.transport.read, PROTON_TRANSPORT__ProducerRead);
  ASSERT_EQ(producer_node.transport.write, PROTON_TRANSPORT__ProducerWrite);
  ASSERT_EQ(producer_node.receive, PROTON_BUNDLE_Receive);
  ASSERT_EQ(producer_node.read_buf.data, proton_producer_read_buffer.data);
  ASSERT_EQ(producer_node.read_buf.len, proton_producer_read_buffer.len);
  ASSERT_EQ(producer_node.write_buf.data, proton_producer_write_buffer.data);
  ASSERT_EQ(producer_node.write_buf.len, proton_producer_write_buffer.len);
  EXPECT_FALSE(producer_node.connected);
}


TEST(PROTONC_Proton, Encode) {
  // Buffer to encode with
  uint8_t buffer[256];

  // Create structs to hold bundle and signals
  proton_bundle_handle_t test_bundle_handle;
  proton_signal_handle_t test_signal_handles[PROTON_SIGNALS__VALUE_TEST_COUNT];
  PROTON_BUNDLE__value_test_t test_bundle;

  // Create test values to test against
  double double_value = 1.234;
  float float_value = 1.23f;
  int32_t int32_value = -12;
  int64_t int64_value = -125;
  uint32_t uint32_value = 32;
  uint64_t uint64_value = 64;
  bool bool_value = true;
  char string_value[PROTON_SIGNALS__VALUE_TEST__STRING_VALUE__CAPACITY] = "test";
  uint8_t bytes_value[PROTON_SIGNALS__VALUE_TEST__BYTES_VALUE__CAPACITY] = {0, 1, 2, 3};
  double list_double_value[PROTON_SIGNALS__VALUE_TEST__LIST_DOUBLE_VALUE__LENGTH] = {0.1, 0.2};
  float list_float_value[PROTON_SIGNALS__VALUE_TEST__LIST_FLOAT_VALUE__LENGTH] = {0.12f, 0.23f};
  int32_t list_int32_value[PROTON_SIGNALS__VALUE_TEST__LIST_INT32_VALUE__LENGTH] = {1, 2};
  int64_t list_int64_value[PROTON_SIGNALS__VALUE_TEST__LIST_INT64_VALUE__LENGTH] = {3, 4};
  uint32_t list_uint32_value[PROTON_SIGNALS__VALUE_TEST__LIST_UINT32_VALUE__LENGTH] = {5, 6};
  uint64_t list_uint64_value[PROTON_SIGNALS__VALUE_TEST__LIST_UINT64_VALUE__LENGTH] = {7, 8};
  bool list_bool_value[PROTON_SIGNALS__VALUE_TEST__LIST_BOOL_VALUE__LENGTH] = {true, false};
  char list_string_value[PROTON_SIGNALS__VALUE_TEST__LIST_STRING_VALUE__LENGTH][PROTON_SIGNALS__VALUE_TEST__LIST_STRING_VALUE__CAPACITY] = {"test1", "test2"};
  uint8_t list_bytes_value[PROTON_SIGNALS__VALUE_TEST__LIST_BYTES_VALUE__LENGTH][PROTON_SIGNALS__VALUE_TEST__LIST_BYTES_VALUE__CAPACITY] = {{0, 1}, {2, 3}};

  // Copy test values into signals
  test_bundle.double_value = double_value;
  test_bundle.float_value = float_value;
  test_bundle.int32_value = int32_value;
  test_bundle.int64_value = int64_value;
  test_bundle.uint32_value = uint32_value;
  test_bundle.uint64_value = uint64_value;
  test_bundle.bool_value = bool_value;
  strncpy(test_bundle.string_value, string_value, PROTON_SIGNALS__VALUE_TEST__STRING_VALUE__CAPACITY);
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
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__DOUBLE_VALUE].signal.which_signal = proton_Signal_double_value_tag;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__DOUBLE_VALUE].arg.data = &value_test_bundle.double_value;

  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__FLOAT_VALUE].signal.which_signal = proton_Signal_float_value_tag;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__FLOAT_VALUE].arg.data = &value_test_bundle.float_value;

  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__INT32_VALUE].signal.which_signal = proton_Signal_int32_value_tag;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__INT32_VALUE].arg.data = &value_test_bundle.int32_value;

  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__INT64_VALUE].signal.which_signal = proton_Signal_int64_value_tag;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__INT64_VALUE].arg.data = &value_test_bundle.int64_value;

  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__UINT32_VALUE].signal.which_signal = proton_Signal_uint32_value_tag;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__UINT32_VALUE].arg.data = &value_test_bundle.uint32_value;

  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__UINT64_VALUE].signal.which_signal = proton_Signal_uint64_value_tag;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__UINT64_VALUE].arg.data = &value_test_bundle.uint64_value;

  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__BOOL_VALUE].signal.which_signal = proton_Signal_bool_value_tag;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__BOOL_VALUE].arg.data = &value_test_bundle.bool_value;

  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__STRING_VALUE].signal.which_signal = proton_Signal_string_value_tag;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__STRING_VALUE].signal.signal.string_value = &test_signal_handles[PROTON_SIGNALS__VALUE_TEST__STRING_VALUE].arg;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__STRING_VALUE].arg.data = value_test_bundle.string_value;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__STRING_VALUE].arg.capacity = PROTON_SIGNALS__VALUE_TEST__STRING_VALUE__CAPACITY;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__STRING_VALUE].arg.size = 0;

  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__BYTES_VALUE].signal.which_signal = proton_Signal_bytes_value_tag;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__BYTES_VALUE].signal.signal.bytes_value = &test_signal_handles[PROTON_SIGNALS__VALUE_TEST__BYTES_VALUE].arg;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__BYTES_VALUE].arg.data = value_test_bundle.bytes_value;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__BYTES_VALUE].arg.capacity = PROTON_SIGNALS__VALUE_TEST__BYTES_VALUE__CAPACITY;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__BYTES_VALUE].arg.size = 0;

  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_DOUBLE_VALUE].signal.which_signal = proton_Signal_list_double_value_tag;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_DOUBLE_VALUE].signal.signal.list_double_value.doubles = &test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_DOUBLE_VALUE].arg;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_DOUBLE_VALUE].arg.data = value_test_bundle.list_double_value;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_DOUBLE_VALUE].arg.length = PROTON_SIGNALS__VALUE_TEST__LIST_DOUBLE_VALUE__LENGTH;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_DOUBLE_VALUE].arg.size = 0;

  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_FLOAT_VALUE].signal.which_signal = proton_Signal_list_float_value_tag;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_FLOAT_VALUE].signal.signal.list_float_value.floats = &test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_FLOAT_VALUE].arg;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_FLOAT_VALUE].arg.data = value_test_bundle.list_float_value;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_FLOAT_VALUE].arg.length = PROTON_SIGNALS__VALUE_TEST__LIST_FLOAT_VALUE__LENGTH;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_FLOAT_VALUE].arg.size = 0;

  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_INT32_VALUE].signal.which_signal = proton_Signal_list_int32_value_tag;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_INT32_VALUE].signal.signal.list_int32_value.int32s = &test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_INT32_VALUE].arg;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_INT32_VALUE].arg.data = value_test_bundle.list_int32_value;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_INT32_VALUE].arg.length = PROTON_SIGNALS__VALUE_TEST__LIST_INT32_VALUE__LENGTH;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_INT32_VALUE].arg.size = 0;

  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_INT64_VALUE].signal.which_signal = proton_Signal_list_int64_value_tag;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_INT64_VALUE].signal.signal.list_int64_value.int64s = &test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_INT64_VALUE].arg;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_INT64_VALUE].arg.data = value_test_bundle.list_int64_value;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_INT64_VALUE].arg.length = PROTON_SIGNALS__VALUE_TEST__LIST_INT64_VALUE__LENGTH;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_INT64_VALUE].arg.size = 0;

  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_UINT32_VALUE].signal.which_signal = proton_Signal_list_uint32_value_tag;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_UINT32_VALUE].signal.signal.list_uint32_value.uint32s = &test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_UINT32_VALUE].arg;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_UINT32_VALUE].arg.data = value_test_bundle.list_uint32_value;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_UINT32_VALUE].arg.length = PROTON_SIGNALS__VALUE_TEST__LIST_UINT32_VALUE__LENGTH;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_UINT32_VALUE].arg.size = 0;

  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_UINT64_VALUE].signal.which_signal = proton_Signal_list_uint64_value_tag;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_UINT64_VALUE].signal.signal.list_uint64_value.uint64s = &test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_UINT64_VALUE].arg;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_UINT64_VALUE].arg.data = value_test_bundle.list_uint64_value;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_UINT64_VALUE].arg.length = PROTON_SIGNALS__VALUE_TEST__LIST_UINT64_VALUE__LENGTH;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_UINT64_VALUE].arg.size = 0;

  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_BOOL_VALUE].signal.which_signal = proton_Signal_list_bool_value_tag;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_BOOL_VALUE].signal.signal.list_bool_value.bools = &test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_BOOL_VALUE].arg;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_BOOL_VALUE].arg.data = value_test_bundle.list_bool_value;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_BOOL_VALUE].arg.length = PROTON_SIGNALS__VALUE_TEST__LIST_BOOL_VALUE__LENGTH;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_BOOL_VALUE].arg.size = 0;

  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_STRING_VALUE].signal.which_signal = proton_Signal_list_string_value_tag;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_STRING_VALUE].signal.signal.list_string_value.strings = &test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_STRING_VALUE].arg;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_STRING_VALUE].arg.data = value_test_bundle.list_string_value;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_STRING_VALUE].arg.capacity = PROTON_SIGNALS__VALUE_TEST__LIST_STRING_VALUE__CAPACITY;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_STRING_VALUE].arg.length = PROTON_SIGNALS__VALUE_TEST__LIST_STRING_VALUE__LENGTH;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_STRING_VALUE].arg.size = 0;

  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_BYTES_VALUE].signal.which_signal = proton_Signal_list_bytes_value_tag;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_BYTES_VALUE].signal.signal.list_bytes_value.bytes = &test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_BYTES_VALUE].arg;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_BYTES_VALUE].arg.data = value_test_bundle.list_bytes_value;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_BYTES_VALUE].arg.capacity = PROTON_SIGNALS__VALUE_TEST__LIST_BYTES_VALUE__CAPACITY;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_BYTES_VALUE].arg.length = PROTON_SIGNALS__VALUE_TEST__LIST_BYTES_VALUE__LENGTH;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_BYTES_VALUE].arg.size = 0;

  // Initialise bundle
  PROTON_InitBundle(&test_bundle_handle, PROTON_BUNDLE__VALUE_TEST, test_signal_handles, PROTON_SIGNALS__VALUE_TEST_COUNT);

  // Encode bundle
  int bytes_written = PROTON_Encode(&test_bundle_handle, buffer, 256);

  // More than 0 bytes written if encoding is successful
  EXPECT_GT(bytes_written, 0);
}

TEST(PROTONC_Proton, DecodeId) {
  // Buffer to decode with
  uint8_t buffer[256];

  // Create structs to hold bundle and signals
  proton_bundle_handle_t test_bundle_handle;
  proton_signal_handle_t test_signal_handle;
  PROTON_BUNDLE__value_test_t test_bundle;

  // Initialise bundle with ID 0x123
  PROTON_InitBundle(&test_bundle_handle, 0x123, &test_signal_handle, 0);

  // Encode bundle
  int bytes_written = PROTON_Encode(&test_bundle_handle, buffer, 256);

  // More than 0 bytes written if encoding is successful
  EXPECT_GT(bytes_written, 0);

  uint32_t id = 0;

  // Decoding should succeed
  EXPECT_TRUE(PROTON_DecodeId(&id, buffer, 256));

  // ID should match
  ASSERT_EQ(id, 0x123);
}

TEST(PROTONC_Proton, Decode) {
  // Buffer to encode/decode with
  uint8_t buffer[256];

  // Create structs to hold bundle and signals
  proton_bundle_handle_t test_bundle_handle;
  proton_signal_handle_t test_signal_handles[PROTON_SIGNALS__VALUE_TEST_COUNT];
  PROTON_BUNDLE__value_test_t test_bundle;

  // Create test values to test against
  double double_value = 1.234;
  float float_value = 1.23f;
  int32_t int32_value = -12;
  int64_t int64_value = -125;
  uint32_t uint32_value = 32;
  uint64_t uint64_value = 64;
  bool bool_value = true;
  char string_value[PROTON_SIGNALS__VALUE_TEST__STRING_VALUE__CAPACITY] = "test";
  uint8_t bytes_value[PROTON_SIGNALS__VALUE_TEST__BYTES_VALUE__CAPACITY] = {0, 1, 2, 3};
  double list_double_value[PROTON_SIGNALS__VALUE_TEST__LIST_DOUBLE_VALUE__LENGTH] = {0.1, 0.2};
  float list_float_value[PROTON_SIGNALS__VALUE_TEST__LIST_FLOAT_VALUE__LENGTH] = {0.12f, 0.23f};
  int32_t list_int32_value[PROTON_SIGNALS__VALUE_TEST__LIST_INT32_VALUE__LENGTH] = {1, 2};
  int64_t list_int64_value[PROTON_SIGNALS__VALUE_TEST__LIST_INT64_VALUE__LENGTH] = {3, 4};
  uint32_t list_uint32_value[PROTON_SIGNALS__VALUE_TEST__LIST_UINT32_VALUE__LENGTH] = {5, 6};
  uint64_t list_uint64_value[PROTON_SIGNALS__VALUE_TEST__LIST_UINT64_VALUE__LENGTH] = {7, 8};
  bool list_bool_value[PROTON_SIGNALS__VALUE_TEST__LIST_BOOL_VALUE__LENGTH] = {true, false};
  char list_string_value[PROTON_SIGNALS__VALUE_TEST__LIST_STRING_VALUE__LENGTH][PROTON_SIGNALS__VALUE_TEST__LIST_STRING_VALUE__CAPACITY] = {"test1", "test2"};
  uint8_t list_bytes_value[PROTON_SIGNALS__VALUE_TEST__LIST_BYTES_VALUE__LENGTH][PROTON_SIGNALS__VALUE_TEST__LIST_BYTES_VALUE__CAPACITY] = {{0, 1}, {2, 3}};

  // Copy test values into signals
  test_bundle.double_value = double_value;
  test_bundle.float_value = float_value;
  test_bundle.int32_value = int32_value;
  test_bundle.int64_value = int64_value;
  test_bundle.uint32_value = uint32_value;
  test_bundle.uint64_value = uint64_value;
  test_bundle.bool_value = bool_value;
  strncpy(test_bundle.string_value, string_value, PROTON_SIGNALS__VALUE_TEST__STRING_VALUE__CAPACITY);
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
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__DOUBLE_VALUE].signal.which_signal = proton_Signal_double_value_tag;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__DOUBLE_VALUE].arg.data = &value_test_bundle.double_value;

  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__FLOAT_VALUE].signal.which_signal = proton_Signal_float_value_tag;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__FLOAT_VALUE].arg.data = &value_test_bundle.float_value;

  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__INT32_VALUE].signal.which_signal = proton_Signal_int32_value_tag;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__INT32_VALUE].arg.data = &value_test_bundle.int32_value;

  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__INT64_VALUE].signal.which_signal = proton_Signal_int64_value_tag;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__INT64_VALUE].arg.data = &value_test_bundle.int64_value;

  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__UINT32_VALUE].signal.which_signal = proton_Signal_uint32_value_tag;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__UINT32_VALUE].arg.data = &value_test_bundle.uint32_value;

  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__UINT64_VALUE].signal.which_signal = proton_Signal_uint64_value_tag;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__UINT64_VALUE].arg.data = &value_test_bundle.uint64_value;

  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__BOOL_VALUE].signal.which_signal = proton_Signal_bool_value_tag;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__BOOL_VALUE].arg.data = &value_test_bundle.bool_value;

  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__STRING_VALUE].signal.which_signal = proton_Signal_string_value_tag;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__STRING_VALUE].signal.signal.string_value = &test_signal_handles[PROTON_SIGNALS__VALUE_TEST__STRING_VALUE].arg;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__STRING_VALUE].arg.data = value_test_bundle.string_value;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__STRING_VALUE].arg.capacity = PROTON_SIGNALS__VALUE_TEST__STRING_VALUE__CAPACITY;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__STRING_VALUE].arg.size = 0;

  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__BYTES_VALUE].signal.which_signal = proton_Signal_bytes_value_tag;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__BYTES_VALUE].signal.signal.bytes_value = &test_signal_handles[PROTON_SIGNALS__VALUE_TEST__BYTES_VALUE].arg;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__BYTES_VALUE].arg.data = value_test_bundle.bytes_value;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__BYTES_VALUE].arg.capacity = PROTON_SIGNALS__VALUE_TEST__BYTES_VALUE__CAPACITY;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__BYTES_VALUE].arg.size = 0;

  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_DOUBLE_VALUE].signal.which_signal = proton_Signal_list_double_value_tag;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_DOUBLE_VALUE].signal.signal.list_double_value.doubles = &test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_DOUBLE_VALUE].arg;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_DOUBLE_VALUE].arg.data = value_test_bundle.list_double_value;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_DOUBLE_VALUE].arg.length = PROTON_SIGNALS__VALUE_TEST__LIST_DOUBLE_VALUE__LENGTH;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_DOUBLE_VALUE].arg.size = 0;

  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_FLOAT_VALUE].signal.which_signal = proton_Signal_list_float_value_tag;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_FLOAT_VALUE].signal.signal.list_float_value.floats = &test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_FLOAT_VALUE].arg;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_FLOAT_VALUE].arg.data = value_test_bundle.list_float_value;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_FLOAT_VALUE].arg.length = PROTON_SIGNALS__VALUE_TEST__LIST_FLOAT_VALUE__LENGTH;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_FLOAT_VALUE].arg.size = 0;

  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_INT32_VALUE].signal.which_signal = proton_Signal_list_int32_value_tag;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_INT32_VALUE].signal.signal.list_int32_value.int32s = &test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_INT32_VALUE].arg;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_INT32_VALUE].arg.data = value_test_bundle.list_int32_value;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_INT32_VALUE].arg.length = PROTON_SIGNALS__VALUE_TEST__LIST_INT32_VALUE__LENGTH;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_INT32_VALUE].arg.size = 0;

  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_INT64_VALUE].signal.which_signal = proton_Signal_list_int64_value_tag;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_INT64_VALUE].signal.signal.list_int64_value.int64s = &test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_INT64_VALUE].arg;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_INT64_VALUE].arg.data = value_test_bundle.list_int64_value;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_INT64_VALUE].arg.length = PROTON_SIGNALS__VALUE_TEST__LIST_INT64_VALUE__LENGTH;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_INT64_VALUE].arg.size = 0;

  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_UINT32_VALUE].signal.which_signal = proton_Signal_list_uint32_value_tag;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_UINT32_VALUE].signal.signal.list_uint32_value.uint32s = &test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_UINT32_VALUE].arg;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_UINT32_VALUE].arg.data = value_test_bundle.list_uint32_value;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_UINT32_VALUE].arg.length = PROTON_SIGNALS__VALUE_TEST__LIST_UINT32_VALUE__LENGTH;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_UINT32_VALUE].arg.size = 0;

  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_UINT64_VALUE].signal.which_signal = proton_Signal_list_uint64_value_tag;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_UINT64_VALUE].signal.signal.list_uint64_value.uint64s = &test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_UINT64_VALUE].arg;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_UINT64_VALUE].arg.data = value_test_bundle.list_uint64_value;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_UINT64_VALUE].arg.length = PROTON_SIGNALS__VALUE_TEST__LIST_UINT64_VALUE__LENGTH;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_UINT64_VALUE].arg.size = 0;

  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_BOOL_VALUE].signal.which_signal = proton_Signal_list_bool_value_tag;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_BOOL_VALUE].signal.signal.list_bool_value.bools = &test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_BOOL_VALUE].arg;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_BOOL_VALUE].arg.data = value_test_bundle.list_bool_value;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_BOOL_VALUE].arg.length = PROTON_SIGNALS__VALUE_TEST__LIST_BOOL_VALUE__LENGTH;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_BOOL_VALUE].arg.size = 0;

  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_STRING_VALUE].signal.which_signal = proton_Signal_list_string_value_tag;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_STRING_VALUE].signal.signal.list_string_value.strings = &test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_STRING_VALUE].arg;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_STRING_VALUE].arg.data = value_test_bundle.list_string_value;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_STRING_VALUE].arg.capacity = PROTON_SIGNALS__VALUE_TEST__LIST_STRING_VALUE__CAPACITY;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_STRING_VALUE].arg.length = PROTON_SIGNALS__VALUE_TEST__LIST_STRING_VALUE__LENGTH;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_STRING_VALUE].arg.size = 0;

  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_BYTES_VALUE].signal.which_signal = proton_Signal_list_bytes_value_tag;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_BYTES_VALUE].signal.signal.list_bytes_value.bytes = &test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_BYTES_VALUE].arg;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_BYTES_VALUE].arg.data = value_test_bundle.list_bytes_value;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_BYTES_VALUE].arg.capacity = PROTON_SIGNALS__VALUE_TEST__LIST_BYTES_VALUE__CAPACITY;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_BYTES_VALUE].arg.length = PROTON_SIGNALS__VALUE_TEST__LIST_BYTES_VALUE__LENGTH;
  test_signal_handles[PROTON_SIGNALS__VALUE_TEST__LIST_BYTES_VALUE].arg.size = 0;

  // Initialise bundle
  PROTON_InitBundle(&test_bundle_handle, PROTON_BUNDLE__VALUE_TEST, test_signal_handles, PROTON_SIGNALS__VALUE_TEST_COUNT);

  // Encode bundle
  int bytes_written = PROTON_Encode(&test_bundle_handle, buffer, 256);

  // More than 0 bytes written if encoding is successful
  EXPECT_GT(bytes_written, 0);

  uint32_t id = 0;

  // Decode ID
  EXPECT_TRUE(PROTON_DecodeId(&id, buffer, 256));

  // ID should be the same
  EXPECT_EQ(id, PROTON_BUNDLE__VALUE_TEST);

  // Decode bundle in place
  int bytes_left = PROTON_Decode(&test_bundle_handle, buffer, bytes_written);

  // Check that all bytes were successfully decoded
  EXPECT_EQ(bytes_left, 0);

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
//   ASSERT_EQ(PROTON_SpinOnce(NULL), PROTON_ERR);

//   proton_node_t node = proton_node_default;

//   // Uninitialised node should report an error
//   ASSERT_EQ(PROTON_SpinOnce(&node), PROTON_ERR);

//   // Set node to connected state
//   node.connected = true;

//   proton_transport_read_t read_func = [](uint8_t *buf, size_t len)->size_t { return 1; };
//   proton_receive_t receive_func = [](const uint8_t *buf, size_t len)->bool { return false; };
//   node.transport.read = read_func;

//   // Initialise node without defined buffers
//   PROTON_InitNode(&node, node.transport, receive_func, proton_buffer_default, proton_buffer_default);

//   ASSERT_EQ(PROTON_SpinOnce(&node), PROTON_ERR);

//   uint8_t buffer[256];
//   proton_buffer_t read_buffer = {buffer, sizeof(buffer)};

//   // Initialise node with a read buffer
//   PROTON_InitNode(&node, node.transport, receive_func, read_buffer, proton_buffer_default);

//   ASSERT_EQ(PROTON_SpinOnce(&node), PROTON_READ_ERR);
// }


int main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
