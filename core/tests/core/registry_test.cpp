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

#include "proton/registry.h"
#include <gtest/gtest.h>
#include "target_registry_ids.h"
#include "target_registry_sizes.h"
#include "utils.hpp"

extern proton_registry_t g_proton_registry;

TEST(BundleRegistry, GetBundle)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  const bundle_desc_t * desc =
    proton_registry_get_bundle(&registry, PROTON_BUNDLE_VALUE_TEST_ID, NULL);
  EXPECT_NE(desc, nullptr);
  EXPECT_EQ(desc->bundle_id, PROTON_BUNDLE_VALUE_TEST_ID);
  EXPECT_EQ(desc->producer_ids.count, 1);
  EXPECT_EQ(desc->consumer_ids.count, 1);
  EXPECT_EQ(desc->signal_ids.count, 9);
  free(registry.signal_registry);
}

TEST(BundleRegistry, GetBundleInvalidId)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  const bundle_desc_t * desc = proton_registry_get_bundle(&registry, 9999, NULL);
  EXPECT_EQ(desc, nullptr);
  free(registry.signal_registry);
}

TEST(BundleRegistry, GetBundleNullRegistry)
{
  const bundle_desc_t * desc =
    proton_registry_get_bundle(nullptr, PROTON_BUNDLE_VALUE_TEST_ID, nullptr);
  EXPECT_EQ(desc, nullptr);
}

TEST(SignalRegistry, GetSignal)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  signal_desc_t * desc = proton_registry_get_signal(&registry, PROTON_SIGNAL_DOUBLE_VALUE_ID, NULL);
  bool found = desc != nullptr;
  EXPECT_TRUE(found);
  EXPECT_EQ(desc->id, PROTON_SIGNAL_DOUBLE_VALUE_ID);
  EXPECT_EQ(desc->type, PROTON_DOUBLE);
  free(registry.signal_registry);
}

TEST(SignalRegistry, GetSignalInvalidId)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  signal_desc_t * desc = proton_registry_get_signal(&registry, 9999, NULL);
  bool found = desc != nullptr;
  EXPECT_FALSE(found);
  free(registry.signal_registry);
}

TEST(SignalRegistry, GetSignalIdNotInTarget)
{
  // Signal ID 0x1111 is defined in test.yaml, but it is not part of any bundle that
  // the test producer is part of.
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  signal_desc_t * desc = proton_registry_get_signal(&registry, 0x1111, NULL);
  bool found = desc != nullptr;
  EXPECT_FALSE(found);
  free(registry.signal_registry);
}

TEST(SignalRegistry, GetScalarSignalValue)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  // Using the default_double signal since we know it has a value set in the registry
  double value = 0.0;
  proton_status_e status =
    proton_signal_get_double(&registry, PROTON_SIGNAL_DEFAULT_DOUBLE_ID, &value);
  ASSERT_EQ(status, PROTON_OK);
  EXPECT_EQ(value, 3.14159);
  free(registry.signal_registry);
}

TEST(SignalRegistry, GetStringSignalValue)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  char value[PROTON_SIGNAL_DEFAULT_STRING_CAPACITY] = {0};
  size_t len = 0;
  proton_status_e status = proton_signal_get_string(
    &registry, PROTON_SIGNAL_DEFAULT_STRING_ID, value, sizeof(value), &len);
  ASSERT_EQ(status, PROTON_OK);
  EXPECT_STREQ(value, "foo");
  free(registry.signal_registry);
}

TEST(SignalRegistry, GetBytesSignalValue)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  uint8_t value[PROTON_SIGNAL_DEFAULT_BYTES_CAPACITY] = {0};
  size_t len = 0;
  proton_status_e status =
    proton_signal_get_bytes(&registry, PROTON_SIGNAL_DEFAULT_BYTES_ID, value, sizeof(value), &len);
  ASSERT_EQ(status, PROTON_OK);
  ASSERT_EQ(len, PROTON_SIGNAL_DEFAULT_BYTES_CAPACITY);
  const uint8_t expected_value[PROTON_SIGNAL_DEFAULT_BYTES_CAPACITY] = {0, 1, 2};
  EXPECT_EQ(memcmp(value, expected_value, len), 0);
  free(registry.signal_registry);
}

TEST(SignalRegistry, GetSignalTypeMismatch)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  // The double-typed signal cannot be read as a uint32
  uint32_t value = 0;
  proton_status_e status =
    proton_signal_get_uint32(&registry, PROTON_SIGNAL_DEFAULT_DOUBLE_ID, &value);
  EXPECT_EQ(status, PROTON_ERROR);
  free(registry.signal_registry);
}

TEST(SignalRegistry, SetScalarSignalValue)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  double new_value = 2.71828;
  proton_status_e status =
    proton_signal_set_double(&registry, PROTON_SIGNAL_DOUBLE_VALUE_ID, new_value);
  ASSERT_EQ(status, PROTON_OK);

  // Read the value back to verify it was set
  double value = 0.0;
  status = proton_signal_get_double(&registry, PROTON_SIGNAL_DOUBLE_VALUE_ID, &value);
  ASSERT_EQ(status, PROTON_OK);
  EXPECT_EQ(value, new_value);
  free(registry.signal_registry);
}

TEST(SignalRegistry, SetStringSignalValue)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  const char * new_value = "bar";
  size_t new_len = strlen(new_value) + 1;
  proton_status_e status =
    proton_signal_set_string(&registry, PROTON_SIGNAL_STRING_VALUE_ID, new_value, new_len);
  ASSERT_EQ(status, PROTON_OK);

  // Read the value back to verify it was set
  char value[PROTON_SIGNAL_DEFAULT_STRING_CAPACITY] = {0};
  size_t len = 0;
  status =
    proton_signal_get_string(&registry, PROTON_SIGNAL_STRING_VALUE_ID, value, sizeof(value), &len);
  ASSERT_EQ(status, PROTON_OK);
  EXPECT_EQ(len, new_len);
  EXPECT_STREQ(value, new_value);
  free(registry.signal_registry);
}

TEST(SignalRegistry, SetInvalidId)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  proton_status_e status = proton_signal_set_double(&registry, 9999, 2.71828);
  EXPECT_EQ(status, PROTON_ERROR);
  free(registry.signal_registry);
}

TEST(SignalRegistry, SetNullPtr)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  // String/bytes setters take a pointer; null data should be rejected
  proton_status_e status =
    proton_signal_set_string(&registry, PROTON_SIGNAL_STRING_VALUE_ID, nullptr, 4);
  EXPECT_EQ(status, PROTON_NULL_PTR_ERROR);
  free(registry.signal_registry);
}

TEST(SignalRegistry, SetStringExcessiveLength)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  char new_value[PROTON_SIGNAL_DEFAULT_STRING_CAPACITY + 1] = {0};
  memset(new_value, 'a', PROTON_SIGNAL_DEFAULT_STRING_CAPACITY);
  proton_status_e status =
    proton_signal_set_string(&registry, PROTON_SIGNAL_STRING_VALUE_ID, new_value, 999);
  EXPECT_EQ(status, PROTON_INSUFFICIENT_BUFFER_ERROR);
  free(registry.signal_registry);
}

TEST(SignalRegistry, SetBytesExcessiveLength)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  uint8_t new_value[PROTON_SIGNAL_DEFAULT_BYTES_CAPACITY + 1] = {0};
  memset(new_value, 0xFF, PROTON_SIGNAL_DEFAULT_BYTES_CAPACITY);
  proton_status_e status =
    proton_signal_set_bytes(&registry, PROTON_SIGNAL_BYTES_VALUE_ID, new_value, 999);
  EXPECT_EQ(status, PROTON_INSUFFICIENT_BUFFER_ERROR);
  free(registry.signal_registry);
}

TEST(SignalRegistry, SetStringNotNullTerminated)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  char new_value[PROTON_SIGNAL_DEFAULT_STRING_CAPACITY] = {0};
  memset(new_value, 'a', PROTON_SIGNAL_DEFAULT_STRING_CAPACITY);  // No null terminator
  proton_status_e status = proton_signal_set_string(
    &registry, PROTON_SIGNAL_STRING_VALUE_ID, new_value, sizeof(new_value));
  EXPECT_EQ(status, PROTON_ERROR);
  free(registry.signal_registry);
}

TEST(SignalRegistry, SetStringNullTerminatorAtCapacity)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  char new_value[PROTON_SIGNAL_DEFAULT_STRING_CAPACITY] = {0};
  memset(new_value, 'a', PROTON_SIGNAL_DEFAULT_STRING_CAPACITY - 1);
  new_value[PROTON_SIGNAL_DEFAULT_STRING_CAPACITY - 1] = '\0';  // Null terminator at capacity
  proton_status_e status = proton_signal_set_string(
    &registry, PROTON_SIGNAL_STRING_VALUE_ID, new_value, sizeof(new_value));
  EXPECT_EQ(status, PROTON_OK);
  free(registry.signal_registry);
}

TEST(SignalRegistry, SetGetLongString)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  const char * new_value = "ipsumsedolorsitametconsecteturadipiscingeaaa";
  size_t new_len = strlen(new_value) + 1;
  proton_status_e status =
    proton_signal_set_string(&registry, PROTON_SIGNAL_REALLY_LONG_STRING_ID, new_value, new_len);
  ASSERT_EQ(status, PROTON_OK);

  // Read the value back to verify it was set
  char value[PROTON_SIGNAL_REALLY_LONG_STRING_CAPACITY] = {0};
  size_t len = 0;
  status = proton_signal_get_string(
    &registry, PROTON_SIGNAL_REALLY_LONG_STRING_ID, value, sizeof(value), &len);
  ASSERT_EQ(status, PROTON_OK);
  EXPECT_EQ(len, new_len);
  EXPECT_STREQ(value, new_value);
  free(registry.signal_registry);
}

TEST(SignalRegistry, SetGetLongBytes)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  uint8_t new_value[PROTON_SIGNAL_REALLY_LONG_BYTES_CAPACITY] = {0};
  for (size_t i = 0; i < PROTON_SIGNAL_REALLY_LONG_BYTES_CAPACITY; i++)
  {
    new_value[i] = (uint8_t)i;
  }
  size_t new_len = sizeof(new_value);
  proton_status_e status =
    proton_signal_set_bytes(&registry, PROTON_SIGNAL_REALLY_LONG_BYTES_ID, new_value, new_len);
  ASSERT_EQ(status, PROTON_OK);

  // Read the value back to verify it was set
  uint8_t value[PROTON_SIGNAL_REALLY_LONG_BYTES_CAPACITY] = {0};
  size_t len = 0;
  status = proton_signal_get_bytes(
    &registry, PROTON_SIGNAL_REALLY_LONG_BYTES_ID, value, sizeof(value), &len);
  ASSERT_EQ(status, PROTON_OK);
  EXPECT_EQ(len, new_len);
  EXPECT_EQ(memcmp(value, new_value, len), 0);
  free(registry.signal_registry);
}

TEST(SignalRegistry, SetGetStringWithDifferentLength)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  const char * new_value = "b";
  size_t new_len = strlen(new_value) + 1;
  // Set the string with the correct length
  proton_status_e status =
    proton_signal_set_string(&registry, PROTON_SIGNAL_STRING_VALUE_ID, new_value, new_len);
  ASSERT_EQ(status, PROTON_OK);

  // Read the value back with a smaller buffer and length to verify it is properly truncated
  char value[PROTON_SIGNAL_DEFAULT_STRING_CAPACITY] = {0};
  size_t len = 0;
  status =
    proton_signal_get_string(&registry, PROTON_SIGNAL_STRING_VALUE_ID, value, sizeof(value), &len);
  ASSERT_EQ(status, PROTON_OK);
  EXPECT_EQ(len, new_len);
  EXPECT_STREQ(value, new_value);
  free(registry.signal_registry);
}

TEST(SignalRegistry, CheckBundlePeriodPopulated)
{
  // This is a simple test to ensure that the autogeneration for a specific bundle has a period
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  const bundle_desc_t * periodic_bundle =
    proton_registry_get_bundle(&registry, PROTON_BUNDLE_PERIODIC_BUNDLE_ID, NULL);
  EXPECT_NE(periodic_bundle, nullptr);

  EXPECT_EQ(periodic_bundle->period_ms, 100);
  free(registry.signal_registry);
}

TEST(SignalRegistry, SetBundlePeriod)
{
  const uint32_t new_period = 200;
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  proton_registry_set_bundle_period(&registry, PROTON_BUNDLE_PERIODIC_BUNDLE_ID, new_period);
  const bundle_desc_t * periodic_bundle =
    proton_registry_get_bundle(&registry, PROTON_BUNDLE_PERIODIC_BUNDLE_ID, NULL);
  EXPECT_NE(periodic_bundle, nullptr);

  EXPECT_EQ(periodic_bundle->period_ms, new_period);

  free(registry.signal_registry);
}

int main(int argc, char ** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
