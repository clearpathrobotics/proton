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

#include "proton/common.h"

const char * proton_status_to_string(proton_status_e status)
{
  switch (status)
  {
    case (PROTON_OK):
      return "Success";
    case (PROTON_ERROR):
      return "Generic error";
    case (PROTON_NULL_PTR_ERROR):
      return "Null pointer error";
    case (PROTON_SERIALIZATION_ERROR):
      return "Error serializing or deserializing protobuf";
    case (PROTON_INVALID_HEADER_ERROR):
      return "Invalid header received over serial";
    case (PROTON_CRC16_ERROR):
      return "CRC16 mismatch";
    case (PROTON_MUTEX_ERROR):
      return "Failed to lock or unlock mutex";
    case (PROTON_INSUFFICIENT_BUFFER_ERROR):
      return "Buffer is too small to fit required data";
    case (PROTON_INCORRECT_TARGET_ERROR):
      return "Message has been sent to the wrong target";
    case (PROTON_UNSUPPORTED_OPERATION_ERROR):
      return "Message is not a supported operation";
    default:
      return "Unknown value";
  }

  return "Unknown value";
}
