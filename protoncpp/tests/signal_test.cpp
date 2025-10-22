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

#include "utils.hpp"

using namespace proton;

std::map<std::string, BundleHandle> bundles;

TEST(SignalValues, DoubleValue) {
  auto& bundle = bundles.at("value_test");
  auto& signal = bundle.getSignal("double_value");

  // Check that the default value is correctly set
  AssertSignalValue(signal, default_values::DOUBLE);

  // Set a value
  double value = 1.2345;
  signal.setValue<double>(value);

  // Check that the value is correctly set
  AssertSignalValue(signal, value);
}

TEST(SignalValues, FloatValue) {
  auto& bundle = bundles.at("value_test");
  auto& signal = bundle.getSignal("float_value");

  // Check that the default value is correctly set
  AssertSignalValue(signal, default_values::FLOAT);

  // Set a value
  float value = 1.2345f;
  signal.setValue<float>(value);

  // Check that the value is correctly set
  AssertSignalValue(signal, value);
}

TEST(SignalValues, Int32Value) {
  auto& bundle = bundles.at("value_test");
  auto& signal = bundle.getSignal("int32_value");

  // Check that the default value is correctly set
  AssertSignalValue(signal, default_values::INT32);

  // Set a positive value
  int32_t value = 32;
  signal.setValue<int32_t>(value);

  // Check that the value is correctly set
  ASSERT_EQ(signal.getValue<int32_t>(), value);

  // Set a negative value
  value = -32;
  signal.setValue<int32_t>(value);

  // Check that the value is correctly set
  AssertSignalValue(signal, value);
}

TEST(SignalValues, Int64Value) {
  auto& bundle = bundles.at("value_test");
  auto& signal = bundle.getSignal("int64_value");

  // Check that the default value is correctly set
  AssertSignalValue(signal, default_values::INT64);

  // Set a positive value
  int64_t value = 64;
  signal.setValue<int64_t>(value);

  // Check that the value is correctly set
  ASSERT_EQ(signal.getValue<int64_t>(), value);

  // Set a negative value
  value = -64;
  signal.setValue<int64_t>(value);

  // Check that the value is correctly set
  AssertSignalValue(signal, value);
}

TEST(SignalValues, Uint32Value) {
  auto& bundle = bundles.at("value_test");
  auto& signal = bundle.getSignal("uint32_value");

  // Check that the default value is correctly set
  AssertSignalValue(signal, default_values::UINT32);

  // Set a value
  uint32_t value = 32;
  signal.setValue<uint32_t>(value);

  // Check that the value is correctly set
  AssertSignalValue(signal, value);
}

TEST(SignalValues, Uint64Value) {
  auto& bundle = bundles.at("value_test");
  auto& signal = bundle.getSignal("uint64_value");

  // Check that the default value is correctly set
  AssertSignalValue(signal, default_values::UINT64);

  // Set a value
  uint64_t value = 64;
  signal.setValue<uint64_t>(value);

  // Check that the value is correctly set
  AssertSignalValue(signal, value);
}

TEST(SignalValues, BoolValue) {
  auto& bundle = bundles.at("value_test");
  auto& signal = bundle.getSignal("bool_value");

  // Check that the default value is correctly set
  AssertSignalValue(signal, default_values::BOOL);

  // Set a value
  bool value = true;
  signal.setValue<bool>(value);

  // Check that the value is correctly set
  AssertSignalValue(signal, value);
}

TEST(SignalValues, StringValue) {
  auto& bundle = bundles.at("value_test");
  auto& signal = bundle.getSignal("string_value");

  // Set a value
  std::string value = "test";
  signal.setValue<std::string>(value);

  // Check that the value is correctly set
  AssertSignalValue(signal, value);
}

TEST(SignalValues, BytesValue) {
  auto& bundle = bundles.at("value_test");
  auto& signal = bundle.getSignal("bytes_value");

  // Set a value
  proton::bytes value = {0, 1, 2, 3};
  signal.setValue<proton::bytes>(value);

  // Check that the value is correctly set
  AssertSignalValue(signal, value);
}

TEST(SignalValues, ListDoubleValue) {
  auto& bundle = bundles.at("value_test");
  auto& signal = bundle.getSignal("list_double_value");

  // Set a value
  proton::list_double value = {10.0, -12.34};
  signal.setValue<proton::list_double>(value);

  // Check that the value is correctly set
  AssertSignalValue(signal, value);
}

TEST(SignalValues, ListFloatValue) {
  auto& bundle = bundles.at("value_test");
  auto& signal = bundle.getSignal("list_float_value");

  // Set a value
  proton::list_float value = {12.3f, -12.345f};
  signal.setValue<proton::list_float>(value);

  // Check that the value is correctly set
  AssertSignalValue(signal, value);
}

TEST(SignalValues, ListInt32Value) {
  auto& bundle = bundles.at("value_test");
  auto& signal = bundle.getSignal("list_int32_value");

  // Set a value
  proton::list_int32 value = {-1, 2};
  signal.setValue<proton::list_int32>(value);

  // Check that the value is correctly set
  AssertSignalValue(signal, value);
}

TEST(SignalValues, ListInt64Value) {
  auto& bundle = bundles.at("value_test");
  auto& signal = bundle.getSignal("list_int64_value");

  // Set a value
  proton::list_int64 value = {-1, 2};
  signal.setValue<proton::list_int64>(value);

  // Check that the value is correctly set
  AssertSignalValue(signal, value);
}

TEST(SignalValues, ListUint32Value) {
  auto& bundle = bundles.at("value_test");
  auto& signal = bundle.getSignal("list_uint32_value");

  // Set a value
  proton::list_uint32 value = {1, 2};
  signal.setValue<proton::list_uint32>(value);

  // Check that the value is correctly set
  AssertSignalValue(signal, value);
}

TEST(SignalValues, ListUint64Value) {
  auto& bundle = bundles.at("value_test");
  auto& signal = bundle.getSignal("list_uint64_value");

  // Set a value
  proton::list_uint64 value = {1, 2};
  signal.setValue<proton::list_uint64>(value);

  // Check that the value is correctly set
  AssertSignalValue(signal, value);
}

TEST(SignalValues, ListBoolValue) {
  auto& bundle = bundles.at("value_test");
  auto& signal = bundle.getSignal("list_bool_value");

  // Set a value
  proton::list_bool value = {true, false};
  signal.setValue<proton::list_bool>(value);

  // Check that the value is correctly set
  AssertSignalValue(signal, value);
}

TEST(SignalValues, ListStringValue) {
  auto& bundle = bundles.at("value_test");
  auto& signal = bundle.getSignal("list_string_value");

  // Set a value
  proton::list_string value = {"string1", "string2"};
  signal.setValue<proton::list_string>(value);

  // Check that the value is correctly set
  AssertSignalValue(signal, value);
}

TEST(SignalValues, ListBytesValue) {
  auto& bundle = bundles.at("value_test");
  auto& signal = bundle.getSignal("list_bytes_value");

  // Set a value
  proton::list_bytes value = {{1, 2}, {3, 4}};
  signal.setValue<proton::list_bytes>(value);

  // Check that the value is correctly set
  AssertSignalValue(signal, value);
}

int main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);
  bundles = getBundles(CONFIG_FILE);
  return RUN_ALL_TESTS();
}
