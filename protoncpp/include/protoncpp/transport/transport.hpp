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

#ifndef INC_PROTONCPP_TRANSPORT_TRANSPORT_HPP_
#define INC_PROTONCPP_TRANSPORT_TRANSPORT_HPP_

#include <stdint.h>
#include <stdio.h>

namespace proton {

class Transport {
public:
  Transport() : connected_(false) {}

  virtual bool connect() { return false; }
  virtual bool disconnect() { return false; }

  virtual size_t read(uint8_t *buf, size_t len) { return 0; }
  virtual size_t write(const uint8_t *buf, size_t len) { return 0; }

protected:
  bool connected_;
};

} // namespace proton

#endif // INC_PROTONCPP_TRANSPORT_TRANSPORT_HPP_
