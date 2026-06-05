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

#include "protoncpp/signal_access.hpp"

namespace proton
{

proton_status_e SignalAccess::get(uint32_t id, double & out) const noexcept
{
  return proton_signal_get_double(registry_, id, &out);
}
proton_status_e SignalAccess::set(uint32_t id, double value) noexcept
{
  return proton_signal_set_double(registry_, id, value);
}

proton_status_e SignalAccess::get(uint32_t id, float & out) const noexcept
{
  return proton_signal_get_float(registry_, id, &out);
}
proton_status_e SignalAccess::set(uint32_t id, float value) noexcept
{
  return proton_signal_set_float(registry_, id, value);
}

proton_status_e SignalAccess::get(uint32_t id, int32_t & out) const noexcept
{
  return proton_signal_get_int32(registry_, id, &out);
}
proton_status_e SignalAccess::set(uint32_t id, int32_t value) noexcept
{
  return proton_signal_set_int32(registry_, id, value);
}

proton_status_e SignalAccess::get(uint32_t id, int64_t & out) const noexcept
{
  return proton_signal_get_int64(registry_, id, &out);
}
proton_status_e SignalAccess::set(uint64_t id, int64_t value) noexcept
{
  return proton_signal_set_int64(registry_, id, value);
}

proton_status_e SignalAccess::get(uint32_t id, uint32_t & out) const noexcept
{
  return proton_signal_get_uint32(registry_, id, &out);
}
proton_status_e SignalAccess::set(uint32_t id, uint32_t value) noexcept
{
  return proton_signal_set_uint32(registry_, id, value);
}

proton_status_e SignalAccess::get(uint32_t id, uint64_t & out) const noexcept
{
  return proton_signal_get_uint64(registry_, id, &out);
}
proton_status_e SignalAccess::set(uint32_t id, uint64_t value) noexcept
{
  return proton_signal_set_uint64(registry_, id, value);
}

proton_status_e SignalAccess::get(uint32_t id, bool & out) const noexcept
{
  return proton_signal_get_bool(registry_, id, &out);
}
proton_status_e SignalAccess::set(uint32_t id, bool value) noexcept
{
  return proton_signal_set_bool(registry_, id, value);
}

proton_status_e SignalAccess::get(uint32_t id, char * buf, size_t cap, size_t & len) const noexcept
{
  return proton_signal_get_string(registry_, id, buf, cap, &len);
}
proton_status_e SignalAccess::set(uint32_t id, const char * buf, size_t len) const noexcept
{
  return proton_signal_set_string(registry_, id, buf, len);
}

proton_status_e SignalAccess::get(
  uint32_t id, uint8_t * buf, size_t cap, size_t & len) const noexcept
{
  return proton_signal_get_bytes(registry_, id, buf, cap, &len);
}
proton_status_e SignalAccess::set(uint32_t id, const uint8_t * buf, size_t len) const noexcept
{
  return proton_signal_set_bytes(registry_, id, buf, len);
}

}  // namespace proton
