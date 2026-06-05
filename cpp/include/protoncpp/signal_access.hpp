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

#ifndef PROTON_SIGNAL_ACCESS_HPP
#define PROTON_SIGNAL_ACCESS_HPP

#include "proton/common.h"
#include "proton/registry.h"

namespace proton
{

class SignalAccess
{
public:
  constexpr explicit SignalAccess(proton_registry_t * registry) noexcept : registry_(registry) {}

  proton_status_e get(uint32_t id, double & out) const noexcept;
  proton_status_e set(uint32_t id, double value) noexcept;

  proton_status_e get(uint32_t id, float & out) const noexcept;
  proton_status_e set(uint32_t id, float value) noexcept;

  proton_status_e get(uint32_t id, int32_t & out) const noexcept;
  proton_status_e set(uint32_t id, int32_t value) noexcept;

  proton_status_e get(uint32_t id, int64_t & out) const noexcept;
  proton_status_e set(uint64_t id, int64_t value) noexcept;

  proton_status_e get(uint32_t id, uint32_t & out) const noexcept;
  proton_status_e set(uint32_t id, uint32_t value) noexcept;

  proton_status_e get(uint32_t id, uint64_t & out) const noexcept;
  proton_status_e set(uint32_t id, uint64_t value) noexcept;

  proton_status_e get(uint32_t id, char * buf, size_t cap, size_t & len) const noexcept;
  proton_status_e set(uint32_t id, const char * buf, size_t len) const noexcept;

  proton_status_e get(uint32_t id, uint8_t * buf, size_t cap, size_t & len) const noexcept;
  proton_status_e set(uint32_t id, const uint8_t * buf, size_t len) const noexcept;

private:
  proton_registry_t * registry_;
};

template <typename T>
class Signal
{
public:
  constexpr explicit Signal(proton_registry_t * registry, uint32_t id)
  : registry_(registry), id_(id)
  {
  }

  uint32_t id() const noexcept { return id_; }

  signal_desc_t * desc() const noexcept
  {
    return proton_registry_get_signal(registry_, id_, nullptr);
  }

  proton_status_e get(T & out) const noexcept { return SignalAccess(registry_).get(id_, out); }

  proton_status_e set(T value) noexcept { return SignalAccess(registry_).set(id_, value); }

private:
  proton_registry_t * registry_;
  uint32_t id_;
};

}  // namespace proton

#endif  // PROTON_SIGNAL_ACCESS_HPP
