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
#include "protoncpp/bundle_access.hpp"
#include "protoncpp/node_access.hpp"
#include "protoncpp/signal_access.hpp"
#include "target_connections.h"
#include "target_registry_ids.h"
#include "target_registry_sizes.h"
#include "utils.hpp"

#if __cplusplus >= 202002L
#include <array>
#include <span>
#endif

#if PROTON_ENABLE_ALLOC
#include <vector>
#endif

extern proton_registry_t g_proton_registry;
extern proton_node_t g_target_node;

using namespace proton;

// -----------------------------------------------------------------------
// Test fixture
// -----------------------------------------------------------------------

class NodeAccessTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    // Reset shared bundle table state before each test
    for (size_t i = 0; i < g_proton_registry.bundle_count; i++)
    {
      g_proton_registry.bundle_table[i].last_send_ms = 0;
      g_proton_registry.bundle_table[i].send_now = false;
      g_proton_registry.bundle_table[i].callback = {NULL, NULL};
    }
    registry_ = copy_default_registry(&g_proton_registry);
    node_ = copy_default_node(&g_target_node);
    node_.registry = &registry_;
  }

  void TearDown() override
  {
    free(registry_.signal_registry);
    free(registry_.bundle_table);
    if (node_.num_peers > 0)
    {
      free(const_cast<proton_endpoint_t *>(node_.destination_peers));
    }
  }

  proton_registry_t registry_;
  proton_node_t node_;
};

// -----------------------------------------------------------------------
// Accessors
// -----------------------------------------------------------------------

TEST_F(NodeAccessTest, Id_ReturnsNodeId)
{
  NodeAccess access(&node_);
  EXPECT_EQ(access.id(), node_.id);
}

TEST_F(NodeAccessTest, NumPeers_ReturnsCorrectCount)
{
  NodeAccess access(&node_);
  EXPECT_EQ(access.num_peers(), node_.num_peers);
}

TEST_F(NodeAccessTest, HasPendingTriggers_InitiallyFalse)
{
  NodeAccess access(&node_);
  EXPECT_FALSE(access.has_pending_triggers());
}

TEST_F(NodeAccessTest, HasPendingTriggers_TrueAfterTrigger)
{
  NodeAccess access(&node_);
  ASSERT_EQ(access.trigger_bundle(PROTON_BUNDLE_VALUE_TEST_ID), PROTON_OK);
  EXPECT_TRUE(access.has_pending_triggers());
}

// -----------------------------------------------------------------------
// signals() / bundle() factory methods
// -----------------------------------------------------------------------

TEST_F(NodeAccessTest, Signals_ReturnsValidSignalAccess)
{
  NodeAccess access(&node_);
  SignalAccess signals = access.signals();

  // Verify we can use the returned SignalAccess
  double value;
  proton_status_e status = signals.get(PROTON_SIGNAL_DEFAULT_DOUBLE_ID, value);
  EXPECT_EQ(status, PROTON_OK);
  EXPECT_EQ(value, 3.14159);
}

TEST_F(NodeAccessTest, Bundle_ReturnsValidBundleAccess)
{
  NodeAccess access(&node_);
  BundleAccess bundle = access.bundle(PROTON_BUNDLE_VALUE_TEST_ID);

  // Verify we can use the returned BundleAccess
  EXPECT_EQ(bundle.id(), PROTON_BUNDLE_VALUE_TEST_ID);
  EXPECT_NE(bundle.descriptor(), nullptr);
}

// -----------------------------------------------------------------------
// trigger_bundle
// -----------------------------------------------------------------------

TEST_F(NodeAccessTest, TriggerBundle_ValidBundle_ReturnsOk)
{
  NodeAccess access(&node_);
  EXPECT_EQ(access.trigger_bundle(PROTON_BUNDLE_VALUE_TEST_ID), PROTON_OK);
}

TEST_F(NodeAccessTest, TriggerBundle_InvalidBundle_ReturnsError)
{
  NodeAccess access(&node_);
  EXPECT_EQ(access.trigger_bundle(0x9999), PROTON_INCORRECT_TARGET_ERROR);
}

// -----------------------------------------------------------------------
// receive
// -----------------------------------------------------------------------

TEST_F(NodeAccessTest, Receive_NullBuffer_ReturnsNullPtrError)
{
  NodeAccess access(&node_);
  EXPECT_EQ(access.receive(nullptr, BUFFER_SIZE), PROTON_NULL_PTR_ERROR);
}

TEST_F(NodeAccessTest, Receive_EmptyBuffer_ReturnsNotSupportedError)
{
  NodeAccess access(&node_);
  uint8_t buf[1] = {};
  EXPECT_EQ(access.receive(buf, 0), PROTON_UNSUPPORTED_OPERATION_ERROR);
}

// -----------------------------------------------------------------------
// update
// -----------------------------------------------------------------------

TEST_F(NodeAccessTest, Update_NoPendingBundles_ReturnsOkNothingToSend)
{
  NodeAccess access(&node_);

  uint8_t buffer[BUFFER_SIZE] = {};
  size_t out_len = 0;
  NodeAccess::Endpoint dest[4] = {};
  size_t num_selected = 0;

  proton_status_e status =
    access.update(1000, buffer, sizeof(buffer), out_len, dest, 4, num_selected);
  EXPECT_EQ(status, PROTON_OK);
}

TEST_F(NodeAccessTest, Update_AfterTrigger_ReturnsOk)
{
  NodeAccess access(&node_);

  // Trigger a bundle first
  ASSERT_EQ(access.trigger_bundle(PROTON_BUNDLE_VALUE_TEST_ID), PROTON_OK);

  uint8_t buffer[BUFFER_SIZE] = {};
  size_t out_len = 0;
  NodeAccess::Endpoint dest[4] = {};
  size_t num_selected = 0;

  proton_status_e status =
    access.update(1000, buffer, sizeof(buffer), out_len, dest, 4, num_selected);
  EXPECT_EQ(status, PROTON_OK);
  EXPECT_GT(out_len, 0u);
}

// -----------------------------------------------------------------------
// encode_bundle
// -----------------------------------------------------------------------

TEST_F(NodeAccessTest, EncodeBundle_ValidBundle_ReturnsOk)
{
  NodeAccess access(&node_);

  uint8_t buffer[BUFFER_SIZE] = {};
  size_t out_len = 0;
  NodeAccess::Endpoint dest[4] = {};
  size_t num_selected = 0;

  proton_status_e status = access.encode_bundle(
    PROTON_BUNDLE_VALUE_TEST_ID, 1000, buffer, sizeof(buffer), out_len, dest, 4, num_selected);
  EXPECT_EQ(status, PROTON_OK);
  EXPECT_GT(out_len, 0u);
}

TEST_F(NodeAccessTest, EncodeBundle_InvalidBundle_ReturnsError)
{
  NodeAccess access(&node_);

  uint8_t buffer[BUFFER_SIZE] = {};
  size_t out_len = 0;
  NodeAccess::Endpoint dest[4] = {};
  size_t num_selected = 0;

  proton_status_e status =
    access.encode_bundle(0x9999, 1000, buffer, sizeof(buffer), out_len, dest, 4, num_selected);
  EXPECT_EQ(status, PROTON_INCORRECT_TARGET_ERROR);
}

// -----------------------------------------------------------------------
// on_bundle_update (PROTON_ENABLE_ALLOC only)
// -----------------------------------------------------------------------

#if PROTON_ENABLE_ALLOC

TEST_F(NodeAccessTest, OnBundleUpdate_CallbackIsInvoked)
{
  NodeAccess access(&node_);

  bool callback_called = false;
  uint32_t received_bundle_id = 0;

  access.on_bundle_update(
    PROTON_BUNDLE_VALUE_TEST_ID,
    [&](uint32_t bundle_id, const uint32_t *, size_t)
    {
      callback_called = true;
      received_bundle_id = bundle_id;
    });

  // Manually invoke the callback through the registry
  proton_bundle_cb_t * cb =
    proton_registry_get_bundle_callback(&registry_, PROTON_BUNDLE_VALUE_TEST_ID);
  ASSERT_NE(cb, nullptr);
  ASSERT_NE(cb->cb, nullptr);

  uint8_t buffer[BUFFER_SIZE] = {};
  size_t out_len = 0;
  NodeAccess::Endpoint dest[4] = {};
  size_t num_selected = 0;

  proton_status_e status = access.encode_bundle(
    PROTON_BUNDLE_VALUE_TEST_ID, 1000, buffer, sizeof(buffer), out_len, dest, 4, num_selected);

  EXPECT_EQ(status, PROTON_OK);

  status = access.receive(buffer, out_len);

  EXPECT_EQ(status, PROTON_OK);
  EXPECT_TRUE(callback_called);
  EXPECT_EQ(received_bundle_id, PROTON_BUNDLE_VALUE_TEST_ID);
}

#endif  // PROTON_ENABLE_ALLOC

// -----------------------------------------------------------------------
// C++20 span overloads
// -----------------------------------------------------------------------

#if __cplusplus >= 202002L

TEST_F(NodeAccessTest, Peers_ReturnsSpanOfEndpoints)
{
  NodeAccess access(&node_);
  std::span<const NodeAccess::Endpoint> peers = access.peers();

  EXPECT_EQ(peers.size(), node_.num_peers);
  if (!peers.empty())
  {
    EXPECT_EQ(peers[0].node_id, node_.destination_peers[0].node_id);
  }
}

TEST_F(NodeAccessTest, ReceiveSpan_EmptySpan_ReturnsNullPtrError)
{
  NodeAccess access(&node_);
  std::span<const uint8_t> empty_span;
  EXPECT_EQ(access.receive(empty_span), PROTON_NULL_PTR_ERROR);
}

TEST_F(NodeAccessTest, UpdateSpan_NoPendingBundles_ReturnsNothingToSend)
{
  NodeAccess access(&node_);

  std::array<uint8_t, BUFFER_SIZE> buffer = {};
  size_t out_len = 0;
  std::array<NodeAccess::Endpoint, 4> dest = {};
  size_t num_selected = 0;

  proton_status_e status = access.update(1000, buffer, out_len, dest, num_selected);
  EXPECT_EQ(status, PROTON_OK);
}

TEST_F(NodeAccessTest, UpdateSpan_AfterTrigger_ReturnsOk)
{
  NodeAccess access(&node_);

  ASSERT_EQ(access.trigger_bundle(PROTON_BUNDLE_VALUE_TEST_ID), PROTON_OK);

  std::array<uint8_t, BUFFER_SIZE> buffer = {};
  size_t out_len = 0;
  std::array<NodeAccess::Endpoint, 4> dest = {};
  size_t num_selected = 0;

  proton_status_e status = access.update(1000, buffer, out_len, dest, num_selected);
  EXPECT_EQ(status, PROTON_OK);
  EXPECT_GT(out_len, 0u);
}

TEST_F(NodeAccessTest, EncodeBundleSpan_ValidBundle_ReturnsOk)
{
  NodeAccess access(&node_);

  std::array<uint8_t, BUFFER_SIZE> buffer = {};
  size_t out_len = 0;
  std::array<NodeAccess::Endpoint, 4> dest = {};
  size_t num_selected = 0;

  proton_status_e status =
    access.encode_bundle(PROTON_BUNDLE_VALUE_TEST_ID, 1000, buffer, out_len, dest, num_selected);
  EXPECT_EQ(status, PROTON_OK);
  EXPECT_GT(out_len, 0u);
}

#endif  // __cplusplus >= 202002L

// -----------------------------------------------------------------------
// C++17 [] operator overloads
// -----------------------------------------------------------------------

TEST_F(NodeAccessTest, GetValidBundleByIndex)
{
  NodeAccess access(&node_);
  std::optional<BundleAccess> bundle = access[PROTON_BUNDLE_VALUE_TEST_ID];
  EXPECT_TRUE(bundle.has_value());
  EXPECT_EQ(bundle->id(), PROTON_BUNDLE_VALUE_TEST_ID);
}

TEST_F(NodeAccessTest, InvalidBundleIdReturnsNullopt)
{
  NodeAccess access(&node_);
  std::optional<BundleAccess> bundle = access[9999];
  EXPECT_FALSE(bundle.has_value());
}

TEST_F(NodeAccessTest, InvalidNodeReturnsNullopt)
{
  NodeAccess access(nullptr);
  std::optional<BundleAccess> bundle = access[PROTON_BUNDLE_VALUE_TEST_ID];
  EXPECT_FALSE(bundle.has_value());
}

TEST_F(NodeAccessTest, InvalidRegistryReturnsNullopt)
{
  node_.registry = nullptr;
  NodeAccess access(&node_);
  std::optional<BundleAccess> bundle = access[PROTON_BUNDLE_VALUE_TEST_ID];
  EXPECT_FALSE(bundle.has_value());
}

int main(int argc, char ** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
