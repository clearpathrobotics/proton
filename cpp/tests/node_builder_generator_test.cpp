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
#include <sstream>
#include <string>

#include "protoncpp/node_builder/generator.hpp"

using namespace proton::node_builder;

// Helper to create a minimal valid config for testing
Config create_base_config()
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

  NodeConfig node_c;
  node_c.name = "node_c";
  node_c.id = 3;
  node_c.endpoints[0] = EndpointConfig{0, "udp4", "", "192.168.1.3", 5000};

  config.nodes["node_a"] = node_a;
  config.nodes["node_b"] = node_b;
  config.nodes["node_c"] = node_c;

  // Add connections: a <-> b, b <-> c
  ConnectionConfig conn_ab;
  conn_ab.first = {0, "node_a"};
  conn_ab.second = {0, "node_b"};
  config.connections.push_back(conn_ab);

  ConnectionConfig conn_bc;
  conn_bc.first = {0, "node_b"};
  conn_bc.second = {0, "node_c"};
  config.connections.push_back(conn_bc);

  // Add signals
  SignalConfig signal1;
  signal1.name = "signal_1";
  signal1.id = 100;
  signal1.type_string = "int32";
  signal1.capacity = 0;
  signal1.has_default_value = false;
  config.signals.push_back(signal1);

  SignalConfig signal2;
  signal2.name = "signal_2";
  signal2.id = 101;
  signal2.type_string = "double";
  signal2.capacity = 0;
  signal2.has_default_value = false;
  config.signals.push_back(signal2);

  SignalConfig signal3;
  signal3.name = "signal_3";
  signal3.id = 102;
  signal3.type_string = "bool";
  signal3.capacity = 0;
  signal3.has_default_value = false;
  config.signals.push_back(signal3);

  // Add bundles
  BundleConfig bundle1;
  bundle1.name = "bundle_ab";
  bundle1.id = 10;
  bundle1.period_ms = 100;
  bundle1.producers = {"node_a"};
  bundle1.consumers = {"node_b"};
  bundle1.signals = {100, 101};
  config.bundles.push_back(bundle1);

  BundleConfig bundle2;
  bundle2.name = "bundle_bc";
  bundle2.id = 11;
  bundle2.period_ms = 200;
  bundle2.producers = {"node_b"};
  bundle2.consumers = {"node_c"};
  bundle2.signals = {102};
  config.bundles.push_back(bundle2);

  return config;
}

// ============================================================================
// find_duplicates tests
// ============================================================================

TEST(FindDuplicates, NoDuplicatesDoesNotThrow)
{
  std::vector<uint32_t> ids = {1, 2, 3, 4, 5};
  EXPECT_NO_THROW(find_duplicates<uint32_t>(ids, "test IDs"));
}

TEST(FindDuplicates, EmptyListDoesNotThrow)
{
  std::vector<uint32_t> ids = {};
  EXPECT_NO_THROW(find_duplicates<uint32_t>(ids, "test IDs"));
}

TEST(FindDuplicates, SingleElementDoesNotThrow)
{
  std::vector<uint32_t> ids = {42};
  EXPECT_NO_THROW(find_duplicates<uint32_t>(ids, "test IDs"));
}

TEST(FindDuplicates, DuplicateIntegersThrows)
{
  std::vector<uint32_t> ids = {1, 2, 3, 2, 4};
  EXPECT_THROW(find_duplicates<uint32_t>(ids, "test IDs"), NodeBuilderException);
}

TEST(FindDuplicates, MultipleDuplicatesThrows)
{
  std::vector<uint32_t> ids = {1, 2, 1, 3, 2, 4};
  EXPECT_THROW(find_duplicates<uint32_t>(ids, "test IDs"), NodeBuilderException);
}

TEST(FindDuplicates, DuplicateStringsThrows)
{
  std::vector<std::string> names = {"alpha", "beta", "alpha"};
  EXPECT_THROW(find_duplicates<std::string>(names, "test names"), NodeBuilderException);
}

TEST(FindDuplicates, NoDuplicateStringsDoesNotThrow)
{
  std::vector<std::string> names = {"alpha", "beta", "gamma"};
  EXPECT_NO_THROW(find_duplicates<std::string>(names, "test names"));
}

// ============================================================================
// validate tests
// ============================================================================

TEST(Validate, ValidConfigDoesNotThrow)
{
  Config config = create_base_config();
  EXPECT_NO_THROW(validate(config));
}

TEST(Validate, EmptyConfigDoesNotThrow)
{
  Config config;
  EXPECT_NO_THROW(validate(config));
}

TEST(Validate, DuplicateSignalIdThrows)
{
  Config config = create_base_config();

  SignalConfig dup_signal;
  dup_signal.name = "signal_dup";
  dup_signal.id = 100;  // Duplicate of signal_1
  dup_signal.type_string = "float";
  config.signals.push_back(dup_signal);

  EXPECT_THROW(validate(config), NodeBuilderException);
}

TEST(Validate, DuplicateSignalNameThrows)
{
  Config config = create_base_config();

  SignalConfig dup_signal;
  dup_signal.name = "signal_1";  // Duplicate name
  dup_signal.id = 999;
  dup_signal.type_string = "float";
  config.signals.push_back(dup_signal);

  EXPECT_THROW(validate(config), NodeBuilderException);
}

TEST(Validate, DuplicateBundleIdThrows)
{
  Config config = create_base_config();

  BundleConfig dup_bundle;
  dup_bundle.name = "bundle_dup";
  dup_bundle.id = 10;  // Duplicate of bundle_ab
  dup_bundle.period_ms = 50;
  dup_bundle.producers = {"node_a"};
  dup_bundle.consumers = {"node_b"};
  config.bundles.push_back(dup_bundle);

  EXPECT_THROW(validate(config), NodeBuilderException);
}

TEST(Validate, DuplicateBundleNameThrows)
{
  Config config = create_base_config();

  BundleConfig dup_bundle;
  dup_bundle.name = "bundle_ab";  // Duplicate name
  dup_bundle.id = 999;
  dup_bundle.period_ms = 50;
  dup_bundle.producers = {"node_a"};
  dup_bundle.consumers = {"node_b"};
  config.bundles.push_back(dup_bundle);

  EXPECT_THROW(validate(config), NodeBuilderException);
}

TEST(Validate, DuplicateNodeIdThrows)
{
  Config config = create_base_config();

  NodeConfig dup_node;
  dup_node.name = "node_d";
  dup_node.id = 1;  // Duplicate of node_a
  config.nodes["node_d"] = dup_node;

  EXPECT_THROW(validate(config), NodeBuilderException);
}

// ============================================================================
// filter_for_target tests
// ============================================================================

TEST(FilterForTarget, InvalidTargetThrows)
{
  Config config = create_base_config();
  EXPECT_THROW(filter_for_target(config, "nonexistent_node"), NodeBuilderException);
}

TEST(FilterForTarget, TargetNodeIncluded)
{
  Config config = create_base_config();
  Config filtered = filter_for_target(config, "node_a");

  EXPECT_TRUE(filtered.nodes.contains("node_a"));
  EXPECT_EQ(filtered.nodes.at("node_a").id, 1);
}

TEST(FilterForTarget, ConnectedPeerIncluded)
{
  Config config = create_base_config();
  Config filtered = filter_for_target(config, "node_a");

  // node_a is connected to node_b
  EXPECT_TRUE(filtered.nodes.contains("node_b"));
  // node_a is NOT connected to node_c
  EXPECT_FALSE(filtered.nodes.contains("node_c"));
}

TEST(FilterForTarget, MiddleNodeIncludesBothPeers)
{
  Config config = create_base_config();
  Config filtered = filter_for_target(config, "node_b");

  // node_b is connected to both node_a and node_c
  EXPECT_TRUE(filtered.nodes.contains("node_a"));
  EXPECT_TRUE(filtered.nodes.contains("node_b"));
  EXPECT_TRUE(filtered.nodes.contains("node_c"));
}

TEST(FilterForTarget, OnlyRelevantConnectionsIncluded)
{
  Config config = create_base_config();
  Config filtered = filter_for_target(config, "node_a");

  // Only the a <-> b connection should be included
  EXPECT_EQ(filtered.connections.size(), 1);
  EXPECT_TRUE(
    (filtered.connections[0].first.node == "node_a" &&
     filtered.connections[0].second.node == "node_b") ||
    (filtered.connections[0].first.node == "node_b" &&
     filtered.connections[0].second.node == "node_a"));
}

TEST(FilterForTarget, MiddleNodeIncludesAllConnections)
{
  Config config = create_base_config();
  Config filtered = filter_for_target(config, "node_b");

  // Both connections should be included
  EXPECT_EQ(filtered.connections.size(), 2);
}

TEST(FilterForTarget, OnlyProducerBundlesIncluded)
{
  Config config = create_base_config();
  Config filtered = filter_for_target(config, "node_a");

  // node_a produces bundle_ab, but doesn't produce/consume bundle_bc
  EXPECT_EQ(filtered.bundles.size(), 1);
  EXPECT_EQ(filtered.bundles[0].name, "bundle_ab");
}

TEST(FilterForTarget, OnlyConsumerBundlesIncluded)
{
  Config config = create_base_config();
  Config filtered = filter_for_target(config, "node_c");

  // node_c consumes bundle_bc only
  EXPECT_EQ(filtered.bundles.size(), 1);
  EXPECT_EQ(filtered.bundles[0].name, "bundle_bc");
}

TEST(FilterForTarget, MiddleNodeIncludesAllBundles)
{
  Config config = create_base_config();
  Config filtered = filter_for_target(config, "node_b");

  // node_b consumes bundle_ab and produces bundle_bc
  EXPECT_EQ(filtered.bundles.size(), 2);
}

TEST(FilterForTarget, OnlyRelevantSignalsIncluded)
{
  Config config = create_base_config();
  Config filtered = filter_for_target(config, "node_a");

  // bundle_ab uses signal_1 (100) and signal_2 (101), not signal_3 (102)
  EXPECT_EQ(filtered.signals.size(), 2);

  bool has_signal_100 = false;
  bool has_signal_101 = false;
  for (const auto & sig : filtered.signals)
  {
    if (sig.id == 100) has_signal_100 = true;
    if (sig.id == 101) has_signal_101 = true;
  }
  EXPECT_TRUE(has_signal_100);
  EXPECT_TRUE(has_signal_101);
}

TEST(FilterForTarget, NodeCOnlyHasSignal3)
{
  Config config = create_base_config();
  Config filtered = filter_for_target(config, "node_c");

  // bundle_bc only uses signal_3 (102)
  EXPECT_EQ(filtered.signals.size(), 1);
  EXPECT_EQ(filtered.signals[0].id, 102);
}

TEST(FilterForTarget, MiddleNodeIncludesAllSignals)
{
  Config config = create_base_config();
  Config filtered = filter_for_target(config, "node_b");

  // node_b uses all bundles, so all signals
  EXPECT_EQ(filtered.signals.size(), 3);
}

TEST(FilterForTarget, IsolatedNodeHasNoBundles)
{
  Config config = create_base_config();

  // Add an isolated node with no connections or bundles
  NodeConfig isolated;
  isolated.name = "isolated";
  isolated.id = 99;
  config.nodes["isolated"] = isolated;

  Config filtered = filter_for_target(config, "isolated");

  EXPECT_EQ(filtered.nodes.size(), 1);
  EXPECT_TRUE(filtered.nodes.contains("isolated"));
  EXPECT_EQ(filtered.connections.size(), 0);
  EXPECT_EQ(filtered.bundles.size(), 0);
  EXPECT_EQ(filtered.signals.size(), 0);
}

TEST(FilterForTarget, ValidationRunsBeforeFiltering)
{
  Config config = create_base_config();

  // Add a duplicate signal ID to make config invalid
  SignalConfig dup_signal;
  dup_signal.name = "dup";
  dup_signal.id = 100;  // Duplicate
  config.signals.push_back(dup_signal);

  // Should throw during validation, even though target exists
  EXPECT_THROW(filter_for_target(config, "node_a"), NodeBuilderException);
}

int main(int argc, char ** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
