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
#include <fstream>
#include <functional>
#include <sstream>
#include <string>

#include "protoncpp/node_builder/config.hpp"

#include <yaml-cpp/yaml.h>
#include <nlohmann/json.hpp>

using namespace proton::node_builder;

void expect_yaml_throw_with_message(const std::string & filepath, const std::string & expected_msg)
{
  try
  {
    Config config = Config::from_yaml(filepath);
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

void expect_json_throw_with_message(const std::string & filepath, const std::string & expected_msg)
{
  try
  {
    Config config = Config::from_json(filepath);
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

TEST(YamlConfigTest, HappyPathTest)
{
  Config config = Config::from_yaml("test_configs/yaml/test.yaml");
  EXPECT_EQ(config.bundles.size(), 7);
  EXPECT_EQ(config.nodes.size(), 3);
  EXPECT_EQ(config.connections.size(), 2);
  EXPECT_EQ(config.signals.size(), 16);

  YAML::Node node = YAML::LoadFile("test_configs/yaml/test.yaml");
  std::stringstream ss;
  ss << node;

  ConfigTree config_tree = ConfigTree::from_yaml_string(ss.str());
  Config config_2(config_tree);

  EXPECT_EQ(config.bundles.size(), config_2.bundles.size());
  EXPECT_EQ(config.nodes.size(), config_2.nodes.size());
  EXPECT_EQ(config.connections.size(), config_2.connections.size());
  EXPECT_EQ(config.signals.size(), config_2.signals.size());
}

TEST(YamlSignalConfigTest, InvalidBytesCapacity)
{
  expect_yaml_throw_with_message(
    "test_configs/yaml/signal_invalid_bytes_capacity.yaml",
    "Error in signal: invalid_capacity_bytes: Capacity 1 is shorter than default value: 3");
}

TEST(YamlSignalConfigTest, InvalidStringCapacity)
{
  expect_yaml_throw_with_message(
    "test_configs/yaml/signal_invalid_string_capacity.yaml",
    "Error in signal: invalid_capacity_string: Capacity 1 is shorter than default value: 3 + 1");
}

TEST(YamlSignalConfigTest, InvalidId)
{
  EXPECT_THROW(Config::from_yaml("test_configs/yaml/signal_invalid_id.yaml"), std::runtime_error);
}

TEST(YamlSignalConfigTest, InvalidTypeString)
{
  expect_yaml_throw_with_message(
    "test_configs/yaml/signal_invalid_type_string.yaml",
    "Signal value type not_a_real_type_string is not a valid type");
}

TEST(YamlSignalConfigTest, ListForNonListType)
{
  expect_yaml_throw_with_message(
    "test_configs/yaml/signal_list_for_non_list_type.yaml",
    "Error in signal list_floats: only bytes type signals can have a sequence as a default value");
}

TEST(YamlSignalConfigTest, SignalNoBytesCapacity)
{
  expect_yaml_throw_with_message(
    "test_configs/yaml/signal_no_bytes_capacity.yaml",
    "Signal invalid_capacity_bytes of type bytes must define a capacity");
}

TEST(YamlSignalConfigTest, SignalNoStringCapacity)
{
  expect_yaml_throw_with_message(
    "test_configs/yaml/signal_no_string_capacity.yaml",
    "Signal invalid_capacity_string of type string must define a capacity");
}

TEST(YamlBundleConfigTest, NoId)
{
  expect_yaml_throw_with_message(
    "test_configs/yaml/bundle_no_id.yaml", "Bundle must contain name, ID, producers and consumers");
}

TEST(YamlBundleConfigTest, NoName)
{
  expect_yaml_throw_with_message(
    "test_configs/yaml/bundle_no_name.yaml",
    "Bundle must contain name, ID, producers and consumers");
}

TEST(YamlBundleConfigTest, NoProducers)
{
  expect_yaml_throw_with_message(
    "test_configs/yaml/bundle_no_producers.yaml",
    "Bundle must contain name, ID, producers and consumers");
}

TEST(YamlBundleConfigTest, NoConsumers)
{
  expect_yaml_throw_with_message(
    "test_configs/yaml/bundle_no_consumers.yaml",
    "Bundle must contain name, ID, producers and consumers");
}

TEST(YamlBundleConfigTest, InvalidProducer)
{
  expect_yaml_throw_with_message(
    "test_configs/yaml/bundle_invalid_producer.yaml",
    "Bundle value_test must have a sequence of producers");
}

TEST(YamlBundleConfigTest, InvalidConsumer)
{
  expect_yaml_throw_with_message(
    "test_configs/yaml/bundle_invalid_consumer.yaml",
    "Bundle value_test must have a sequence of consumers");
}

TEST(YamlBundleConfigTest, SignalsNotList)
{
  expect_yaml_throw_with_message(
    "test_configs/yaml/bundle_signals_not_list.yaml", "Bundle value_test signals are not a list");
}

TEST(YamlBundleConfigTest, ValidWithNoSignals)
{
  Config config = Config::from_yaml("test_configs/yaml/bundle_valid_with_no_signals.yaml");
  EXPECT_EQ(config.bundles.size(), 1);
  EXPECT_TRUE(config.bundles[0].signals.empty());
}

TEST(YamlBundleConfigTest, WithNoProducer)
{
  Config config = Config::from_yaml("test_configs/yaml/bundle_with_no_producer.yaml");
  EXPECT_EQ(config.bundles.size(), 1);
  EXPECT_TRUE(config.bundles[0].producers.empty());
}

TEST(YamlBundleConfigTest, WithNoConsumer)
{
  Config config = Config::from_yaml("test_configs/yaml/bundle_with_no_consumer.yaml");
  EXPECT_EQ(config.bundles.size(), 1);
  EXPECT_TRUE(config.bundles[0].consumers.empty());
}

TEST(YamlConnectionConfigTest, NoFirstElement)
{
  expect_yaml_throw_with_message(
    "test_configs/yaml/connection_no_first_element.yaml",
    "Node connection requires first and second element");
}

TEST(YamlConnectionConfigTest, NoSecondElement)
{
  expect_yaml_throw_with_message(
    "test_configs/yaml/connection_no_second_element.yaml",
    "Node connection requires first and second element");
}

TEST(YamlConnectionConfigTest, NoId)
{
  expect_yaml_throw_with_message(
    "test_configs/yaml/connections_no_id.yaml", "Connection element requires a node ID and name");
}

TEST(YamlConnectionConfigTest, NoNodeName)
{
  expect_yaml_throw_with_message(
    "test_configs/yaml/connections_no_node_name.yaml",
    "Connection element requires a node ID and name");
}

TEST(YamlEndpointConfigTest, NoId)
{
  expect_yaml_throw_with_message(
    "test_configs/yaml/endpoint_no_id.yaml", "Node producer endpoint requires an ID");
}

TEST(YamlEndpointConfigTest, NoType)
{
  expect_yaml_throw_with_message(
    "test_configs/yaml/endpoint_no_type.yaml", "Endpoint must define id and type");
}

TEST(YamlEndpointConfigTest, InvalidType)
{
  expect_yaml_throw_with_message(
    "test_configs/yaml/endpoint_invalid_type.yaml",
    "Endpoint type not_a_real_type is not a valid type");
}

TEST(YamlEndpointConfigTest, Udp4MissingIp)
{
  expect_yaml_throw_with_message(
    "test_configs/yaml/endpoint_udp4_missing_ip.yaml", "udp4 endpoints require ip and port");
}

TEST(YamlEndpointConfigTest, Udp4MissingPort)
{
  expect_yaml_throw_with_message(
    "test_configs/yaml/endpoint_udp4_missing_port.yaml", "udp4 endpoints require ip and port");
}

TEST(YamlEndpointConfigTest, SerialNoDevice)
{
  expect_yaml_throw_with_message(
    "test_configs/yaml/endpoint_serial_no_device.yaml", "serial endpoints require a device");
}

TEST(YamlNodeConfigTest, NoId)
{
  expect_yaml_throw_with_message(
    "test_configs/yaml/node_no_id.yaml", "Node name, ID, and endpoints must be defined");
}

TEST(YamlNodeConfigTest, NoName)
{
  expect_yaml_throw_with_message(
    "test_configs/yaml/node_no_name.yaml", "Node name, ID, and endpoints must be defined");
}

TEST(YamlNodeConfigTest, NoEndpoints)
{
  expect_yaml_throw_with_message(
    "test_configs/yaml/node_no_endpoints.yaml", "Node name, ID, and endpoints must be defined");
}

TEST(YamlNodeConfigTest, EndpointsNotSequence)
{
  expect_yaml_throw_with_message(
    "test_configs/yaml/node_endpoints_not_sequence.yaml",
    "Node producer requires a sequence of endpoints");
}

TEST(JsonConfigTest, HappyPathTest)
{
  Config config = Config::from_json("test_configs/json/test.json");
  EXPECT_EQ(config.bundles.size(), 7);
  EXPECT_EQ(config.nodes.size(), 3);
  EXPECT_EQ(config.connections.size(), 2);
  EXPECT_EQ(config.signals.size(), 16);

  std::ifstream f("test_configs/json/test.json");
  std::stringstream ss;
  ss << f.rdbuf();
  ConfigTree config_tree = ConfigTree::from_json_string(ss.str());
  Config config_2(config_tree);

  EXPECT_EQ(config.bundles.size(), config_2.bundles.size());
  EXPECT_EQ(config.nodes.size(), config_2.nodes.size());
  EXPECT_EQ(config.connections.size(), config_2.connections.size());
  EXPECT_EQ(config.signals.size(), config_2.signals.size());
}

TEST(JsonSignalConfigTest, InvalidBytesCapacity)
{
  expect_json_throw_with_message(
    "test_configs/json/signal_invalid_bytes_capacity.json",
    "Error in signal: invalid_capacity_bytes: Capacity 1 is shorter than default value: 3");
}

TEST(JsonSignalConfigTest, InvalidStringCapacity)
{
  expect_json_throw_with_message(
    "test_configs/json/signal_invalid_string_capacity.json",
    "Error in signal: invalid_capacity_string: Capacity 1 is shorter than default value: 3 + 1");
}

TEST(JsonSignalConfigTest, InvalidId)
{
  EXPECT_THROW(Config::from_json("test_configs/json/signal_invalid_id.json"), std::runtime_error);
}

TEST(JsonSignalConfigTest, InvalidTypeString)
{
  expect_json_throw_with_message(
    "test_configs/json/signal_invalid_type_string.json",
    "Signal value type not_a_real_type_string is not a valid type");
}

TEST(JsonSignalConfigTest, ListForNonListType)
{
  expect_json_throw_with_message(
    "test_configs/json/signal_list_for_non_list_type.json",
    "Error in signal list_floats: only bytes type signals can have a sequence as a default value");
}

TEST(JsonSignalConfigTest, SignalNoBytesCapacity)
{
  expect_json_throw_with_message(
    "test_configs/json/signal_no_bytes_capacity.json",
    "Signal invalid_capacity_bytes of type bytes must define a capacity");
}

TEST(JsonSignalConfigTest, SignalNoStringCapacity)
{
  expect_json_throw_with_message(
    "test_configs/json/signal_no_string_capacity.json",
    "Signal invalid_capacity_string of type string must define a capacity");
}

TEST(JsonBundleConfigTest, NoId)
{
  expect_json_throw_with_message(
    "test_configs/json/bundle_no_id.json", "Bundle must contain name, ID, producers and consumers");
}

TEST(JsonBundleConfigTest, NoName)
{
  expect_json_throw_with_message(
    "test_configs/json/bundle_no_name.json",
    "Bundle must contain name, ID, producers and consumers");
}

TEST(JsonBundleConfigTest, NoProducers)
{
  expect_json_throw_with_message(
    "test_configs/json/bundle_no_producers.json",
    "Bundle must contain name, ID, producers and consumers");
}

TEST(JsonBundleConfigTest, NoConsumers)
{
  expect_json_throw_with_message(
    "test_configs/json/bundle_no_consumers.json",
    "Bundle must contain name, ID, producers and consumers");
}

TEST(JsonBundleConfigTest, InvalidProducer)
{
  expect_json_throw_with_message(
    "test_configs/json/bundle_invalid_producer.json",
    "Bundle value_test must have a sequence of producers");
}

TEST(JsonBundleConfigTest, InvalidConsumer)
{
  expect_json_throw_with_message(
    "test_configs/json/bundle_invalid_consumer.json",
    "Bundle value_test must have a sequence of consumers");
}

TEST(JsonBundleConfigTest, SignalsNotList)
{
  expect_json_throw_with_message(
    "test_configs/json/bundle_signals_not_list.json", "Bundle value_test signals are not a list");
}

TEST(JsonBundleConfigTest, ValidWithNoSignals)
{
  Config config = Config::from_json("test_configs/json/bundle_valid_with_no_signals.json");
  EXPECT_EQ(config.bundles.size(), 1);
  EXPECT_TRUE(config.bundles[0].signals.empty());
}

TEST(JsonBundleConfigTest, WithNoProducer)
{
  Config config = Config::from_json("test_configs/json/bundle_with_no_producer.json");
  EXPECT_EQ(config.bundles.size(), 1);
  EXPECT_TRUE(config.bundles[0].producers.empty());
}

TEST(JsonBundleConfigTest, WithNoConsumer)
{
  Config config = Config::from_json("test_configs/json/bundle_with_no_consumer.json");
  EXPECT_EQ(config.bundles.size(), 1);
  EXPECT_TRUE(config.bundles[0].consumers.empty());
}

TEST(JsonConnectionConfigTest, NoFirstElement)
{
  expect_yaml_throw_with_message(
    "test_configs/json/connection_no_first_element.json",
    "Node connection requires first and second element");
}

TEST(JsonConnectionConfigTest, NoSecondElement)
{
  expect_yaml_throw_with_message(
    "test_configs/json/connection_no_second_element.json",
    "Node connection requires first and second element");
}

TEST(JsonConnectionConfigTest, NoId)
{
  expect_yaml_throw_with_message(
    "test_configs/json/connections_no_id.json", "Connection element requires a node ID and name");
}

TEST(JsonConnectionConfigTest, NoNodeName)
{
  expect_yaml_throw_with_message(
    "test_configs/json/connections_no_node_name.json",
    "Connection element requires a node ID and name");
}

TEST(JsonEndpointConfigTest, NoId)
{
  expect_yaml_throw_with_message(
    "test_configs/json/endpoint_no_id.json", "Node producer endpoint requires an ID");
}

TEST(JsonEndpointConfigTest, NoType)
{
  expect_yaml_throw_with_message(
    "test_configs/json/endpoint_no_type.json", "Endpoint must define id and type");
}

TEST(JsonEndpointConfigTest, InvalidType)
{
  expect_yaml_throw_with_message(
    "test_configs/json/endpoint_invalid_type.json",
    "Endpoint type not_a_real_type is not a valid type");
}

TEST(JsonEndpointConfigTest, Udp4MissingIp)
{
  expect_yaml_throw_with_message(
    "test_configs/json/endpoint_udp4_missing_ip.json", "udp4 endpoints require ip and port");
}

TEST(JsonEndpointConfigTest, Udp4MissingPort)
{
  expect_yaml_throw_with_message(
    "test_configs/json/endpoint_udp4_missing_port.json", "udp4 endpoints require ip and port");
}

TEST(JsonEndpointConfigTest, SerialNoDevice)
{
  expect_yaml_throw_with_message(
    "test_configs/json/endpoint_serial_no_device.json", "serial endpoints require a device");
}

TEST(JsonNodeConfigTest, NoId)
{
  expect_yaml_throw_with_message(
    "test_configs/json/node_no_id.json", "Node name, ID, and endpoints must be defined");
}

TEST(JsonNodeConfigTest, NoName)
{
  expect_yaml_throw_with_message(
    "test_configs/json/node_no_name.json", "Node name, ID, and endpoints must be defined");
}

TEST(JsonNodeConfigTest, NoEndpoints)
{
  expect_yaml_throw_with_message(
    "test_configs/json/node_no_endpoints.json", "Node name, ID, and endpoints must be defined");
}

TEST(JsonNodeConfigTest, EndpointsNotSequence)
{
  expect_yaml_throw_with_message(
    "test_configs/json/node_endpoints_not_sequence.json",
    "Node producer requires a sequence of endpoints");
}

int main(int argc, char ** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
