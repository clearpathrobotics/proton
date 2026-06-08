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

#ifndef PROTON_REGISTRY_LOCK_HPP
#define PROTON_REGISTRY_LOCK_HPP

#include "proton/registry.h"

namespace proton
{

/**
 * @class RegistryLock class for proton_registry_t optional mutex.
 * For using within std::lock_guard or std::unique_lock
 */
class RegistryLock
{
public:
  explicit RegistryLock(const proton_registry_t * registry) noexcept : registry_(registry) {}

  ~RegistryLock() noexcept = default;

  void lock() noexcept { proton_lock_registry(registry_); }
  void unlock() noexcept { proton_unlock_registry(registry_); }

  RegistryLock(const RegistryLock &) = delete;
  RegistryLock & operator=(const RegistryLock &) = delete;

private:
  const proton_registry_t * registry_;
};

/**
 * @class ScopedLock class for RAII-style locking of proton_registry_t optional mutex.
 * Automatically releases the lock when going out of scope.
 */
class ScopedLock
{
public:
  explicit ScopedLock(const proton_registry_t * registry) noexcept : registry_(registry)
  {
    acquired_ = proton_lock_registry(registry_) == PROTON_OK;
  }

  ~ScopedLock() noexcept
  {
    if (acquired_)
    {
      proton_unlock_registry(registry_);
    }
  }

  ScopedLock(const ScopedLock &) = delete;
  ScopedLock & operator=(const ScopedLock &) = delete;

  bool ok() const noexcept { return acquired_; }

  explicit operator bool() const noexcept { return acquired_; }

private:
  const proton_registry_t * registry_;
  bool acquired_;
};

}  // namespace proton

#endif  // PROTON_REGISTRY_LOCK_HPP
