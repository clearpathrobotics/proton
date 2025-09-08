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

#ifndef INC_PROTONCPP_TRANSPORT_SERIAL_HPP_
#define INC_PROTONCPP_TRANSPORT_SERIAL_HPP_

#include <string>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <iostream>

#include "protoncpp/transport/transport.hpp"

namespace proton
{

using serial_device = std::pair<std::string, uint32_t>;

class SerialTransport : public Transport
{
public:
  static constexpr uint8_t FRAME_HEADER1 = 0xAA;
  static constexpr uint8_t FRAME_HEADER2 = 0x55;
  static constexpr uint8_t LENGTH_OVERHEAD = 2;
  static constexpr uint8_t CRC16_OVERHEAD = 2;
  static constexpr uint8_t HEADER_OVERHEAD = sizeof(FRAME_HEADER1) + sizeof(FRAME_HEADER2) + LENGTH_OVERHEAD;
  static constexpr uint8_t FRAME_OVERHEAD = HEADER_OVERHEAD + CRC16_OVERHEAD;

  SerialTransport(serial_device device);

  bool connect() override;
  bool disconnect() override;
  size_t read(uint8_t * buf, size_t len) override;
  size_t write(const uint8_t * buf, size_t len) override;

  int initDevice(serial_device s, bool server, bool blocking);

  static uint16_t crc16_ccitt(const uint8_t *data, size_t len);
  static size_t framePayload(const uint8_t * payload, size_t payload_len, uint8_t * out_buf, size_t out_max);
  static size_t getPayloadLength(const uint8_t * header_buf);
  static size_t getPayload(const uint8_t * in_buf, size_t in_len, uint8_t * payload);

private:
  serial_device device_;
  int serial_port_;
};

}

#endif  // INC_PROTONCPP_TRANSPORT_SERIAL_HPP_
