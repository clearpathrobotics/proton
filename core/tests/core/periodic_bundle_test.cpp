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

static constexpr size_t BUFFER_SIZE = 1024;

extern proton_registry_t g_proton_registry;
extern proton_core_node_t g_target_node;

// -----------------------------------------------------------------------
// Test fixture
// -----------------------------------------------------------------------

class PeriodicBundleTest : public ::testing::Test
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
  }

  void TearDown() override
  {
    free(registry_.signal_registry);
    free(registry_.bundle_callbacks);
  }

  proton_registry_t registry_;
  proton_core_node_t node_;
};

/**
 * More advanced bundle periodicity tests
 */

TEST_F(PeriodicBundleTest, DoNotSendBeforeFirstPeriod)
{
  uint64_t uptime_ms = 10;
  uint8_t buf[BUFFER_SIZE];
  size_t out_len = 0;
  proton_endpoint_t dest[1];
  size_t num_peers = 0;

  ASSERT_EQ(
    proton_node_update(&node_, uptime_ms, buf, sizeof(buf), &out_len, dest, 1, &num_peers),
    PROTON_OK);
  EXPECT_EQ(out_len, 0);
  EXPECT_EQ(num_peers, 0);

  for (size_t i = 0; i < node_.registry->bundle_count; i++)
  {
    EXPECT_EQ(node_.registry->bundle_table[i].last_send_ms, 0);
  }
}

TEST_F(PeriodicBundleTest, SendNextOverdueBundle)
{
  uint8_t buf[BUFFER_SIZE];
  size_t out_len = 0;
  proton_endpoint_t dest[1];
  size_t num_peers = 0;

  size_t bundle_100_ms_slot;
  const bundle_desc_t * bundle_100_ms =
    proton_registry_get_bundle(node_.registry, PROTON_BUNDLE_100_MS_ID, &bundle_100_ms_slot);
  size_t bundle_120_ms_slot;
  const bundle_desc_t * bundle_120_ms =
    proton_registry_get_bundle(node_.registry, PROTON_BUNDLE_120_MS_ID, &bundle_120_ms_slot);

  ASSERT_NE(bundle_100_ms, nullptr);
  ASSERT_NE(bundle_120_ms, nullptr);

  uint64_t first_uptime_ms = bundle_100_ms->period_ms + 1;

  ASSERT_EQ(
    proton_node_update(&node_, first_uptime_ms, buf, sizeof(buf), &out_len, dest, 1, &num_peers),
    PROTON_OK);
  EXPECT_GT(out_len, 0);
  EXPECT_EQ(num_peers, 1);

  EXPECT_EQ(bundle_100_ms->last_send_ms, first_uptime_ms);

  uint64_t second_uptime = bundle_120_ms->period_ms + 1;

  memset(buf, 0, BUFFER_SIZE);
  num_peers = 0;

  ASSERT_EQ(
    proton_node_update(&node_, second_uptime, buf, sizeof(buf), &out_len, dest, 1, &num_peers),
    PROTON_OK);
  EXPECT_GT(out_len, 0);
  EXPECT_EQ(num_peers, 1);

  ASSERT_NE(bundle_120_ms, nullptr);
  EXPECT_EQ(bundle_120_ms->last_send_ms, second_uptime);
  EXPECT_EQ(bundle_100_ms->last_send_ms, first_uptime_ms);
}

TEST_F(PeriodicBundleTest, SendMostOverdueBundle)
{
  uint8_t buf[BUFFER_SIZE];
  size_t out_len = 0;
  proton_endpoint_t dest[1];
  size_t num_peers = 0;

  size_t bundle_100_ms_slot;
  const bundle_desc_t * bundle_100_ms =
    proton_registry_get_bundle(node_.registry, PROTON_BUNDLE_100_MS_ID, &bundle_100_ms_slot);
  size_t bundle_120_ms_slot;
  const bundle_desc_t * bundle_120_ms =
    proton_registry_get_bundle(node_.registry, PROTON_BUNDLE_120_MS_ID, &bundle_120_ms_slot);

  ASSERT_NE(bundle_100_ms, nullptr);
  ASSERT_NE(bundle_120_ms, nullptr);

  uint64_t uptime_ms = 1000;

  ASSERT_EQ(
    proton_node_update(&node_, uptime_ms, buf, sizeof(buf), &out_len, dest, 1, &num_peers),
    PROTON_OK);
  EXPECT_GT(out_len, 0);
  EXPECT_EQ(num_peers, 1);

  EXPECT_EQ(bundle_100_ms->last_send_ms, uptime_ms);
  EXPECT_EQ(bundle_120_ms->last_send_ms, 0);
}

TEST_F(PeriodicBundleTest, SendTriggeredBeforeOverdue)
{
  uint8_t buf[BUFFER_SIZE];
  size_t out_len = 0;
  proton_endpoint_t dest[1];
  size_t num_peers = 0;

  size_t bundle_100_ms_slot;
  const bundle_desc_t * bundle_100_ms =
    proton_registry_get_bundle(node_.registry, PROTON_BUNDLE_100_MS_ID, &bundle_100_ms_slot);
  size_t bundle_120_ms_slot;
  const bundle_desc_t * bundle_120_ms =
    proton_registry_get_bundle(node_.registry, PROTON_BUNDLE_120_MS_ID, &bundle_120_ms_slot);

  ASSERT_NE(bundle_100_ms, nullptr);
  ASSERT_NE(bundle_120_ms, nullptr);

  EXPECT_EQ(proton_node_trigger_bundle(&node_, PROTON_BUNDLE_120_MS_ID), PROTON_OK);

  uint64_t uptime_ms = 1000;

  // Even though bundle 100 ms is most overdue, bundle 120 ms has been triggered, so it's taking priority
  ASSERT_EQ(
    proton_node_update(&node_, uptime_ms, buf, sizeof(buf), &out_len, dest, 1, &num_peers),
    PROTON_OK);
  EXPECT_GT(out_len, 0);
  EXPECT_EQ(num_peers, 1);

  EXPECT_EQ(bundle_100_ms->last_send_ms, 0);
  EXPECT_EQ(bundle_120_ms->last_send_ms, uptime_ms);

  // Update again, 100 ms is most overdue due to the fact that it was skipped
  num_peers = 0;
  uint64_t second_uptime = uptime_ms + 1;

  ASSERT_EQ(
    proton_node_update(&node_, second_uptime, buf, sizeof(buf), &out_len, dest, 1, &num_peers),
    PROTON_OK);
  EXPECT_GT(out_len, 0);
  EXPECT_EQ(num_peers, 1);

  EXPECT_EQ(bundle_100_ms->last_send_ms, second_uptime);
  EXPECT_EQ(bundle_120_ms->last_send_ms, uptime_ms);

  // Due to bundle 120 being triggered, its last send was set to the trigger point,
  // so the 100 ms bundle will be sent again
  num_peers = 0;
  uint64_t third_uptime = second_uptime + 100;

  ASSERT_EQ(
    proton_node_update(&node_, third_uptime, buf, sizeof(buf), &out_len, dest, 1, &num_peers),
    PROTON_OK);
  EXPECT_GT(out_len, 0);
  EXPECT_EQ(num_peers, 1);

  EXPECT_EQ(bundle_100_ms->last_send_ms, third_uptime);
  EXPECT_EQ(bundle_120_ms->last_send_ms, uptime_ms);

  // And after the 120 ms period since the trigger point, the 120 ms bundle will finally be sent again
  num_peers = 0;
  uint64_t fourth_uptime = third_uptime + 20;

  ASSERT_EQ(
    proton_node_update(&node_, fourth_uptime, buf, sizeof(buf), &out_len, dest, 1, &num_peers),
    PROTON_OK);
  EXPECT_GT(out_len, 0);
  EXPECT_EQ(num_peers, 1);

  EXPECT_EQ(bundle_100_ms->last_send_ms, third_uptime);
  EXPECT_EQ(bundle_120_ms->last_send_ms, fourth_uptime);
}

TEST_F(PeriodicBundleTest, SendMostOverdueTriggeredBundle)
{
  uint8_t buf[BUFFER_SIZE];
  size_t out_len = 0;
  proton_endpoint_t dest[1];
  size_t num_peers = 0;

  size_t bundle_100_ms_slot;
  const bundle_desc_t * bundle_100_ms =
    proton_registry_get_bundle(node_.registry, PROTON_BUNDLE_100_MS_ID, &bundle_100_ms_slot);
  size_t bundle_120_ms_slot;
  const bundle_desc_t * bundle_120_ms =
    proton_registry_get_bundle(node_.registry, PROTON_BUNDLE_120_MS_ID, &bundle_120_ms_slot);

  ASSERT_NE(bundle_100_ms, nullptr);
  ASSERT_NE(bundle_120_ms, nullptr);

  EXPECT_EQ(proton_node_trigger_bundle(&node_, PROTON_BUNDLE_100_MS_ID), PROTON_OK);
  EXPECT_EQ(proton_node_trigger_bundle(&node_, PROTON_BUNDLE_120_MS_ID), PROTON_OK);

  uint64_t uptime_ms = 1000;

  // 100 ms is most overdue of the triggered bundles
  ASSERT_EQ(
    proton_node_update(&node_, uptime_ms, buf, sizeof(buf), &out_len, dest, 1, &num_peers),
    PROTON_OK);
  EXPECT_GT(out_len, 0);
  EXPECT_EQ(num_peers, 1);

  EXPECT_EQ(bundle_100_ms->last_send_ms, uptime_ms);
  EXPECT_EQ(bundle_120_ms->last_send_ms, 0);
}

int main(int argc, char ** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
