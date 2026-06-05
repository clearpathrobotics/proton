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

#include "protoncpp/bundle_access.hpp"

namespace proton
{

const bundle_desc_t * BundleAccess::descriptor() const noexcept
{
  return proton_registry_get_bundle(registry_, id_, nullptr);
}

void BundleAccess::set_period(uint32_t period_ms) noexcept
{
  proton_registry_set_bundle_period(registry_, id_, period_ms);
}

void BundleAccess::set_callback(proton_bundle_cb_f cb, void * ctx) noexcept
{
  proton_registry_set_bundle_callback(registry_, id_, cb, ctx);
}

}  // namespace proton
