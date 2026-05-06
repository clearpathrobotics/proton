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
 * @author Roni Kreinin (roni.kreinin@rockwellautomation.com)
 */
#include "protoncpp/utils.hpp"

using namespace proton;

TEST(SignalValues, DoubleValue)
{
  std::map<std::string, BundleHandle> bundles;
  bundles = getBundles(CONFIG_FILE);
  auto & bundle = bundles.at("value_test");
  auto & signal = bundle.getSignal("double_value");

  // Check that the default value is correctly set
  AssertSignalValue(signal, default_values::DOUBLE);

  // Set a value
  double value = 1.2345;
  signal.setValue<double>(value);

  // Check that the value is correctly set
  AssertSignalValue(signal, value);
}

TEST(SignalValues, FloatValue)
{
  std::map<std::string, BundleHandle> bundles;
  bundles = getBundles(CONFIG_FILE);
  auto & bundle = bundles.at("value_test");
  auto & signal = bundle.getSignal("float_value");

  // Check that the default value is correctly set
  AssertSignalValue(signal, default_values::FLOAT);

  // Set a value
  float value = 1.2345f;
  signal.setValue<float>(value);

  // Check that the value is correctly set
  AssertSignalValue(signal, value);
}

TEST(SignalValues, Int32Value)
{
  std::map<std::string, BundleHandle> bundles;
  bundles = getBundles(CONFIG_FILE);
  auto & bundle = bundles.at("value_test");
  auto & signal = bundle.getSignal("int32_value");

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

TEST(SignalValues, Int64Value)
{
  std::map<std::string, BundleHandle> bundles;
  bundles = getBundles(CONFIG_FILE);
  auto & bundle = bundles.at("value_test");
  auto & signal = bundle.getSignal("int64_value");

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

TEST(SignalValues, Uint32Value)
{
  std::map<std::string, BundleHandle> bundles;
  bundles = getBundles(CONFIG_FILE);
  auto & bundle = bundles.at("value_test");
  auto & signal = bundle.getSignal("uint32_value");

  // Check that the default value is correctly set
  AssertSignalValue(signal, default_values::UINT32);

  // Set a value
  uint32_t value = 32;
  signal.setValue<uint32_t>(value);

  // Check that the value is correctly set
  AssertSignalValue(signal, value);
}

TEST(SignalValues, Uint64Value)
{
  std::map<std::string, BundleHandle> bundles;
  bundles = getBundles(CONFIG_FILE);
  auto & bundle = bundles.at("value_test");
  auto & signal = bundle.getSignal("uint64_value");

  // Check that the default value is correctly set
  AssertSignalValue(signal, default_values::UINT64);

  // Set a value
  uint64_t value = 64;
  signal.setValue<uint64_t>(value);

  // Check that the value is correctly set
  AssertSignalValue(signal, value);
}

TEST(SignalValues, BoolValue)
{
  std::map<std::string, BundleHandle> bundles;
  bundles = getBundles(CONFIG_FILE);
  auto & bundle = bundles.at("value_test");
  auto & signal = bundle.getSignal("bool_value");

  // Check that the default value is correctly set
  AssertSignalValue(signal, default_values::BOOL);

  // Set a value
  bool value = true;
  signal.setValue<bool>(value);

  // Check that the value is correctly set
  AssertSignalValue(signal, value);
}

TEST(SignalValues, StringValue)
{
  std::map<std::string, BundleHandle> bundles;
  bundles = getBundles(CONFIG_FILE);
  auto & bundle = bundles.at("value_test");
  auto & signal = bundle.getSignal("string_value");

  // Set a value
  std::string value = "test";
  signal.setValue<std::string>(value);

  // Check that the value is correctly set
  AssertSignalValue(signal, value);
}

TEST(SignalValues, BytesValue)
{
  std::map<std::string, BundleHandle> bundles;
  bundles = getBundles(CONFIG_FILE);
  auto & bundle = bundles.at("value_test");
  auto & signal = bundle.getSignal("bytes_value");

  // Set a value
  proton::bytes value = {0, 1, 2, 3};
  signal.setValue<proton::bytes>(value);

  // Check that the value is correctly set
  AssertSignalValue(signal, value);
}

TEST(SignalValues, DefaultValue)
{
  std::map<std::string, BundleHandle> bundles;
  bundles = getBundles(CONFIG_FILE);
  auto & bundle = bundles.at("default_value_test");
  auto & double_signal = bundle.getSignal("default_double");
  auto & string_signal = bundle.getSignal("default_string");
  auto & bytes_signal = bundle.getSignal("default_bytes");

  // Check that the default values are correctly set
  AssertSignalValue(double_signal, 3.14159);
  AssertSignalValue(string_signal, std::string("foo"));
  proton::bytes expected_bytes = {0, 1, 2};
  AssertSignalValue(bytes_signal, expected_bytes);
}

int main(int argc, char ** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
