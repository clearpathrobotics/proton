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
#include "protoncpp/registry_lock.hpp"

using namespace proton;

TEST(RegistryLock, LockUnlock)
{
  proton_registry_t registry;
  registry.mutex_handles.lock = [](void * mutex, void * ctx) -> proton_status_e
  {
    (void)ctx;
    // Using mutex as a lock count for testing purposes
    int * lock_count = static_cast<int *>(mutex);
    *lock_count = 1;
    return PROTON_OK;
  };

  registry.mutex_handles.unlock = [](void * mutex, void * ctx) -> proton_status_e
  {
    (void)mutex;
    int * unlock_count = static_cast<int *>(ctx);
    *unlock_count = 1;
    return PROTON_OK;
  };

  int lock_count = 0;
  int unlock_count = 0;
  registry.mutex_handles.mutex = &lock_count;
  registry.mutex_handles.arg = &unlock_count;

  // Test that we can lock and unlock the registry without error
  RegistryLock lock(&registry);
  EXPECT_NO_THROW(lock.lock());
  EXPECT_NO_THROW(lock.unlock());
  EXPECT_EQ(lock_count, 1);
  EXPECT_EQ(unlock_count, 1);
}

TEST(ScopedLock, LockUnlock)
{
  proton_registry_t registry;
  registry.mutex_handles.lock = [](void * mutex, void * ctx) -> proton_status_e
  {
    (void)ctx;
    int * lock_count = static_cast<int *>(mutex);
    *lock_count = 1;
    return PROTON_OK;
  };

  registry.mutex_handles.unlock = [](void * mutex, void * ctx) -> proton_status_e
  {
    (void)mutex;
    int * unlock_count = static_cast<int *>(ctx);
    *unlock_count = 1;
    return PROTON_OK;
  };

  int lock_count = 0;
  int unlock_count = 0;
  registry.mutex_handles.mutex = &lock_count;
  registry.mutex_handles.arg = &unlock_count;

  // Test that the ScopedLock acquires and releases the lock properly
  {
    ScopedLock lock(&registry);
    EXPECT_EQ(lock_count, 1);
    EXPECT_EQ(unlock_count, 0);
  }
  EXPECT_EQ(unlock_count, 1);
}

TEST(ScopedLock, LockFailure)
{
  proton_registry_t registry;
  registry.mutex_handles.lock = [](void * mutex, void * ctx) -> proton_status_e
  {
    (void)mutex;
    (void)ctx;
    return PROTON_ERROR;  // Simulate lock failure
  };

  registry.mutex_handles.unlock = [](void * mutex, void * ctx) -> proton_status_e
  {
    (void)mutex;
    int * unlock_count = static_cast<int *>(ctx);
    *unlock_count = 1;
    return PROTON_OK;
  };

  int unlock_count = 0;
  registry.mutex_handles.arg = &unlock_count;

  // Test that the ScopedLock handles lock failure gracefully
  {
    ScopedLock lock(&registry);
    EXPECT_FALSE(lock.ok());
    EXPECT_EQ(unlock_count, 0);  // Unlock should not be called if lock failed
  }
  EXPECT_EQ(unlock_count, 0);
}

int main(int argc, char ** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
