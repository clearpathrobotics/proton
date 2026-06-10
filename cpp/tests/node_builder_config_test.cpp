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
#include <functional>
#include <string>

#include "protoncpp/node_builder/config.hpp"

using namespace proton::node_builder;

void expect_throw_with_message(const std::string & filepath, const std::string & expected_msg)
{
  try
  {
    Config config(filepath);
    FAIL() << "Expected exception was not thrown.";
  }
  catch (const NodeBuilderException & e)
  {
    EXPECT_EQ(std::string(e.what()), expected_msg);
  }
  catch (...)
  {
    FAIL() << "An unknown exception type was thrown.";
  }
};

TEST(ConfigTest, HappyPathTest)
{
  Config config("test_configs/test.yaml");
  EXPECT_EQ(config.bundles.size(), 7);
  EXPECT_EQ(config.nodes.size(), 3);
  EXPECT_EQ(config.connections.size(), 2);
  EXPECT_EQ(config.signals.size(), 16);
}

TEST(SignalConfigTest, InvalidBytesCapacity)
{
  expect_throw_with_message(
    "test_configs/signal_invalid_bytes_capacity.yaml",
    "Error in signal: invalid_capacity_bytes: Capacity 1 is shorter than default value: 3");
}

TEST(SignalConfigTest, InvalidStringCapacity)
{
  expect_throw_with_message(
    "test_configs/signal_invalid_string_capacity.yaml",
    "Error in signal: invalid_capacity_string: Capacity 1 is shorter than default value: 3 + 1");
}

TEST(SignalConfigTest, InvalidId)
{
  EXPECT_THROW(Config("test_configs/signal_invalid_id.yaml"), std::runtime_error);
}

TEST(SignalConfigTest, InvalidTypeString)
{
  expect_throw_with_message(
    "test_configs/signal_invalid_type_string.yaml",
    "Signal value type not_a_real_type_string is not a valid type");
}

TEST(SignalConfigTest, ListForNonListType)
{
  expect_throw_with_message(
    "test_configs/signal_list_for_non_list_type.yaml",
    "Error in signal list_floats: only bytes type signals can have a sequence as a default value");
}

TEST(SignalConfigTest, SignalNoBytesCapacity)
{
  expect_throw_with_message(
    "test_configs/signal_no_bytes_capacity.yaml",
    "Signal invalid_capacity_bytes of type bytes must define a capacity");
}

TEST(SignalConfigTest, SignalNoStringCapacity)
{
  expect_throw_with_message(
    "test_configs/signal_no_string_capacity.yaml",
    "Signal invalid_capacity_string of type string must define a capacity");
}

TEST(BundleConfigTest, NoId)
{
  expect_throw_with_message(
    "test_configs/bundle_no_id.yaml", "Bundle must contain name, ID, producers and consumers");
}

TEST(BundleConfigTest, NoName)
{
  expect_throw_with_message(
    "test_configs/bundle_no_name.yaml", "Bundle must contain name, ID, producers and consumers");
}

TEST(BundleConfigTest, NoProducers)
{
  expect_throw_with_message(
    "test_configs/bundle_no_producers.yaml",
    "Bundle must contain name, ID, producers and consumers");
}

TEST(BundleConfigTest, NoConsumers)
{
  expect_throw_with_message(
    "test_configs/bundle_no_consumers.yaml",
    "Bundle must contain name, ID, producers and consumers");
}

TEST(BundleConfigTest, InvalidProducer)
{
  expect_throw_with_message(
    "test_configs/bundle_invalid_producer.yaml",
    "Bundle value_test must have a sequence of producers");
}

TEST(BundleConfigTest, InvalidConsumer)
{
  expect_throw_with_message(
    "test_configs/bundle_invalid_consumer.yaml",
    "Bundle value_test must have a sequence of consumers");
}

TEST(BundleConfigTest, SignalsNotList)
{
  expect_throw_with_message(
    "test_configs/bundle_signals_not_list.yaml", "Bundle value_test signals are not a list");
}

TEST(BundleConfigTest, ValidWithNoSignals)
{
  Config config("test_configs/bundle_valid_with_no_signals.yaml");
  EXPECT_EQ(config.bundles.size(), 1);
  EXPECT_TRUE(config.bundles[0].signals.empty());
}

TEST(BundleConfigTest, WithNoProducer)
{
  Config config("test_configs/bundle_with_no_producer.yaml");
  EXPECT_EQ(config.bundles.size(), 1);
  EXPECT_TRUE(config.bundles[0].producers.empty());
}

TEST(BundleConfigTest, WithNoConsumer)
{
  Config config("test_configs/bundle_with_no_consumer.yaml");
  EXPECT_EQ(config.bundles.size(), 1);
  EXPECT_TRUE(config.bundles[0].consumers.empty());
}

TEST(ConnectionConfigTest, NoFirstElement)
{
  expect_throw_with_message(
    "test_configs/connection_no_first_element.yaml",
    "Node connection requires first and second element");
}

TEST(ConnectionConfigTest, NoSecondElement)
{
  expect_throw_with_message(
    "test_configs/connection_no_second_element.yaml",
    "Node connection requires first and second element");
}

TEST(ConnectionConfigTest, NoId)
{
  expect_throw_with_message(
    "test_configs/connections_no_id.yaml", "Connection element requires a node ID and name");
}

TEST(ConnectionConfigTest, NoNodeName)
{
  expect_throw_with_message(
    "test_configs/connections_no_node_name.yaml", "Connection element requires a node ID and name");
}

TEST(EndpointConfigTest, NoId)
{
  expect_throw_with_message(
    "test_configs/endpoint_no_id.yaml", "Node producer endpoint requires an ID");
}

TEST(EndpointConfigTest, NoType)
{
  expect_throw_with_message(
    "test_configs/endpoint_no_type.yaml", "Endpoint must define id and type");
}

TEST(EndpointConfigTest, InvalidType)
{
  expect_throw_with_message(
    "test_configs/endpoint_invalid_type.yaml", "Endpoint type not_a_real_type is not a valid type");
}

TEST(EndpointConfigTest, Udp4MissingIp)
{
  expect_throw_with_message(
    "test_configs/endpoint_udp4_missing_ip.yaml", "udp4 endpoints require ip and port");
}

TEST(EndpointConfigTest, Udp4MissingPort)
{
  expect_throw_with_message(
    "test_configs/endpoint_udp4_missing_port.yaml", "udp4 endpoints require ip and port");
}

TEST(EndpointConfigTest, SerialNoDevice)
{
  expect_throw_with_message(
    "test_configs/endpoint_serial_no_device.yaml", "serial endpoints require a device");
}

TEST(NodeConfigTest, NoId)
{
  expect_throw_with_message(
    "test_configs/node_no_id.yaml", "Node name, ID, and endpoints must be defined");
}

TEST(NodeConfigTest, NoName)
{
  expect_throw_with_message(
    "test_configs/node_no_name.yaml", "Node name, ID, and endpoints must be defined");
}

TEST(NodeConfigTest, NoEndpoints)
{
  expect_throw_with_message(
    "test_configs/node_no_endpoints.yaml", "Node name, ID, and endpoints must be defined");
}

TEST(NodeConfigTest, EndpointsNotSequence)
{
  expect_throw_with_message(
    "test_configs/node_endpoints_not_sequence.yaml",
    "Node producer requires a sequence of endpoints");
}

int main(int argc, char ** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
