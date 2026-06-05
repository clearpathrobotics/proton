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
#include "protoncpp/signal_access.hpp"
#include "target_registry_ids.h"
#include "target_registry_sizes.h"
#include "utils.hpp"

extern proton_registry_t g_proton_registry;

using namespace proton;

TEST(SignalAccess, GetDouble)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);

  double value;
  SignalAccess access(&registry);
  proton_status_e status = access.get(PROTON_SIGNAL_DEFAULT_DOUBLE_ID, value);
  EXPECT_EQ(status, PROTON_OK);
  EXPECT_EQ(value, 3.14159);

  free(registry.signal_registry);
}

TEST(SignalAccess, SetDouble)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);

  double new_value = 2.71828;
  SignalAccess access(&registry);
  proton_status_e status = access.set(PROTON_SIGNAL_DOUBLE_VALUE_ID, new_value);
  ASSERT_EQ(status, PROTON_OK);

  // Read the value back to verify it was set
  double value;
  status = access.get(PROTON_SIGNAL_DOUBLE_VALUE_ID, value);
  ASSERT_EQ(status, PROTON_OK);
  EXPECT_EQ(value, new_value);

  free(registry.signal_registry);
}

TEST(SignalAccess, GetSetFloat)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  SignalAccess access(&registry);

  float new_value = 1.5f;
  proton_status_e status = access.set(PROTON_SIGNAL_FLOAT_VALUE_ID, new_value);
  ASSERT_EQ(status, PROTON_OK);

  float value;
  status = access.get(PROTON_SIGNAL_FLOAT_VALUE_ID, value);
  ASSERT_EQ(status, PROTON_OK);
  EXPECT_FLOAT_EQ(value, new_value);

  free(registry.signal_registry);
}

TEST(SignalAccess, GetSetInt32)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  SignalAccess access(&registry);

  int32_t new_value = -42;
  proton_status_e status = access.set(PROTON_SIGNAL_INT32_VALUE_ID, new_value);
  ASSERT_EQ(status, PROTON_OK);

  int32_t value;
  status = access.get(PROTON_SIGNAL_INT32_VALUE_ID, value);
  ASSERT_EQ(status, PROTON_OK);
  EXPECT_EQ(value, new_value);

  free(registry.signal_registry);
}

TEST(SignalAccess, GetSetInt64)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  SignalAccess access(&registry);

  int64_t new_value = -9223372036854775807LL;
  proton_status_e status = access.set(PROTON_SIGNAL_INT64_VALUE_ID, new_value);
  ASSERT_EQ(status, PROTON_OK);

  int64_t value;
  status = access.get(PROTON_SIGNAL_INT64_VALUE_ID, value);
  ASSERT_EQ(status, PROTON_OK);
  EXPECT_EQ(value, new_value);

  free(registry.signal_registry);
}

TEST(SignalAccess, GetSetUint32)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  SignalAccess access(&registry);

  uint32_t new_value = 0xDEADBEEF;
  proton_status_e status = access.set(PROTON_SIGNAL_UINT32_VALUE_ID, new_value);
  ASSERT_EQ(status, PROTON_OK);

  uint32_t value;
  status = access.get(PROTON_SIGNAL_UINT32_VALUE_ID, value);
  ASSERT_EQ(status, PROTON_OK);
  EXPECT_EQ(value, new_value);

  free(registry.signal_registry);
}

TEST(SignalAccess, GetSetUint64)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  SignalAccess access(&registry);

  uint64_t new_value = 0xDEADBEEFCAFEBABEULL;
  proton_status_e status = access.set(PROTON_SIGNAL_UINT64_VALUE_ID, new_value);
  ASSERT_EQ(status, PROTON_OK);

  uint64_t value;
  status = access.get(PROTON_SIGNAL_UINT64_VALUE_ID, value);
  ASSERT_EQ(status, PROTON_OK);
  EXPECT_EQ(value, new_value);

  free(registry.signal_registry);
}

TEST(SignalAccess, GetSetBool)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  SignalAccess access(&registry);

  bool new_value = true;
  proton_status_e status = access.set(PROTON_SIGNAL_BOOL_VALUE_ID, new_value);
  ASSERT_EQ(status, PROTON_OK);

  bool value;
  status = access.get(PROTON_SIGNAL_BOOL_VALUE_ID, value);
  ASSERT_EQ(status, PROTON_OK);
  EXPECT_EQ(value, new_value);

  // Test false as well
  status = access.set(PROTON_SIGNAL_BOOL_VALUE_ID, false);
  ASSERT_EQ(status, PROTON_OK);
  status = access.get(PROTON_SIGNAL_BOOL_VALUE_ID, value);
  ASSERT_EQ(status, PROTON_OK);
  EXPECT_FALSE(value);

  free(registry.signal_registry);
}

TEST(SignalAccess, GetDefaultString)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  SignalAccess access(&registry);

  char buf[PROTON_SIGNAL_DEFAULT_STRING_CAPACITY] = {0};
  size_t len = 0;
  proton_status_e status = access.get(PROTON_SIGNAL_DEFAULT_STRING_ID, buf, sizeof(buf), len);
  ASSERT_EQ(status, PROTON_OK);
  EXPECT_STREQ(buf, "foo");
  EXPECT_EQ(len, strlen(buf) + 1);

  free(registry.signal_registry);
}

TEST(SignalAccess, SetString)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  SignalAccess access(&registry);

  const char * new_value = "bar";
  proton_status_e status =
    access.set(PROTON_SIGNAL_STRING_VALUE_ID, new_value, strlen(new_value) + 1);
  ASSERT_EQ(status, PROTON_OK);

  char buf[PROTON_SIGNAL_STRING_VALUE_CAPACITY] = {0};
  size_t len = 0;
  status = access.get(PROTON_SIGNAL_STRING_VALUE_ID, buf, sizeof(buf), len);
  ASSERT_EQ(status, PROTON_OK);
  EXPECT_STREQ(buf, new_value);
  EXPECT_EQ(len, strlen(new_value) + 1);

  free(registry.signal_registry);
}

TEST(SignalAccess, GetDefaultBytes)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  SignalAccess access(&registry);

  uint8_t buf[PROTON_SIGNAL_DEFAULT_BYTES_CAPACITY] = {0};
  size_t len = 0;
  proton_status_e status = access.get(PROTON_SIGNAL_DEFAULT_BYTES_ID, buf, sizeof(buf), len);
  ASSERT_EQ(status, PROTON_OK);
  EXPECT_EQ(len, 3);
  EXPECT_EQ(buf[0], 0);
  EXPECT_EQ(buf[1], 1);
  EXPECT_EQ(buf[2], 2);

  free(registry.signal_registry);
}

TEST(SignalAccess, SetBytes)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  SignalAccess access(&registry);

  uint8_t new_value[] = {0xAA, 0xBB, 0xCC, 0xDD};
  proton_status_e status = access.set(PROTON_SIGNAL_BYTES_VALUE_ID, new_value, sizeof(new_value));
  ASSERT_EQ(status, PROTON_OK);

  uint8_t buf[PROTON_SIGNAL_BYTES_VALUE_CAPACITY] = {0};
  size_t len = 0;
  status = access.get(PROTON_SIGNAL_BYTES_VALUE_ID, buf, sizeof(buf), len);
  ASSERT_EQ(status, PROTON_OK);
  EXPECT_EQ(len, sizeof(new_value));
  EXPECT_EQ(memcmp(buf, new_value, len), 0);

  free(registry.signal_registry);
}

TEST(SignalAccess, GetInvalidSignalId)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  SignalAccess access(&registry);

  double value;
  proton_status_e status = access.get(0x9999, value);
  EXPECT_EQ(status, PROTON_ERROR);

  free(registry.signal_registry);
}

int main(int argc, char ** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
