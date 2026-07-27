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
#include <string>
#include <unordered_map>
#include "proton/common.h"

TEST(ProtonCommon, StatusToString)
{
  std::unordered_map<int, std::string> statuses = {
    {PROTON_OK, "Success"},
    {PROTON_ERROR, "Generic error"},
    {PROTON_NULL_PTR_ERROR, "Null pointer error"},
    {PROTON_SERIALIZATION_ERROR, "Error serializing or deserializing protobuf"},
    {PROTON_INVALID_HEADER_ERROR, "Invalid header received over serial"},
    {PROTON_CRC16_ERROR, "CRC16 mismatch"},
    {PROTON_MUTEX_ERROR, "Failed to lock or unlock mutex"},
    {PROTON_INSUFFICIENT_BUFFER_ERROR, "Buffer is too small to fit required data"},
    {PROTON_INCORRECT_TARGET_ERROR, "Message has been sent to the wrong target"},
    {PROTON_UNSUPPORTED_OPERATION_ERROR, "Message is not a supported operation"},
    {-1, "Unknown value"},
  };

  for (const auto & [status, str] : statuses)
  {
    EXPECT_EQ(proton_status_to_string(static_cast<proton_status_e>(status)), str);
  }
}

int main(int argc, char ** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
