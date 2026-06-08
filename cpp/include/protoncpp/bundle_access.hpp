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

#ifndef PROTON_BUNDLE_ACCESS_HPP
#define PROTON_BUNDLE_ACCESS_HPP

#include "proton/common.h"
#include "proton/registry.h"

#if PROTON_ENABLE_ALLOC
#include <functional>
#include <memory>
#endif

namespace proton
{

/**
 * @class BundleAccess provides access to bundle metadata and state from the proton_core registry API.
 */
class BundleAccess
{
public:
  explicit BundleAccess(proton_registry_t * registry, uint32_t id) noexcept
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
    auto wrapper = std::make_unique<CallbackType>(std::move(cb));
    set_callback(
      [](uint32_t bundle_id, const uint32_t * signal_ids, size_t count, void * ctx)
      {
        auto * callback = static_cast<CallbackType *>(ctx);
        if (*callback)
        {
          (*callback)(bundle_id, signal_ids, count);
        }
      },
      // Note, this is a known memory leak, but it's only expected to be used once
      // per bundle, which keeps the objects allocated until the process
      wrapper.release());
  }
#endif  // PROTON_ENABLE_ALLOC

private:
  proton_registry_t * registry_;
  uint32_t id_;
};

}  // namespace proton

#endif  // PROTON_BUNDLE_ACCESS_HPP
