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

TEST(SignalAccess, SetInvalidSignalId)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  SignalAccess access(&registry);

  double new_value = 1.23;
  proton_status_e status = access.set(0x9999, new_value);
  EXPECT_EQ(status, PROTON_ERROR);

  free(registry.signal_registry);
}

TEST(SignalAccess, GetSignalIdNotInTarget)
{
  // Signal ID 0x1111 is defined in test.yaml, but it is not part of any bundle that
  // the test producer is part of.
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  SignalAccess access(&registry);

  float value;
  proton_status_e status = access.get(0x1111, value);
  EXPECT_EQ(status, PROTON_ERROR);

  free(registry.signal_registry);
}

TEST(SignalAccess, GetSignalTypeMismatch)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  SignalAccess access(&registry);

  // The double-typed signal cannot be read as a uint32
  uint32_t value;
  proton_status_e status = access.get(PROTON_SIGNAL_DEFAULT_DOUBLE_ID, value);
  EXPECT_EQ(status, PROTON_ERROR);

  free(registry.signal_registry);
}

TEST(SignalAccess, SetSignalTypeMismatch)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  SignalAccess access(&registry);

  // The double-typed signal cannot be written as a uint32
  uint32_t value = 42;
  proton_status_e status = access.set(PROTON_SIGNAL_DEFAULT_DOUBLE_ID, value);
  EXPECT_EQ(status, PROTON_ERROR);

  free(registry.signal_registry);
}

TEST(SignalAccess, SetStringNullPtr)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  SignalAccess access(&registry);

  // String setter with null data should be rejected
  proton_status_e status = access.set(PROTON_SIGNAL_STRING_VALUE_ID, (const char *)nullptr, 4);
  EXPECT_EQ(status, PROTON_NULL_PTR_ERROR);

  free(registry.signal_registry);
}

TEST(SignalAccess, SetBytesNullPtr)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  SignalAccess access(&registry);

  // Bytes setter with null data should be rejected
  proton_status_e status = access.set(PROTON_SIGNAL_BYTES_VALUE_ID, (const uint8_t *)nullptr, 4);
  EXPECT_EQ(status, PROTON_NULL_PTR_ERROR);

  free(registry.signal_registry);
}

TEST(SignalAccess, SetStringExcessiveLength)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  SignalAccess access(&registry);

  char new_value[PROTON_SIGNAL_STRING_VALUE_CAPACITY + 1] = {0};
  memset(new_value, 'a', PROTON_SIGNAL_STRING_VALUE_CAPACITY);
  proton_status_e status = access.set(PROTON_SIGNAL_STRING_VALUE_ID, new_value, 999);
  EXPECT_EQ(status, PROTON_INSUFFICIENT_BUFFER_ERROR);

  free(registry.signal_registry);
}

TEST(SignalAccess, SetBytesExcessiveLength)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  SignalAccess access(&registry);

  uint8_t new_value[PROTON_SIGNAL_BYTES_VALUE_CAPACITY + 1] = {0};
  memset(new_value, 0xFF, PROTON_SIGNAL_BYTES_VALUE_CAPACITY);
  proton_status_e status = access.set(PROTON_SIGNAL_BYTES_VALUE_ID, new_value, 999);
  EXPECT_EQ(status, PROTON_INSUFFICIENT_BUFFER_ERROR);

  free(registry.signal_registry);
}

TEST(SignalAccess, SetStringNotNullTerminated)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  SignalAccess access(&registry);

  char new_value[PROTON_SIGNAL_STRING_VALUE_CAPACITY] = {0};
  memset(new_value, 'a', PROTON_SIGNAL_STRING_VALUE_CAPACITY);  // No null terminator
  proton_status_e status = access.set(PROTON_SIGNAL_STRING_VALUE_ID, new_value, sizeof(new_value));
  EXPECT_EQ(status, PROTON_ERROR);

  free(registry.signal_registry);
}

TEST(SignalAccess, SetStringNullTerminatorAtCapacity)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  SignalAccess access(&registry);

  char new_value[PROTON_SIGNAL_STRING_VALUE_CAPACITY] = {0};
  memset(new_value, 'a', PROTON_SIGNAL_STRING_VALUE_CAPACITY - 1);
  new_value[PROTON_SIGNAL_STRING_VALUE_CAPACITY - 1] = '\0';  // Null terminator at capacity
  proton_status_e status = access.set(PROTON_SIGNAL_STRING_VALUE_ID, new_value, sizeof(new_value));
  EXPECT_EQ(status, PROTON_OK);

  free(registry.signal_registry);
}

TEST(SignalAccess, SetGetLongString)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  SignalAccess access(&registry);

  const char * new_value = "ipsumsedolorsitametconsecteturadipiscingeaaa";
  size_t new_len = strlen(new_value) + 1;
  proton_status_e status = access.set(PROTON_SIGNAL_REALLY_LONG_STRING_ID, new_value, new_len);
  ASSERT_EQ(status, PROTON_OK);

  // Read the value back to verify it was set
  char buf[PROTON_SIGNAL_REALLY_LONG_STRING_CAPACITY] = {0};
  size_t len = 0;
  status = access.get(PROTON_SIGNAL_REALLY_LONG_STRING_ID, buf, sizeof(buf), len);
  ASSERT_EQ(status, PROTON_OK);
  EXPECT_EQ(len, new_len);
  EXPECT_STREQ(buf, new_value);

  free(registry.signal_registry);
}

TEST(SignalAccess, SetGetLongBytes)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  SignalAccess access(&registry);

  uint8_t new_value[PROTON_SIGNAL_REALLY_LONG_BYTES_CAPACITY] = {0};
  memset(new_value, 0xAB, PROTON_SIGNAL_REALLY_LONG_BYTES_CAPACITY);
  proton_status_e status =
    access.set(PROTON_SIGNAL_REALLY_LONG_BYTES_ID, new_value, sizeof(new_value));
  ASSERT_EQ(status, PROTON_OK);

  // Read the value back to verify it was set
  uint8_t buf[PROTON_SIGNAL_REALLY_LONG_BYTES_CAPACITY] = {0};
  size_t len = 0;
  status = access.get(PROTON_SIGNAL_REALLY_LONG_BYTES_ID, buf, sizeof(buf), len);
  ASSERT_EQ(status, PROTON_OK);
  EXPECT_EQ(len, sizeof(new_value));
  EXPECT_EQ(memcmp(buf, new_value, len), 0);

  free(registry.signal_registry);
}

// =============================================================================
// Signal<T> template class tests
// =============================================================================

TEST(Signal, GetDouble)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  Signal<double> signal(&registry, PROTON_SIGNAL_DEFAULT_DOUBLE_ID);

  double value;
  proton_status_e status = signal.get(value);
  EXPECT_EQ(status, PROTON_OK);
  EXPECT_EQ(value, 3.14159);

  free(registry.signal_registry);
}

TEST(Signal, SetDouble)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  Signal<double> signal(&registry, PROTON_SIGNAL_DOUBLE_VALUE_ID);

  double new_value = 2.71828;
  proton_status_e status = signal.set(new_value);
  ASSERT_EQ(status, PROTON_OK);

  double value;
  status = signal.get(value);
  ASSERT_EQ(status, PROTON_OK);
  EXPECT_EQ(value, new_value);

  free(registry.signal_registry);
}

TEST(Signal, GetSetFloat)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  Signal<float> signal(&registry, PROTON_SIGNAL_FLOAT_VALUE_ID);

  float new_value = 1.5f;
  proton_status_e status = signal.set(new_value);
  ASSERT_EQ(status, PROTON_OK);

  float value;
  status = signal.get(value);
  ASSERT_EQ(status, PROTON_OK);
  EXPECT_FLOAT_EQ(value, new_value);

  free(registry.signal_registry);
}

TEST(Signal, GetSetInt32)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  Signal<int32_t> signal(&registry, PROTON_SIGNAL_INT32_VALUE_ID);

  int32_t new_value = -42;
  proton_status_e status = signal.set(new_value);
  ASSERT_EQ(status, PROTON_OK);

  int32_t value;
  status = signal.get(value);
  ASSERT_EQ(status, PROTON_OK);
  EXPECT_EQ(value, new_value);

  free(registry.signal_registry);
}

TEST(Signal, GetSetInt64)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  Signal<int64_t> signal(&registry, PROTON_SIGNAL_INT64_VALUE_ID);

  int64_t new_value = -9223372036854775807LL;
  proton_status_e status = signal.set(new_value);
  ASSERT_EQ(status, PROTON_OK);

  int64_t value;
  status = signal.get(value);
  ASSERT_EQ(status, PROTON_OK);
  EXPECT_EQ(value, new_value);

  free(registry.signal_registry);
}

TEST(Signal, GetSetUint32)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  Signal<uint32_t> signal(&registry, PROTON_SIGNAL_UINT32_VALUE_ID);

  uint32_t new_value = 0xDEADBEEF;
  proton_status_e status = signal.set(new_value);
  ASSERT_EQ(status, PROTON_OK);

  uint32_t value;
  status = signal.get(value);
  ASSERT_EQ(status, PROTON_OK);
  EXPECT_EQ(value, new_value);

  free(registry.signal_registry);
}

TEST(Signal, GetSetUint64)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  Signal<uint64_t> signal(&registry, PROTON_SIGNAL_UINT64_VALUE_ID);

  uint64_t new_value = 0xDEADBEEFCAFEBABEULL;
  proton_status_e status = signal.set(new_value);
  ASSERT_EQ(status, PROTON_OK);

  uint64_t value;
  status = signal.get(value);
  ASSERT_EQ(status, PROTON_OK);
  EXPECT_EQ(value, new_value);

  free(registry.signal_registry);
}

TEST(Signal, GetSetBool)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  Signal<bool> signal(&registry, PROTON_SIGNAL_BOOL_VALUE_ID);

  proton_status_e status = signal.set(true);
  ASSERT_EQ(status, PROTON_OK);

  bool value;
  status = signal.get(value);
  ASSERT_EQ(status, PROTON_OK);
  EXPECT_TRUE(value);

  // Test false as well
  status = signal.set(false);
  ASSERT_EQ(status, PROTON_OK);
  status = signal.get(value);
  ASSERT_EQ(status, PROTON_OK);
  EXPECT_FALSE(value);

  free(registry.signal_registry);
}

TEST(Signal, Id)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  Signal<double> signal(&registry, PROTON_SIGNAL_DOUBLE_VALUE_ID);

  EXPECT_EQ(signal.id(), PROTON_SIGNAL_DOUBLE_VALUE_ID);

  free(registry.signal_registry);
}

TEST(Signal, Desc)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  Signal<double> signal(&registry, PROTON_SIGNAL_DEFAULT_DOUBLE_ID);

  signal_desc_t * desc = signal.desc();
  ASSERT_NE(desc, nullptr);
  EXPECT_EQ(desc->id, PROTON_SIGNAL_DEFAULT_DOUBLE_ID);
  EXPECT_EQ(desc->type, PROTON_DOUBLE);

  free(registry.signal_registry);
}

TEST(Signal, DescInvalidId)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  Signal<double> signal(&registry, 0x9999);

  signal_desc_t * desc = signal.desc();
  EXPECT_EQ(desc, nullptr);

  free(registry.signal_registry);
}

TEST(Signal, GetInvalidSignalId)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  Signal<double> signal(&registry, 0x9999);

  double value;
  proton_status_e status = signal.get(value);
  EXPECT_EQ(status, PROTON_ERROR);

  free(registry.signal_registry);
}

TEST(Signal, SetInvalidSignalId)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  Signal<double> signal(&registry, 0x9999);

  proton_status_e status = signal.set(1.23);
  EXPECT_EQ(status, PROTON_ERROR);

  free(registry.signal_registry);
}

TEST(Signal, GetSignalIdNotInTarget)
{
  // Signal ID 0x1111 is defined in test.yaml, but it is not part of any bundle that
  // the test producer is part of.
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  Signal<float> signal(&registry, 0x1111);

  float value;
  proton_status_e status = signal.get(value);
  EXPECT_EQ(status, PROTON_ERROR);

  free(registry.signal_registry);
}

TEST(Signal, GetSignalTypeMismatch)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  // Create Signal<uint32_t> for a double signal - type mismatch
  Signal<uint32_t> signal(&registry, PROTON_SIGNAL_DEFAULT_DOUBLE_ID);

  uint32_t value;
  proton_status_e status = signal.get(value);
  EXPECT_EQ(status, PROTON_ERROR);

  free(registry.signal_registry);
}

TEST(Signal, SetSignalTypeMismatch)
{
  proton_registry_t registry = copy_default_registry(&g_proton_registry);
  // Create Signal<uint32_t> for a double signal - type mismatch
  Signal<uint32_t> signal(&registry, PROTON_SIGNAL_DEFAULT_DOUBLE_ID);

  proton_status_e status = signal.set(42);
  EXPECT_EQ(status, PROTON_ERROR);

  free(registry.signal_registry);
}

int main(int argc, char ** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
