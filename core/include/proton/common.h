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

#ifndef INC_PROTON_COMMON_H_
#define INC_PROTON_COMMON_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

// Max message size
#define PROTON_MAX_MESSAGE_SIZE UINT16_MAX

  /**
   * @brief Proton return status enum
   *
   */
  typedef enum
  {
    PROTON_OK,                           // Success
    PROTON_ERROR,                        // Generic error
    PROTON_NULL_PTR_ERROR,               // Null pointer error
    PROTON_SERIALIZATION_ERROR,          // Error serializing or deserializing protobuf
    PROTON_INVALID_HEADER_ERROR,         // Invalid header received over serial
    PROTON_CRC16_ERROR,                  // CRC16 mismatch
    PROTON_MUTEX_ERROR,                  // Failed to lock or unlock mutex
    PROTON_INSUFFICIENT_BUFFER_ERROR,    // Buffer is too small to fit required data
    PROTON_INCORRECT_TARGET_ERROR,       // Message has been sent to the wrong target
    PROTON_UNSUPPORTED_OPERATION_ERROR,  // Message is not a supported operation
  } proton_status_e;

  /**
   * @brief Proton node state enum
   *
   */
  typedef enum
  {
    PROTON_NODE_UNCONFIGURED,  // Node is unconfigured
    PROTON_NODE_INACTIVE,      // Node has been configured but has not started sending or receiving
                               // bundles
    PROTON_NODE_ACTIVE         // Node is configured and actively sending and receiving bundles
  } proton_node_state_e;

  /**
   * @brief Proton transport state enum
   *
   */
  typedef enum
  {
    PROTON_TRANSPORT_DISCONNECTED,  // Transport is disconnected
    PROTON_TRANSPORT_CONNECTED,     // Transport is connected
    PROTON_TRANSPORT_ERROR          // Transport is in an error state
  } proton_transport_state_e;

#ifdef __cplusplus
}
#endif

#endif  // INC_PROTON_COMMON_H_
