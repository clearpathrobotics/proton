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

#include "proton/encode_decode.h"
#include "proton/node_manager.h"
#include "target_connections.h"
#include "target_registry_ids.h"
#include "utils.hpp"

#include <gtest/gtest.h>
#include <cstring>

extern proton_registry_t g_proton_registry;
extern proton_core_node_t g_target_node;

TEST(NodeManagerTest, BundleForMultipleNodes)
{
  g_target_node.registry = &g_proton_registry;
  uint8_t buf[BUFFER_SIZE];
  size_t out_len = 0;
  static constexpr size_t num_endpoints = 2;
  proton_endpoint_t dest[num_endpoints];
  size_t num_peers = 0;

  proton_node_trigger_bundle(&g_target_node, PROTON_BUNDLE_NODE1_HEARTBEAT_ID);

  ASSERT_EQ(
    proton_node_update(
      &g_target_node, 1000, buf, sizeof(buf), &out_len, dest, num_endpoints, &num_peers),
    PROTON_OK);

  EXPECT_GT(out_len, 0);
  EXPECT_EQ(num_peers, 2);

  EXPECT_EQ(dest[0].node_id, static_cast<uint32_t>(PROTON_NODE_NODE2_ID));
  EXPECT_EQ(dest[0].endpoint_id, static_cast<uint32_t>(PROTON_NODE_NODE2_ENDPOINT_0_ID));
  EXPECT_EQ(dest[0].transport_type, PROTON_NODE_NODE2_ENDPOINT_0_TRANSPORT);

  EXPECT_EQ(dest[1].node_id, static_cast<uint32_t>(PROTON_NODE_NODE3_ID));
  EXPECT_EQ(dest[1].endpoint_id, static_cast<uint32_t>(PROTON_NODE_NODE3_ENDPOINT_0_ID));
  EXPECT_EQ(dest[1].transport_type, PROTON_NODE_NODE3_ENDPOINT_0_TRANSPORT);
}

int main(int argc, char ** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
