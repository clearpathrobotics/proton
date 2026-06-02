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

#include "proton/node_manager.h"
#include "proton/encode_decode.h"
#include "target_connections.h"
#include "target_registry_ids.h"
#include "utils.hpp"

#include <gtest/gtest.h>
#include <cstring>

static constexpr size_t BUFFER_SIZE = 1024;

extern proton_registry_t g_proton_registry;
extern proton_core_node_t g_target_node;

// -----------------------------------------------------------------------
// Test fixture
// -----------------------------------------------------------------------

class NodeManagerTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    // Reset shared bundle table state before each test.
    // bundle_table is not deep-copied by copy_default_registry, so this affects
    // both the global and any per-test registry copy.
    for (size_t i = 0; i < g_proton_registry.bundle_count; i++)
    {
      g_proton_registry.bundle_table[i].last_send_ms = 0;
      g_proton_registry.bundle_table[i].send_now = false;
    }
    registry_ = copy_default_registry(&g_proton_registry);
    node_ = copy_default_node(&g_target_node);
    node_.registry = &registry_;

    mock_mutex_lock_result_ = PROTON_OK;
    mock_mutex_unlock_result_ = PROTON_OK;
    lock_called_ = false;
    unlock_called_ = false;
  }

  void TearDown() override
  {
    free(registry_.signal_registry);
    free(registry_.bundle_callbacks);
    registry_.mutex_handles.arg = nullptr;
    registry_.mutex_handles.lock = nullptr;
    registry_.mutex_handles.unlock = nullptr;
    registry_.mutex_handles.mutex = nullptr;
  }

  static proton_status_e bundle_lock(void * mutex, void * ctx)
  {
    (void)mutex;
    NodeManagerTest * cls = static_cast<NodeManagerTest *>(ctx);
    cls->lock_called_ = true;

    return cls->mock_mutex_lock_result_;
  }

  static proton_status_e bundle_unlock(void * mutex, void * ctx)
  {
    (void)mutex;
    NodeManagerTest * cls = static_cast<NodeManagerTest *>(ctx);
    cls->unlock_called_ = true;

    return cls->mock_mutex_unlock_result_;
  }

  proton_registry_t registry_;
  proton_core_node_t node_;

  bool lock_called_;
  bool unlock_called_;
  proton_status_e mock_mutex_lock_result_;
  proton_status_e mock_mutex_unlock_result_;
};

// -----------------------------------------------------------------------
// proton_node_receive — null-pointer guards
// -----------------------------------------------------------------------

TEST_F(NodeManagerTest, Receive_NullNode_ReturnsNullPtrError)
{
  uint8_t buf[BUFFER_SIZE] = {};
  EXPECT_EQ(proton_node_receive(nullptr, buf, sizeof(buf)), PROTON_NULL_PTR_ERROR);
}

TEST_F(NodeManagerTest, Receive_NullRegistry_ReturnsNullPtrError)
{
  node_.registry = nullptr;
  uint8_t buf[BUFFER_SIZE] = {};
  EXPECT_EQ(proton_node_receive(&node_, buf, sizeof(buf)), PROTON_NULL_PTR_ERROR);
}

TEST_F(NodeManagerTest, Receive_NullBuffer_ReturnsNullPtrError)
{
  EXPECT_EQ(proton_node_receive(&node_, nullptr, BUFFER_SIZE), PROTON_NULL_PTR_ERROR);
}

// -----------------------------------------------------------------------
// proton_node_receive — error cases
// -----------------------------------------------------------------------

TEST_F(NodeManagerTest, Receive_GarbageBuffer_ReturnsSerializationError)
{
  uint8_t buf[BUFFER_SIZE];
  memset(buf, 0xFF, sizeof(buf));
  EXPECT_EQ(proton_node_receive(&node_, buf, sizeof(buf)), PROTON_SERIALIZATION_ERROR);
}

// -----------------------------------------------------------------------
// proton_node_receive — functional tests
// -----------------------------------------------------------------------

TEST_F(NodeManagerTest, Receive_ValidBundle_UpdatesSignalValues)
{
  // Write a distinct value into the registry and encode it
  ASSERT_EQ(
    proton_signal_set_double(&registry_, PROTON_SIGNAL_DEFAULT_DOUBLE_ID, 2.71828), PROTON_OK);

  uint8_t buf[BUFFER_SIZE];
  size_t encoded_len = 0;
  ASSERT_EQ(
    proton_encode_bundle(
      &registry_, PROTON_BUNDLE_DEFAULT_VALUE_TEST_ID, buf, sizeof(buf), &encoded_len),
    PROTON_OK);
  ASSERT_GT(encoded_len, 0);

  // Reset the signal so we can verify receive restores it
  ASSERT_EQ(proton_signal_set_double(&registry_, PROTON_SIGNAL_DEFAULT_DOUBLE_ID, 0.0), PROTON_OK);

  ASSERT_EQ(proton_node_receive(&node_, buf, encoded_len), PROTON_OK);

  double value = 0.0;
  ASSERT_EQ(
    proton_signal_get_double(&registry_, PROTON_SIGNAL_DEFAULT_DOUBLE_ID, &value), PROTON_OK);
  EXPECT_DOUBLE_EQ(value, 2.71828);
}

TEST_F(NodeManagerTest, Receive_ValidBundle_InvokesBundleCallback)
{
  class ReceivedCallback : public BundleCallback
  {
  public:
    explicit ReceivedCallback(proton_registry_t * registry, uint32_t bundle_id)
    {
      proton_registry_set_bundle_callback(registry, bundle_id, bundle_cb, this);
    }
    bool received{false};
    uint32_t received_id{0};
    void callback(uint32_t bundle_id, const uint32_t *, size_t) override
    {
      received = true;
      received_id = bundle_id;
    }
  };

  ReceivedCallback cb(&registry_, PROTON_BUNDLE_VALUE_TEST_ID);

  uint8_t buf[BUFFER_SIZE];
  size_t encoded_len = 0;
  ASSERT_EQ(
    proton_encode_bundle(&registry_, PROTON_BUNDLE_VALUE_TEST_ID, buf, sizeof(buf), &encoded_len),
    PROTON_OK);
  ASSERT_GT(encoded_len, 0);

  ASSERT_EQ(proton_node_receive(&node_, buf, encoded_len), PROTON_OK);

  EXPECT_TRUE(cb.received);
  EXPECT_EQ(cb.received_id, static_cast<uint32_t>(PROTON_BUNDLE_VALUE_TEST_ID));
}

// -----------------------------------------------------------------------
// proton_node_update — null-pointer guards
// -----------------------------------------------------------------------

TEST_F(NodeManagerTest, Update_NullNode_ReturnsNullPtrError)
{
  uint8_t buf[BUFFER_SIZE];
  size_t out_len = 0;
  proton_endpoint_t dest[1];
  size_t num_peers = 0;
  EXPECT_EQ(
    proton_node_update(nullptr, 0, buf, sizeof(buf), &out_len, dest, 1, &num_peers),
    PROTON_NULL_PTR_ERROR);
}

TEST_F(NodeManagerTest, Update_NullRegistry_ReturnsNullPtrError)
{
  node_.registry = nullptr;
  uint8_t buf[BUFFER_SIZE];
  size_t out_len = 0;
  proton_endpoint_t dest[1];
  size_t num_peers = 0;
  EXPECT_EQ(
    proton_node_update(&node_, 0, buf, sizeof(buf), &out_len, dest, 1, &num_peers),
    PROTON_NULL_PTR_ERROR);
}

TEST_F(NodeManagerTest, Update_NullBuffer_ReturnsNullPtrError)
{
  size_t out_len = 0;
  proton_endpoint_t dest[1];
  size_t num_peers = 0;
  EXPECT_EQ(
    proton_node_update(&node_, 0, nullptr, BUFFER_SIZE, &out_len, dest, 1, &num_peers),
    PROTON_NULL_PTR_ERROR);
}

TEST_F(NodeManagerTest, Update_NullOutLen_ReturnsNullPtrError)
{
  uint8_t buf[BUFFER_SIZE];
  proton_endpoint_t dest[1];
  size_t num_peers = 0;
  EXPECT_EQ(
    proton_node_update(&node_, 0, buf, sizeof(buf), nullptr, dest, 1, &num_peers),
    PROTON_NULL_PTR_ERROR);
}

TEST_F(NodeManagerTest, Update_NullDestPeers_ReturnsNullPtrError)
{
  uint8_t buf[BUFFER_SIZE];
  size_t out_len = 0;
  size_t num_peers = 0;
  EXPECT_EQ(
    proton_node_update(&node_, 0, buf, sizeof(buf), &out_len, nullptr, 1, &num_peers),
    PROTON_NULL_PTR_ERROR);
}

TEST_F(NodeManagerTest, Update_NullNumSelectedPeers_ReturnsNullPtrError)
{
  uint8_t buf[BUFFER_SIZE];
  size_t out_len = 0;
  proton_endpoint_t dest[1];
  EXPECT_EQ(
    proton_node_update(&node_, 0, buf, sizeof(buf), &out_len, dest, 1, nullptr),
    PROTON_NULL_PTR_ERROR);
}

// -----------------------------------------------------------------------
// proton_node_update — error cases
// -----------------------------------------------------------------------

TEST_F(NodeManagerTest, Update_InsufficientDestPeerBuffer_ReturnsError)
{
  // All test bundles have exactly 1 consumer; pass 0 slots → should fail
  uint8_t buf[BUFFER_SIZE];
  size_t out_len = 0;
  proton_endpoint_t dest[1];
  size_t num_peers = 0;
  EXPECT_EQ(
    proton_node_update(&node_, 0, buf, sizeof(buf), &out_len, dest, 0, &num_peers),
    PROTON_INSUFFICIENT_BUFFER_ERROR);
}

// -----------------------------------------------------------------------
// proton_node_update — functional tests
// -----------------------------------------------------------------------

TEST_F(NodeManagerTest, Update_EncodesBundle_PopulatesOutputs)
{
  uint8_t buf[BUFFER_SIZE];
  size_t out_len = 0;
  proton_endpoint_t dest[1];
  size_t num_peers = 0;

  proton_node_trigger_bundle(&node_, PROTON_BUNDLE_VALUE_TEST_ID);

  ASSERT_EQ(
    proton_node_update(&node_, 1000, buf, sizeof(buf), &out_len, dest, 1, &num_peers), PROTON_OK);

  EXPECT_GT(out_len, 0);
  EXPECT_EQ(num_peers, 1);
  // dest_peers[0].id is set from the bundle's consumer_ids entry (node ID 1)
  EXPECT_EQ(dest[0].node_id, static_cast<uint32_t>(PROTON_NODE_CONSUMER_ID));
  EXPECT_EQ(dest[0].endpoint_id, static_cast<uint32_t>(PROTON_NODE_CONSUMER_ENDPOINT_0_ID));
}

TEST_F(NodeManagerTest, Update_SetsLastSendMs)
{
  constexpr uint64_t UPTIME_MS = 5000;
  uint8_t buf[BUFFER_SIZE];
  size_t out_len = 0;
  proton_endpoint_t dest[1];
  size_t num_peers = 0;

  ASSERT_EQ(
    proton_node_update(&node_, UPTIME_MS, buf, sizeof(buf), &out_len, dest, 1, &num_peers),
    PROTON_OK);

  // With all bundles at last_send_ms == 0 and no send_now flags, the selection algorithm
  // (using <=) picks the last bundle in the table. Verify exactly one bundle was updated.
  size_t updated_count = 0;
  for (size_t i = 0; i < node_.registry->bundle_count; i++)
  {
    if (node_.registry->bundle_table[i].last_send_ms == UPTIME_MS)
    {
      updated_count++;
    }
  }
  EXPECT_EQ(updated_count, 1);
}

TEST_F(NodeManagerTest, Update_TriggeredBundleIsPrioritized)
{
  // Trigger value_test (index 0). Without a trigger, the algorithm selects the last
  // bundle in the table (all timestamps start at 0 and ties favour later indices).
  // A send_now flag overrides that ordering.
  proton_node_trigger_bundle(&node_, PROTON_BUNDLE_VALUE_TEST_ID);

  constexpr uint64_t UPTIME_MS = 1000;
  uint8_t buf[BUFFER_SIZE];
  size_t out_len = 0;
  proton_endpoint_t dest[1];
  size_t num_peers = 0;

  ASSERT_EQ(
    proton_node_update(&node_, UPTIME_MS, buf, sizeof(buf), &out_len, dest, 1, &num_peers),
    PROTON_OK);
  EXPECT_GT(out_len, 0);

  // value_test (index 0) was sent
  EXPECT_EQ(node_.registry->bundle_table[0].last_send_ms, UPTIME_MS);
  EXPECT_FALSE(node_.registry->bundle_table[0].send_now);

  // The last bundle (shared_2, index 3) was NOT sent
  EXPECT_EQ(node_.registry->bundle_table[3].last_send_ms, 0ULL);
}

TEST_F(NodeManagerTest, PeriodicBundleTest)
{
  uint64_t uptime_ms = 1000;
  uint8_t buf[BUFFER_SIZE];
  size_t out_len = 0;
  proton_endpoint_t dest[1];
  size_t num_peers = 0;

  ASSERT_EQ(
    proton_node_update(&node_, uptime_ms, buf, sizeof(buf), &out_len, dest, 1, &num_peers),
    PROTON_OK);
  EXPECT_GT(out_len, 0);
  EXPECT_EQ(num_peers, 1);

  // periodic_bundle
  size_t bundle_slot;
  const bundle_desc_t * desc =
    proton_registry_get_bundle(node_.registry, PROTON_BUNDLE_PERIODIC_BUNDLE_ID, &bundle_slot);
  ASSERT_NE(desc, nullptr);
  EXPECT_EQ(desc->last_send_ms, uptime_ms);

  // Update bundle again, before the period of the periodic bundle is up
  memset(buf, 0, BUFFER_SIZE);
  num_peers = 0;
  memset(dest, 0, sizeof(proton_endpoint_t));
  uint64_t second_uptime = uptime_ms + 10;

  ASSERT_EQ(
    proton_node_update(&node_, second_uptime, buf, sizeof(buf), &out_len, dest, 1, &num_peers),
    PROTON_OK);
  EXPECT_GT(out_len, 0);
  EXPECT_EQ(num_peers, 0);

  EXPECT_EQ(node_.registry->bundle_table[bundle_slot].last_send_ms, uptime_ms);

  // Update bundle a third time, after the bundle period has expired
  uint64_t third_uptime = second_uptime + desc->period_ms;
  ASSERT_EQ(
    proton_node_update(&node_, third_uptime, buf, sizeof(buf), &out_len, dest, 1, &num_peers),
    PROTON_OK);
  EXPECT_GT(out_len, 0);
  EXPECT_EQ(num_peers, 1);

  EXPECT_EQ(node_.registry->bundle_table[bundle_slot].last_send_ms, third_uptime);
}

// -----------------------------------------------------------------------
// proton_node_encode_bundle — null-pointer guards
// -----------------------------------------------------------------------

TEST_F(NodeManagerTest, EncodeBundle_NullNode_ReturnsNullPtrError)
{
  uint8_t buf[BUFFER_SIZE];
  size_t out_len = 0;
  proton_endpoint_t dest[1];
  size_t num_peers = 0;
  EXPECT_EQ(
    proton_node_encode_bundle(
      nullptr, PROTON_BUNDLE_VALUE_TEST_ID, 0, buf, sizeof(buf), &out_len, dest, 1, &num_peers),
    PROTON_NULL_PTR_ERROR);
}

TEST_F(NodeManagerTest, EncodeBundle_NullRegistry_ReturnsNullPtrError)
{
  node_.registry = nullptr;
  uint8_t buf[BUFFER_SIZE];
  size_t out_len = 0;
  proton_endpoint_t dest[1];
  size_t num_peers = 0;
  EXPECT_EQ(
    proton_node_encode_bundle(
      &node_, PROTON_BUNDLE_VALUE_TEST_ID, 0, buf, sizeof(buf), &out_len, dest, 1, &num_peers),
    PROTON_NULL_PTR_ERROR);
}

TEST_F(NodeManagerTest, EncodeBundle_NullBuffer_ReturnsNullPtrError)
{
  size_t out_len = 0;
  proton_endpoint_t dest[1];
  size_t num_peers = 0;
  EXPECT_EQ(
    proton_node_encode_bundle(
      &node_, PROTON_BUNDLE_VALUE_TEST_ID, 0, nullptr, BUFFER_SIZE, &out_len, dest, 1, &num_peers),
    PROTON_NULL_PTR_ERROR);
}

TEST_F(NodeManagerTest, EncodeBundle_NullOutLen_ReturnsNullPtrError)
{
  uint8_t buf[BUFFER_SIZE];
  proton_endpoint_t dest[1];
  size_t num_peers = 0;
  EXPECT_EQ(
    proton_node_encode_bundle(
      &node_, PROTON_BUNDLE_VALUE_TEST_ID, 0, buf, sizeof(buf), nullptr, dest, 1, &num_peers),
    PROTON_NULL_PTR_ERROR);
}

TEST_F(NodeManagerTest, EncodeBundle_NullDestPeers_ReturnsNullPtrError)
{
  uint8_t buf[BUFFER_SIZE];
  size_t out_len = 0;
  size_t num_peers = 0;
  EXPECT_EQ(
    proton_node_encode_bundle(
      &node_, PROTON_BUNDLE_VALUE_TEST_ID, 0, buf, sizeof(buf), &out_len, nullptr, 1, &num_peers),
    PROTON_NULL_PTR_ERROR);
}

TEST_F(NodeManagerTest, EncodeBundle_NullNumSelectedPeers_ReturnsNullPtrError)
{
  uint8_t buf[BUFFER_SIZE];
  size_t out_len = 0;
  proton_endpoint_t dest[1];
  EXPECT_EQ(
    proton_node_encode_bundle(
      &node_, PROTON_BUNDLE_VALUE_TEST_ID, 0, buf, sizeof(buf), &out_len, dest, 1, nullptr),
    PROTON_NULL_PTR_ERROR);
}

// -----------------------------------------------------------------------
// proton_node_encode_bundle — error cases
// -----------------------------------------------------------------------

TEST_F(NodeManagerTest, EncodeBundle_UnknownBundleId_ReturnsIncorrectTargetError)
{
  uint8_t buf[BUFFER_SIZE];
  size_t out_len = 0;
  proton_endpoint_t dest[1];
  size_t num_peers = 0;
  EXPECT_EQ(
    proton_node_encode_bundle(
      &node_, 0xDEADBEEFu, 0, buf, sizeof(buf), &out_len, dest, 1, &num_peers),
    PROTON_INCORRECT_TARGET_ERROR);
}

TEST_F(NodeManagerTest, EncodeBundle_InsufficientDestPeerBuffer_ReturnsError)
{
  uint8_t buf[BUFFER_SIZE];
  size_t out_len = 0;
  proton_endpoint_t dest[1];
  size_t num_peers = 0;
  EXPECT_EQ(
    proton_node_encode_bundle(
      &node_, PROTON_BUNDLE_VALUE_TEST_ID, 0, buf, sizeof(buf), &out_len, dest, 0, &num_peers),
    PROTON_INSUFFICIENT_BUFFER_ERROR);
}

// -----------------------------------------------------------------------
// proton_node_encode_bundle — functional tests
// -----------------------------------------------------------------------

TEST_F(NodeManagerTest, EncodeBundle_ValidBundle_PopulatesOutputsAndUpdatesTimestamp)
{
  constexpr uint64_t UPTIME_MS = 4242;
  uint8_t buf[BUFFER_SIZE];
  size_t out_len = 0;
  proton_endpoint_t dest[1];
  size_t num_peers = 0;

  ASSERT_EQ(
    proton_node_encode_bundle(
      &node_, PROTON_BUNDLE_VALUE_TEST_ID, UPTIME_MS, buf, sizeof(buf), &out_len, dest, 1,
      &num_peers),
    PROTON_OK);

  EXPECT_GT(out_len, 0);
  EXPECT_EQ(num_peers, 1);
  EXPECT_EQ(dest[0].node_id, static_cast<uint32_t>(PROTON_NODE_CONSUMER_ID));
  EXPECT_EQ(dest[0].endpoint_id, static_cast<uint32_t>(PROTON_NODE_CONSUMER_ENDPOINT_0_ID));
  // bundle_table is shared; verify last_send_ms was updated on the value_test slot (index 0)
  EXPECT_EQ(node_.registry->bundle_table[0].last_send_ms, UPTIME_MS);
}

TEST_F(NodeManagerTest, EncodeBundle_RoundTrip_WithReceive_SignalValuePreserved)
{
  constexpr float SENT_VALUE = 3.14f;
  ASSERT_EQ(
    proton_signal_set_float(&registry_, PROTON_SIGNAL_FLOAT_VALUE_ID, SENT_VALUE), PROTON_OK);

  uint8_t buf[BUFFER_SIZE];
  size_t out_len = 0;
  proton_endpoint_t dest[1];
  size_t num_peers = 0;
  ASSERT_EQ(
    proton_node_encode_bundle(
      &node_, PROTON_BUNDLE_VALUE_TEST_ID, 0, buf, sizeof(buf), &out_len, dest, 1, &num_peers),
    PROTON_OK);
  ASSERT_GT(out_len, 0);

  ASSERT_EQ(proton_signal_set_float(&registry_, PROTON_SIGNAL_FLOAT_VALUE_ID, 0.0f), PROTON_OK);

  ASSERT_EQ(proton_node_receive(&node_, buf, out_len), PROTON_OK);

  float received = 0.0f;
  ASSERT_EQ(
    proton_signal_get_float(&registry_, PROTON_SIGNAL_FLOAT_VALUE_ID, &received), PROTON_OK);
  EXPECT_FLOAT_EQ(received, SENT_VALUE);
}

// -----------------------------------------------------------------------
// proton_node_trigger_bundle — null-pointer guards
// -----------------------------------------------------------------------

TEST_F(NodeManagerTest, Trigger_NullNode_ReturnsNullPtrError)
{
  EXPECT_EQ(
    proton_node_trigger_bundle(nullptr, PROTON_BUNDLE_VALUE_TEST_ID), PROTON_NULL_PTR_ERROR);
}

TEST_F(NodeManagerTest, Trigger_NullRegistry_ReturnsNullPtrError)
{
  node_.registry = nullptr;
  EXPECT_EQ(proton_node_trigger_bundle(&node_, PROTON_BUNDLE_VALUE_TEST_ID), PROTON_NULL_PTR_ERROR);
}

TEST_F(NodeManagerTest, Trigger_UnknownBundleId_ReturnsIncorrectTargetError)
{
  EXPECT_EQ(proton_node_trigger_bundle(&node_, 0xDEADBEEFu), PROTON_INCORRECT_TARGET_ERROR);
}

// -----------------------------------------------------------------------
// proton_node_trigger_bundle — ring buffer behaviour
// -----------------------------------------------------------------------

TEST_F(NodeManagerTest, Trigger_Single_AdvancesHeadByOne)
{
  ASSERT_EQ(node_.trigger_head, 0u);
  ASSERT_EQ(proton_node_trigger_bundle(&node_, PROTON_BUNDLE_VALUE_TEST_ID), PROTON_OK);
  EXPECT_EQ(node_.trigger_head, 1u);
}

TEST_F(NodeManagerTest, Trigger_Single_SlotResolvesToCorrectBundle)
{
  ASSERT_EQ(proton_node_trigger_bundle(&node_, PROTON_BUNDLE_VALUE_TEST_ID), PROTON_OK);
  // pending_triggers[0] holds a bundle_table slot index; resolve it back to a bundle ID
  EXPECT_EQ(
    registry_.bundle_table[node_.pending_triggers[0]].bundle_id,
    static_cast<uint32_t>(PROTON_BUNDLE_VALUE_TEST_ID));
}

TEST_F(NodeManagerTest, Trigger_QueueFull_ReturnsInsufficientBufferError)
{
  // Ring buffer holds PROTON_MAX_PENDING_TRIGGERS - 1 items before the head+1 == tail full check
  for (size_t i = 0; i < PROTON_MAX_PENDING_TRIGGERS - 1; i++)
  {
    ASSERT_EQ(proton_node_trigger_bundle(&node_, PROTON_BUNDLE_VALUE_TEST_ID), PROTON_OK)
      << "Expected PROTON_OK on push " << i;
  }
  EXPECT_EQ(
    proton_node_trigger_bundle(&node_, PROTON_BUNDLE_VALUE_TEST_ID),
    PROTON_INSUFFICIENT_BUFFER_ERROR);
}

// -----------------------------------------------------------------------
// proton_node_update — pending_triggers drain
// -----------------------------------------------------------------------

TEST_F(NodeManagerTest, Update_DrainsPendingTriggers_QueueEmptyAfterUpdate)
{
  ASSERT_EQ(proton_node_trigger_bundle(&node_, PROTON_BUNDLE_VALUE_TEST_ID), PROTON_OK);
  ASSERT_NE(node_.trigger_head, node_.trigger_tail);

  uint8_t buf[BUFFER_SIZE];
  size_t out_len = 0;
  proton_endpoint_t dest[1];
  size_t num_peers = 0;
  ASSERT_EQ(
    proton_node_update(&node_, 100, buf, sizeof(buf), &out_len, dest, 1, &num_peers), PROTON_OK);

  EXPECT_EQ(node_.trigger_head, node_.trigger_tail);
}

TEST_F(NodeManagerTest, Update_QueueTriggeredBundle_IsPrioritizedOverDefault)
{
  // Trigger value_test (slot 0) via the ring buffer only — do not touch send_now directly
  ASSERT_EQ(proton_node_trigger_bundle(&node_, PROTON_BUNDLE_VALUE_TEST_ID), PROTON_OK);

  constexpr uint64_t UPTIME_MS = 1000;
  uint8_t buf[BUFFER_SIZE];
  size_t out_len = 0;
  proton_endpoint_t dest[1];
  size_t num_peers = 0;
  ASSERT_EQ(
    proton_node_update(&node_, UPTIME_MS, buf, sizeof(buf), &out_len, dest, 1, &num_peers),
    PROTON_OK);

  EXPECT_GT(out_len, 0);
  // value_test (slot 0) was the triggered bundle — its timestamp must be updated
  EXPECT_EQ(node_.registry->bundle_table[0].last_send_ms, UPTIME_MS);
  // The fallback bundle (shared_2, slot 3) must NOT have been sent
  EXPECT_EQ(node_.registry->bundle_table[3].last_send_ms, 0ULL);
  // send_now must be cleared after sending
  EXPECT_FALSE(node_.registry->bundle_table[0].send_now);
}

// -----------------------------------------------------------------------
// Round-trip: proton_node_update → proton_node_receive
// -----------------------------------------------------------------------

TEST_F(NodeManagerTest, RoundTrip_UpdateThenReceive_SignalValuePreserved)
{
  // Set a signal value that lives in value_test bundle
  constexpr int32_t SENT_VALUE = 12345;
  ASSERT_EQ(
    proton_signal_set_int32(&registry_, PROTON_SIGNAL_INT32_VALUE_ID, SENT_VALUE), PROTON_OK);

  // Trigger value_test so it is selected by update regardless of timestamp ordering
  proton_node_trigger_bundle(&node_, PROTON_BUNDLE_VALUE_TEST_ID);

  uint8_t buf[BUFFER_SIZE];
  size_t out_len = 0;
  proton_endpoint_t dest[1];
  size_t num_peers = 0;

  ASSERT_EQ(
    proton_node_update(&node_, 100, buf, sizeof(buf), &out_len, dest, 1, &num_peers), PROTON_OK);
  ASSERT_GT(out_len, 0);

  // Clear the signal to confirm receive overwrites it
  ASSERT_EQ(proton_signal_set_int32(&registry_, PROTON_SIGNAL_INT32_VALUE_ID, 0), PROTON_OK);

  ASSERT_EQ(proton_node_receive(&node_, buf, out_len), PROTON_OK);

  int32_t received_value = 0;
  ASSERT_EQ(
    proton_signal_get_int32(&registry_, PROTON_SIGNAL_INT32_VALUE_ID, &received_value), PROTON_OK);
  EXPECT_EQ(received_value, SENT_VALUE);
}

// ---------------------------------------------------------------------------
// Mutex:
//      - Whether mutexes were called in happy-path and unhappy-path scenarios
//      - If intentionally-failing lock/unlock returns the correct value from node_manager
// ---------------------------------------------------------------------------

TEST_F(NodeManagerTest, MutexSuccessfulTrigger)
{
  registry_.mutex_handles.arg = this;
  registry_.mutex_handles.mutex = nullptr;
  registry_.mutex_handles.lock = NodeManagerTest::bundle_lock;
  registry_.mutex_handles.unlock = NodeManagerTest::bundle_unlock;

  EXPECT_EQ(proton_node_trigger_bundle(&node_, PROTON_BUNDLE_VALUE_TEST_ID), PROTON_OK);
  EXPECT_TRUE(lock_called_);
  EXPECT_TRUE(unlock_called_);
}

TEST_F(NodeManagerTest, MutexBadLockReturnsLockError)
{
  registry_.mutex_handles.arg = this;
  registry_.mutex_handles.mutex = nullptr;
  registry_.mutex_handles.lock = NodeManagerTest::bundle_lock;
  registry_.mutex_handles.unlock = NodeManagerTest::bundle_unlock;
  // set mock result to an error that's not used in proton_node_trigger_bundle
  mock_mutex_lock_result_ = PROTON_DISCONNECT_ERROR;

  EXPECT_EQ(
    proton_node_trigger_bundle(&node_, PROTON_BUNDLE_VALUE_TEST_ID), PROTON_DISCONNECT_ERROR);
  EXPECT_TRUE(lock_called_);
  EXPECT_FALSE(unlock_called_);
}

TEST_F(NodeManagerTest, MutexBadUnlockReturnsUnlockError)
{
  registry_.mutex_handles.arg = this;
  registry_.mutex_handles.mutex = nullptr;
  registry_.mutex_handles.lock = NodeManagerTest::bundle_lock;
  registry_.mutex_handles.unlock = NodeManagerTest::bundle_unlock;
  // set mock result to an error that's not used in proton_node_trigger_bundle
  mock_mutex_unlock_result_ = PROTON_DISCONNECT_ERROR;

  EXPECT_EQ(
    proton_node_trigger_bundle(&node_, PROTON_BUNDLE_VALUE_TEST_ID), PROTON_DISCONNECT_ERROR);
  EXPECT_TRUE(lock_called_);
  EXPECT_TRUE(unlock_called_);
}

TEST_F(NodeManagerTest, MutexUnlocksOnIncorrectTargetTrigger)
{
  registry_.mutex_handles.arg = this;
  registry_.mutex_handles.mutex = nullptr;
  registry_.mutex_handles.lock = NodeManagerTest::bundle_lock;
  registry_.mutex_handles.unlock = NodeManagerTest::bundle_unlock;

  EXPECT_EQ(proton_node_trigger_bundle(&node_, 9999), PROTON_INCORRECT_TARGET_ERROR);
  EXPECT_TRUE(lock_called_);
  EXPECT_TRUE(unlock_called_);
}

TEST_F(NodeManagerTest, MutexUnlocksOnTriggerInsufficientBuffer)
{
  registry_.mutex_handles.arg = this;
  registry_.mutex_handles.mutex = nullptr;
  registry_.mutex_handles.lock = NodeManagerTest::bundle_lock;
  registry_.mutex_handles.unlock = NodeManagerTest::bundle_unlock;

  // Ring buffer holds PROTON_MAX_PENDING_TRIGGERS - 1 items before the head+1 == tail full check
  for (size_t i = 0; i < PROTON_MAX_PENDING_TRIGGERS - 1; i++)
  {
    ASSERT_EQ(proton_node_trigger_bundle(&node_, PROTON_BUNDLE_VALUE_TEST_ID), PROTON_OK)
      << "Expected PROTON_OK on push " << i;
  }
  // Reset lock/unlock for the last call
  lock_called_ = false;
  unlock_called_ = false;
  EXPECT_EQ(
    proton_node_trigger_bundle(&node_, PROTON_BUNDLE_VALUE_TEST_ID),
    PROTON_INSUFFICIENT_BUFFER_ERROR);

  EXPECT_TRUE(lock_called_);
  EXPECT_TRUE(unlock_called_);
}

int main(int argc, char ** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
