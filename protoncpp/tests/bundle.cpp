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

TEST(SignalValues, DoubleValue) {
  auto& bundle = bundles.at("value_test");
  auto& signal = bundle.getSignal("double_value");

  // Check that the default value is correctly set
  ASSERT_EQ(signal.getValue<double>(), default_values::DOUBLE);

  // Set a value
  double value = 1.2345;
  signal.setValue<double>(value);

  // Check that the value is correctly set
  ASSERT_EQ(signal.getValue<double>(), value);
}

TEST(SignalValues, FloatValue) {
  auto& bundle = bundles.at("value_test");
  auto& signal = bundle.getSignal("float_value");

  // Check that the default value is correctly set
  ASSERT_EQ(signal.getValue<float>(), default_values::FLOAT);

  // Set a value
  float value = 1.2345f;
  signal.setValue<float>(value);

  // Check that the value is correctly set
  ASSERT_EQ(signal.getValue<float>(), value);
}

TEST(SignalValues, Int32Value) {
  auto& bundle = bundles.at("value_test");
  auto& signal = bundle.getSignal("int32_value");

  // Check that the default value is correctly set
  ASSERT_EQ(signal.getValue<int32_t>(), default_values::INT32);

  // Set a positive value
  int32_t value = 32;
  signal.setValue<int32_t>(value);

  // Check that the value is correctly set
  ASSERT_EQ(signal.getValue<int32_t>(), value);

  // Set a negative value
  value = -32;
  signal.setValue<int32_t>(value);

  // Check that the value is correctly set
  ASSERT_EQ(signal.getValue<int32_t>(), value);
}

TEST(SignalValues, Int64Value) {
  auto& bundle = bundles.at("value_test");
  auto& signal = bundle.getSignal("int64_value");

  // Check that the default value is correctly set
  ASSERT_EQ(signal.getValue<int64_t>(), default_values::INT64);

  // Set a positive value
  int64_t value = 64;
  signal.setValue<int64_t>(value);

  // Check that the value is correctly set
  ASSERT_EQ(signal.getValue<int64_t>(), value);

  // Set a negative value
  value = -64;
  signal.setValue<int64_t>(value);

  // Check that the value is correctly set
  ASSERT_EQ(signal.getValue<int64_t>(), value);
}

TEST(SignalValues, Uint32Value) {
  auto& bundle = bundles.at("value_test");
  auto& signal = bundle.getSignal("uint32_value");

  // Check that the default value is correctly set
  ASSERT_EQ(signal.getValue<uint32_t>(), default_values::UINT32);

  // Set a value
  uint32_t value = 32;
  signal.setValue<uint32_t>(value);

  // Check that the value is correctly set
  ASSERT_EQ(signal.getValue<uint32_t>(), value);
}

TEST(SignalValues, Uint64Value) {
  auto& bundle = bundles.at("value_test");
  auto& signal = bundle.getSignal("uint64_value");

  // Check that the default value is correctly set
  ASSERT_EQ(signal.getValue<uint64_t>(), default_values::UINT64);

  // Set a value
  uint64_t value = 64;
  signal.setValue<uint64_t>(value);

  // Check that the value is correctly set
  ASSERT_EQ(signal.getValue<uint64_t>(), value);
}

TEST(SignalValues, BoolValue) {
  auto& bundle = bundles.at("value_test");
  auto& signal = bundle.getSignal("bool_value");

  // Check that the default value is correctly set
  ASSERT_EQ(signal.getValue<bool>(), default_values::BOOL);

  // Set a value
  bool value = true;
  signal.setValue<bool>(value);

  // Check that the value is correctly set
  ASSERT_EQ(signal.getValue<bool>(), value);
}

TEST(SignalValues, StringValue) {
  auto& bundle = bundles.at("value_test");
  auto& signal = bundle.getSignal("string_value");

  // Set a value
  std::string value = "test";
  signal.setValue<std::string>(value);

  // Check that the value is correctly set
  ASSERT_STREQ(signal.getValue<std::string>().c_str(), value.c_str());
}

TEST(SignalValues, BytesValue) {
  auto& bundle = bundles.at("value_test");
  auto& signal = bundle.getSignal("bytes_value");

  // Set a value
  proton::bytes value = {0, 1, 2, 3};
  signal.setValue<proton::bytes>(value);

  proton::bytes signal_value = signal.getValue<proton::bytes>();

  // Check that the value is correctly set
  for (int i = 0; i < value.size(); i++)
  {
    ASSERT_EQ(value[i], signal_value[i]);
  }
}

int main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);
  InitProton();
  return RUN_ALL_TESTS();
}
