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
    // bundle_table is now deep-copied by copy_default_registry.
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

// -----------------------------------------------------------------------
// Integer wraparound tests
//
// These tests verify that the unsigned subtraction (uptime_ms - last_send_ms)
// in proton_bundle_overdue_ms handles uint64_t rollover correctly, and that
// triggered bundles fired before their period elapses do not corrupt the
// overdue prioritization through underflow.
// -----------------------------------------------------------------------

// A periodic bundle whose last_send_ms is just before UINT64_MAX should still
// be sent once uptime_ms wraps past the due point.
TEST_F(PeriodicBundleTest, Wraparound_PeriodicBundle_SentAfterThreshold)
{
  size_t slot;
  bundle_desc_t * b = const_cast<bundle_desc_t *>(
    proton_registry_get_bundle(node_.registry, PROTON_BUNDLE_100_MS_ID, &slot));
  ASSERT_NE(b, nullptr);

  // Place last_send_ms 50 ms before UINT64_MAX. Due point wraps to uptime_ms == 50.
  b->last_send_ms = UINT64_MAX - 50;

  // uptime_ms = 51 → elapsed = 51 - (UINT64_MAX - 50) = 102 ms via unsigned modular arithmetic.
  // 102 >= 100 → bundle is due.
  uint8_t buf[BUFFER_SIZE];
  size_t out_len = 0;
  proton_endpoint_t dest[1];
  size_t num_peers = 0;
  ASSERT_EQ(
    proton_node_update(&node_, 51, buf, sizeof(buf), &out_len, dest, 1, &num_peers), PROTON_OK);

  EXPECT_GT(out_len, 0);
  EXPECT_EQ(b->last_send_ms, 51ULL);
}

// The same bundle must NOT be sent one millisecond before its due point crosses
// the UINT64_MAX boundary.
TEST_F(PeriodicBundleTest, Wraparound_PeriodicBundle_NotSentBeforeThreshold)
{
  size_t slot;
  bundle_desc_t * b = const_cast<bundle_desc_t *>(
    proton_registry_get_bundle(node_.registry, PROTON_BUNDLE_100_MS_ID, &slot));
  ASSERT_NE(b, nullptr);

  b->last_send_ms = UINT64_MAX - 50;

  // uptime_ms = 48 → elapsed = 48 + 51 = 99 ms < 100 ms period → not yet due.
  uint8_t buf[BUFFER_SIZE];
  size_t out_len = 0;
  proton_endpoint_t dest[1];
  size_t num_peers = 0;
  ASSERT_EQ(
    proton_node_update(&node_, 48, buf, sizeof(buf), &out_len, dest, 1, &num_peers), PROTON_OK);

  EXPECT_EQ(out_len, 0);
  EXPECT_EQ(b->last_send_ms, UINT64_MAX - 50);  // unchanged
}

// A triggered bundle that has not yet reached its period should still be sent
// immediately. This test has no wraparound on
// uptime_ms, isolating the early-trigger underflow case.
TEST_F(PeriodicBundleTest, Wraparound_TriggeredEarly_StillSent)
{
  size_t slot;
  bundle_desc_t * b = const_cast<bundle_desc_t *>(
    proton_registry_get_bundle(node_.registry, PROTON_BUNDLE_100_MS_ID, &slot));
  ASSERT_NE(b, nullptr);

  // 50 ms elapsed, period is 100 ms → triggered before due.
  b->last_send_ms = 900;

  proton_node_trigger_bundle(&node_, PROTON_BUNDLE_100_MS_ID);

  uint8_t buf[BUFFER_SIZE];
  size_t out_len = 0;
  proton_endpoint_t dest[1];
  size_t num_peers = 0;
  ASSERT_EQ(
    proton_node_update(&node_, 950, buf, sizeof(buf), &out_len, dest, 1, &num_peers), PROTON_OK);

  EXPECT_GT(out_len, 0);
  EXPECT_EQ(b->last_send_ms, 950ULL);
}

// Two triggered bundles where one was last sent just before UINT64_MAX and one
// was last sent much earlier. The bundle with genuinely more elapsed time
// (and therefore higher overdue value) must win the priority contest.
TEST_F(PeriodicBundleTest, Wraparound_TwoTriggeredBundles_MostOverdueWins)
{
  size_t slot_100, slot_120;
  bundle_desc_t * b100 = const_cast<bundle_desc_t *>(
    proton_registry_get_bundle(node_.registry, PROTON_BUNDLE_100_MS_ID, &slot_100));
  bundle_desc_t * b120 = const_cast<bundle_desc_t *>(
    proton_registry_get_bundle(node_.registry, PROTON_BUNDLE_120_MS_ID, &slot_120));
  ASSERT_NE(b100, nullptr);
  ASSERT_NE(b120, nullptr);

  // b100: last_send = UINT64_MAX - 200. At uptime=10: elapsed = 211 ms, overdue = 111 ms.
  b100->last_send_ms = UINT64_MAX - 200;
  // b120: last_send = UINT64_MAX - 50.  At uptime=10: elapsed = 61 ms < 120 ms, overdue = 0.
  // Old code: overdue = 61 - 120 = UINT64_MAX - 58  → b120 would wrongly win.
  // Fixed:    overdue = 0                            → b100 wins correctly.
  b120->last_send_ms = UINT64_MAX - 50;

  ASSERT_EQ(proton_node_trigger_bundle(&node_, PROTON_BUNDLE_100_MS_ID), PROTON_OK);
  ASSERT_EQ(proton_node_trigger_bundle(&node_, PROTON_BUNDLE_120_MS_ID), PROTON_OK);

  uint8_t buf[BUFFER_SIZE];
  size_t out_len = 0;
  proton_endpoint_t dest[1];
  size_t num_peers = 0;
  ASSERT_EQ(
    proton_node_update(&node_, 10, buf, sizeof(buf), &out_len, dest, 1, &num_peers), PROTON_OK);

  EXPECT_GT(out_len, 0);
  EXPECT_EQ(num_peers, 1);
  // b100 has overdue = 111 ms; b120 has overdue = 0 → b100 must be selected.
  EXPECT_EQ(b100->last_send_ms, 10ULL);
  EXPECT_NE(b120->last_send_ms, 10ULL);
}

int main(int argc, char ** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
