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
 * @author Roni Kreinin (roni.kreinin@rockwellautomation.com)
 */

#ifndef PROTONCPP_TESTS_PROTONCPP_UTILS_HPP
#define PROTONCPP_TESTS_PROTONCPP_UTILS_HPP

#include "protoncpp/proton.hpp"
#include <gtest/gtest.h>
#include <google/protobuf/util/message_differencer.h>

std::map<std::string, proton::BundleHandle> getBundles(std::string config_file);

template<typename T>
void AssertSignalValue(proton::SignalHandle& handle, const T& value)
{
  ASSERT_EQ(handle.getValue<T>(), value);
}

template<>
inline void AssertSignalValue<std::string>(proton::SignalHandle& handle, const std::string& value)
{
  ASSERT_STREQ(handle.getValue<std::string>().c_str(), value.c_str());
}

#endif  // PROTONCPP_TESTS_PROTONCPP_UTILS_HPP