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

TEST(BundleRegistry, GetBundle)
{
  const bundle_desc_t * desc = proton_registry_get_bundle(PROTON_BUNDLE_VALUE_TEST_ID);
  EXPECT_NE(desc, nullptr);
  EXPECT_EQ(desc->bundle_id, PROTON_BUNDLE_VALUE_TEST_ID);
  EXPECT_EQ(desc->producer_ids.count, 1);
  // EXPECT_EQ(desc->producer_ids.ids[0], PROTON_TEST_PRODUCER_ID);
  EXPECT_EQ(desc->consumer_ids.count, 1);
  EXPECT_EQ(desc->signal_ids.count, 9);
}

TEST(BundleRegistry, GetBundleInvalidId)
{
  const bundle_desc_t * desc = proton_registry_get_bundle(9999);
  EXPECT_EQ(desc, nullptr);
}

TEST(SignalRegistry, GetSignal)
{
  signal_desc_t desc;
  bool found = proton_registry_get_signal(PROTON_SIGNAL_DOUBLE_VALUE_ID, &desc);
  EXPECT_TRUE(found);
  EXPECT_EQ(desc.id, PROTON_SIGNAL_DOUBLE_VALUE_ID);
  EXPECT_EQ(desc.type, PROTON_DOUBLE);
}

TEST(SignalRegistry, GetSignalInvalidId)
{
  signal_desc_t desc;
  bool found = proton_registry_get_signal(9999, &desc);
  EXPECT_FALSE(found);
}

TEST(SignalRegistry, GetSignalNullPtr)
{
  bool found = proton_registry_get_signal(PROTON_SIGNAL_DOUBLE_VALUE_ID, nullptr);
  EXPECT_FALSE(found);
}

TEST(SignalRegistry, GetScalarSignalValue)
{
  // Using the default_double signal since we know it has a value set in the registry
  double value;
  size_t len;
  proton_signal_type_e type;
  bool found =
    proton_signal_get_value(PROTON_SIGNAL_DEFAULT_DOUBLE_ID, (void *)&value, &len, &type);
  EXPECT_TRUE(found);
  EXPECT_EQ(type, PROTON_DOUBLE);
  EXPECT_EQ(len, sizeof(double));
  EXPECT_EQ(value, 3.14159);
}

TEST(SignalRegistry, GetStringSignalValue)
{
  char value[PROTON_SIGNAL_DEFAULT_STRING_CAPACITY];
  size_t len;
  proton_signal_type_e type;
  bool found = proton_signal_get_value(PROTON_SIGNAL_DEFAULT_STRING_ID, (void *)value, &len, &type);
  EXPECT_TRUE(found);
  EXPECT_EQ(type, PROTON_STRING);
  EXPECT_EQ(len, PROTON_SIGNAL_DEFAULT_STRING_CAPACITY);
  EXPECT_STREQ(value, "foo");
}

TEST(SignalRegistry, GetBytesSignalValue)
{
  uint8_t value[PROTON_SIGNAL_DEFAULT_BYTES_CAPACITY];
  size_t len;
  proton_signal_type_e type;
  bool found = proton_signal_get_value(PROTON_SIGNAL_DEFAULT_BYTES_ID, (void *)value, &len, &type);
  EXPECT_TRUE(found);
  EXPECT_EQ(type, PROTON_BYTES);
  EXPECT_EQ(len, PROTON_SIGNAL_DEFAULT_BYTES_CAPACITY);
  const uint8_t expected_value[PROTON_SIGNAL_DEFAULT_BYTES_CAPACITY] = {0, 1, 2};
  EXPECT_EQ(memcmp(value, expected_value, PROTON_SIGNAL_DEFAULT_BYTES_CAPACITY), 0);
}

TEST(SignalRegistry, SetScalarSignalValue)
{
  double new_value = 2.71828;
  bool success =
    proton_signal_set_value(PROTON_SIGNAL_DOUBLE_VALUE_ID, (void *)&new_value, sizeof(new_value));
  EXPECT_TRUE(success);

  // Read the value back to verify it was set
  double value;
  size_t len;
  proton_signal_type_e type;
  bool found = proton_signal_get_value(PROTON_SIGNAL_DOUBLE_VALUE_ID, (void *)&value, &len, &type);
  EXPECT_TRUE(found);
  EXPECT_EQ(type, PROTON_DOUBLE);
  EXPECT_EQ(len, sizeof(double));
  EXPECT_EQ(value, new_value);
}

TEST(SignalRegistry, SetStringSignalValue)
{
  const char * new_value = "bar";
  bool success = proton_signal_set_value(
    PROTON_SIGNAL_STRING_VALUE_ID, (void *)new_value, strlen(new_value) + 1);
  EXPECT_TRUE(success);

  // Read the value back to verify it was set
  char value[PROTON_SIGNAL_DEFAULT_STRING_CAPACITY];
  size_t len;
  proton_signal_type_e type;
  bool found = proton_signal_get_value(PROTON_SIGNAL_STRING_VALUE_ID, (void *)value, &len, &type);
  EXPECT_TRUE(found);
  EXPECT_EQ(type, PROTON_STRING);
  EXPECT_EQ(len, PROTON_SIGNAL_DEFAULT_STRING_CAPACITY);
  EXPECT_STREQ(value, new_value);
}

TEST(SignalRegistry, SetInvalidId)
{
  double new_value = 2.71828;
  bool success = proton_signal_set_value(9999, (void *)&new_value, sizeof(new_value));
  EXPECT_FALSE(success);
}

TEST(SignalRegistry, SetNullPtr)
{
  bool success = proton_signal_set_value(PROTON_SIGNAL_DOUBLE_VALUE_ID, nullptr, sizeof(double));
  EXPECT_FALSE(success);
}

TEST(SignalRegistry, SetStringExcessiveLength)
{
  char new_value[PROTON_SIGNAL_DEFAULT_STRING_CAPACITY + 1] = {0};
  memset(
    new_value, 'a',
    PROTON_SIGNAL_DEFAULT_STRING_CAPACITY);  // Fill with 'a's to ensure it's not just a shorter string
  bool success = proton_signal_set_value(PROTON_SIGNAL_STRING_VALUE_ID, (void *)new_value, 999);
  EXPECT_FALSE(success);
}

TEST(SignalRegistry, SetBytesExcessiveLength)
{
  uint8_t new_value[PROTON_SIGNAL_DEFAULT_BYTES_CAPACITY + 1] = {0};
  memset(
    new_value, 0xFF,
    PROTON_SIGNAL_DEFAULT_BYTES_CAPACITY);  // Fill with 0xFF to ensure it's not just a shorter array
  bool success = proton_signal_set_value(PROTON_SIGNAL_BYTES_VALUE_ID, (void *)new_value, 999);
  EXPECT_FALSE(success);
}

int main(int argc, char ** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
