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

#ifndef PROTON_BUNDLE_HPP
#define PROTON_BUNDLE_HPP

#include "proton/common.h"
#include "proton/registry.h"

#if PROTON_ENABLE_ALLOC
#include <functional>
#endif

namespace proton
{

/**
 * @class BundleAccess provides access to bundle metadata and state from the proton_core registry API.
 */
class BundleAccess
{
public:
  constexpr explicit BundleAccess(proton_registry_t * registry, uint32_t id) noexcept
  : registry_(registry), id_(id)
  {
  }

  uint32_t id() const noexcept { return id_; }

  const bundle_desc_t * descriptor() const noexcept;

  void set_period(uint32_t period_ms) noexcept;
  void set_callback(proton_bundle_cb_f cb, void * ctx) noexcept;

#if PROTON_ENABLE_ALLOC
  using CallbackType =
    std::function<void(uint32_t bundle_id, const uint32_t * signal_ids, size_t count)>;

  void set_callback(CallbackType cb) noexcept
  {
    // Store the std::function in a heap-allocated wrapper and pass it as the context to the C callback
    callback_wrapper_ = std::move(cb);
    set_callback(
      [](uint32_t bundle_id, const uint32_t * signal_ids, size_t count, void * ctx)
      {
        auto * self = static_cast<BundleAccess *>(ctx);
        if (self->callback_wrapper_)
        {
          self->callback_wrapper_(bundle_id, signal_ids, count);
        }
      },
      this);
  }
#endif  // PROTON_ENABLE_ALLOC

private:
  proton_registry_t * registry_;
  uint32_t id_;

#if PROTON_ENABLE_ALLOC
  CallbackType callback_wrapper_ = nullptr;
#endif  // PROTON_ENABLE_ALLOC
};

}  // namespace proton

#endif  // PROTON_BUNDLE_HPP
