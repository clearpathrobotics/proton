/**
 * Software License Agreement (proprietary)
 *
 * @copyright Copyright (c) 2025 Clearpath Robotics, Inc., All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, is not permitted without the express permission of Clearpath
 * Robotics.
 *
 * @author Roni Kreinin (rkreinin@clearpathrobotics.com)
 */

#ifndef INC_PROTONCPP_STATUS_HPP_
#define INC_PROTONCPP_STATUS_HPP_

#include <stdint.h>

namespace proton {

enum Status {
  OK, // No error
  ERROR, // Generic error
  NULL_PTR,
  INVALID_STATE, // Attempting to call a function in an invalid state
  INVALID_STATE_TRANSITION, // Invalid node state transition
  CONNECTION_ERROR, // Error connecting
  SERIALIZATION_ERROR, // Error when serializing or deserializing bundle
  READ_ERROR,
  WRITE_ERROR,
  INVALID_HEADER, // Invalid header received over serial
  CRC16_ERROR, // CRC's do not match
};

} // namespace proton

#endif // INC_PROTONCPP_STATUS_HPP_
