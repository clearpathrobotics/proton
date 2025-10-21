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

#include <gtest/gtest.h>
#include "protoncpp/proton.hpp"

using namespace proton;

Config config;
std::map<std::string, BundleHandle> bundles;

void InitProton()
{
  config = Config(CONFIG_FILE);
  auto bundle_configs = config.getBundles();
  for (auto& config: bundle_configs)
  {
    bundles.emplace(config.name, BundleHandle(config));
  }
}

template<typename T>
void AssertSignalValue(SignalHandle& handle, T value)
{
  ASSERT_EQ(handle.getValue<T>(), value);
}

template<>
void AssertSignalValue<std::string>(SignalHandle& handle, std::string value)
{
  ASSERT_STREQ(handle.getValue<std::string>().c_str(), value.c_str());
}

void AssertSignalsEqual(const proton::Signal& first, const proton::Signal& second)
{
  // Make sure they are of the same case
  ASSERT_EQ(first.signal_case(), second.signal_case());

  switch (first.signal_case()) {
  case proton::Signal::SignalCase::kDoubleValue: {
    ASSERT_EQ(first.double_value(), second.double_value());
    break;
  }

  case proton::Signal::SignalCase::kFloatValue: {
    ASSERT_EQ(first.float_value(), second.float_value());
    break;
  }

  case proton::Signal::SignalCase::kInt32Value: {
    ASSERT_EQ(first.int32_value(), second.int32_value());
    break;
  }

  case proton::Signal::SignalCase::kInt64Value: {
    ASSERT_EQ(first.int64_value(), second.int64_value());
    break;
  }

  case proton::Signal::SignalCase::kUint32Value: {
    ASSERT_EQ(first.uint32_value(), second.uint32_value());
    break;
  }

  case proton::Signal::SignalCase::kUint64Value: {
    ASSERT_EQ(first.uint64_value(), second.uint64_value());
    break;
  }

  case proton::Signal::SignalCase::kBoolValue: {
    ASSERT_EQ(first.bool_value(), second.bool_value());
    break;
  }

  case proton::Signal::SignalCase::kStringValue: {
    ASSERT_STREQ(first.string_value().c_str(), second.string_value().c_str());
    break;
  }

  case proton::Signal::SignalCase::kBytesValue: {
    ASSERT_STREQ(first.bytes_value().c_str(), second.bytes_value().c_str());
    break;
  }

  // case proton::Signal::SignalCase::kListDoubleValue: {
  //   ASSERT_EQ(first.list_double_value(), second.list_double_value());
  //   break;
  // }

  // case proton::Signal::SignalCase::kListFloatValue: {
  //   ASSERT_EQ(first.list_float_value(), second.list_float_value());
  //   break;
  // }

  // case proton::Signal::SignalCase::kListInt32Value: {
  //   ASSERT_EQ(first.list_int32_value(), second.list_int32_value());
  //   break;
  // }

  // case proton::Signal::SignalCase::kListInt64Value: {
  //   ASSERT_EQ(first.list_int64_value(), second.list_int64_value());
  //   break;
  // }

  // case proton::Signal::SignalCase::kListUint32Value: {
  //   ASSERT_EQ(first.list_uint32_value(), second.list_uint32_value());
  //   break;
  // }

  // case proton::Signal::SignalCase::kListUint64Value: {
  //   ASSERT_EQ(first.list_uint64_value(), second.list_uint64_value());
  //   break;
  // }

  // case proton::Signal::SignalCase::kListBoolValue: {
  //   ASSERT_EQ(first.list_bool_value(), second.list_bool_value());
  //   break;
  // }

  // case proton::Signal::SignalCase::kListStringValue: {
  //   ASSERT_EQ(first.list_string_value(), second.list_string_value());
  //   break;
  // }

  // case proton::Signal::SignalCase::kListBytesValue: {
  //   ASSERT_EQ(first.list_bytes_value(), second.list_bytes_value());
  //   break;
  // }

  default:
    break;
  }
}

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

  proton::bytes signal_value = signal.getValue<proton::bytes>();

  // Check that the value is correctly set
  AssertSignalValue(signal, value);
}

TEST(BundleTests, Serialize)
{
  auto& handle = bundles.at("value_test");
  auto bundle = handle.getBundlePtr();

  // Serialize bundle to string
  std::string serialized = bundle->SerializeAsString();

  // Create copy of bundle by parsing serialized string
  auto deserialized_bundle = Bundle();
  deserialized_bundle.ParseFromString(serialized);

  // Check that the IDs match
  ASSERT_EQ(bundle->id(), deserialized_bundle.id());
  // Check that the number of signals match
  ASSERT_EQ(bundle->signals_size(), deserialized_bundle.signals_size());

  for (int i = 0; i < bundle->signals_size(); i++)
  {
    // Check that each signal equal
    AssertSignalsEqual(bundle->signals(i), deserialized_bundle.signals(i));
  }
}

int main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);
  InitProton();
  return RUN_ALL_TESTS();
}
