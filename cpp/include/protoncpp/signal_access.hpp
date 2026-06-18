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

#include <cstddef>
#include <cstdint>
#include <type_traits>

#if PROTON_ENABLE_ALLOC

#include <string>
#include <vector>

#endif  // PROTON_ENABLE_ALLOC

#if __cplusplus >= 202002L

#include <span>

#endif  // __cplusplus >= 202002L

namespace proton
{

/**
 * @class SignalAccess provides type-safe access to signal values from the proton_core registry API.
 */
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
  proton_status_e set(uint32_t id, int64_t value) noexcept;

  proton_status_e get(uint32_t id, uint32_t & out) const noexcept;
  proton_status_e set(uint32_t id, uint32_t value) noexcept;

  proton_status_e get(uint32_t id, uint64_t & out) const noexcept;
  proton_status_e set(uint32_t id, uint64_t value) noexcept;

  proton_status_e get(uint32_t id, bool & out) const noexcept;
  proton_status_e set(uint32_t id, bool value) noexcept;

  proton_status_e get(uint32_t id, char * buf, size_t cap, size_t & len) const noexcept;
  proton_status_e set(uint32_t id, const char * buf, size_t len) const noexcept;

  proton_status_e get(uint32_t id, uint8_t * buf, size_t cap, size_t & len) const noexcept;
  proton_status_e set(uint32_t id, const uint8_t * buf, size_t len) const noexcept;

private:
  proton_registry_t * registry_;
};

/**
 * Forward-declaration of Signal class for RTTI-enabled purposes
 */
template <typename T>
class Signal;

/**
 * @brief Non-templated base class for Signal<T>, enabling storage in containers.
 *
 * Use type() to determine the actual signal type at runtime.
 * When PROTON_ENABLE_ALLOC is enabled, use as<T>() for safe dynamic_cast.
 * When disabled (for embedded applications), cast to Signal<T>* only when type() matches.
 */
class SignalBase
{
public:
  constexpr SignalBase(proton_registry_t * registry, uint32_t id) noexcept
  : registry_(registry), id_(id)
  {
  }

#if PROTON_ENABLE_ALLOC
  virtual ~SignalBase() = default;

  /**
   * @brief Safely cast to Signal<T>* using dynamic_cast (requires RTTI).
   * @return Pointer to Signal<T> if type matches, nullptr otherwise.
   */
  template <typename T>
  Signal<T> * as() noexcept
  {
    return dynamic_cast<Signal<T> *>(this);
  }

  template <typename T>
  const Signal<T> * as() const noexcept
  {
    return dynamic_cast<const Signal<T> *>(this);
  }
#else
  // No virtual destructor in embedded mode (no RTTI)
  ~SignalBase() = default;
#endif  // PROTON_ENABLE_ALLOC

  uint32_t id() const noexcept { return id_; }

  signal_desc_t * desc() const noexcept
  {
    return proton_registry_get_signal(registry_, id_, nullptr);
  }

  proton_signal_type_e type() const noexcept
  {
    signal_desc_t * d = desc();
    return d ? d->type : PROTON_INVALID_TYPE;
  }

protected:
  proton_registry_t * registry_;
  uint32_t id_;
};

namespace detail
{
// Helper trait to detect primitive signal types (types with direct SignalAccess support)
template <typename T>
struct is_primitive_signal_type
: std::bool_constant<
    std::is_same_v<T, double> || std::is_same_v<T, float> || std::is_same_v<T, int32_t> ||
    std::is_same_v<T, int64_t> || std::is_same_v<T, uint32_t> || std::is_same_v<T, uint64_t> ||
    std::is_same_v<T, bool>>
{
};

template <typename T>
inline constexpr bool is_primitive_signal_type_v = is_primitive_signal_type<T>::value;
}  // namespace detail

/**
 * @class Signal typed wrapper around a SignalBase. Does not require RTTI, but if
 * RTTI is enabled, it can be safely stored as a SignalBase pointer and dynamically cast back to Signal<T>.
 */
template <typename T>
class Signal : public SignalBase
{
public:
  constexpr explicit Signal(proton_registry_t * registry, uint32_t id) : SignalBase(registry, id) {}

  template <typename U = T, std::enable_if_t<detail::is_primitive_signal_type_v<U>, int> = 0>
  proton_status_e get(U & out) const noexcept
  {
    return SignalAccess(registry_).get(id_, out);
  }

  template <typename U = T, std::enable_if_t<detail::is_primitive_signal_type_v<U>, int> = 0>
  proton_status_e set(U value) noexcept
  {
    return SignalAccess(registry_).set(id_, value);
  }

  proton_status_e get(char * buf, size_t cap, size_t & len) const noexcept
  {
    static_assert(
      std::is_same_v<T, char *>, "get(char*, size_t, size_t&) is only valid for Signal<char*>");
    return SignalAccess(registry_).get(id_, buf, cap, len);
  }

  proton_status_e set(const char * buf, size_t len) noexcept
  {
    static_assert(
      std::is_same_v<T, char *>, "set(const char*, size_t) is only valid for Signal<char*>");
    return SignalAccess(registry_).set(id_, buf, len);
  }

  proton_status_e get(uint8_t * buf, size_t cap, size_t & len) const noexcept
  {
    static_assert(
      std::is_same_v<T, uint8_t *>,
      "get(uint8_t*, size_t, size_t&) is only valid for Signal<uint8_t*>");
    return SignalAccess(registry_).get(id_, buf, cap, len);
  }

  proton_status_e set(const uint8_t * buf, size_t len) noexcept
  {
    static_assert(
      std::is_same_v<T, uint8_t *>,
      "set(const uint8_t*, size_t) is only valid for Signal<uint8_t*>");
    return SignalAccess(registry_).set(id_, buf, len);
  }

#if PROTON_ENABLE_ALLOC

  // Some convenience methods for STL types when allocations are allowed

  proton_status_e get(std::string & str) const noexcept
  {
    static_assert(
      std::is_same_v<T, std::string>,
      "get(std::string&, size_t&) is only valid for Signal<std::string>");

    size_t len;
    const proton_status_e status =
      SignalAccess(registry_).get(id_, str.data(), str.capacity(), len);

    if (status == PROTON_OK && len > 0)
    {
      str.resize(len - 1);
    }

    return status;
  }

  proton_status_e set(const std::string & str) noexcept
  {
    static_assert(
      std::is_same_v<T, std::string>, "set(std::string&) is only valid for Signal<std::string>");

    return SignalAccess(registry_).set(id_, str.c_str(), str.size() + 1);
  }

  proton_status_e get(std::vector<uint8_t> & buf) const noexcept
  {
    static_assert(
      std::is_same_v<T, std::vector<uint8_t>>,
      "get(std::vector<uint8_t>&) is only valid for Signal<std::vector<uint8_t>>");

    size_t len;
    const proton_status_e status =
      SignalAccess(registry_).get(id_, buf.data(), buf.capacity(), len);

    if (status == PROTON_OK)
    {
      buf.resize(len);
    }

    return status;
  }

  proton_status_e set(const std::vector<uint8_t> & buf) noexcept
  {
    static_assert(
      std::is_same_v<T, std::vector<uint8_t>>,
      "set(std::vector<uint8_t>&) is only valid for Signal<std::vector<uint8_t>>");

    return SignalAccess(registry_).set(id_, buf.data(), buf.size());
  }

#endif  // PROTON_ENABLE_ALLOC

#if __cplusplus >= 202002L

  proton_status_e get(std::span<uint8_t> buf, size_t & len) const noexcept
  {
    return get(buf.data(), buf.size(), len);
  }

  proton_status_e set(std::span<const uint8_t> buf) const noexcept
  {
    return set(buf.data(), buf.size());
  }

#endif  // __cplusplus >= 202002L
};

}  // namespace proton

#endif  // PROTON_SIGNAL_ACCESS_HPP
