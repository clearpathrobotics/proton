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
#include <asio.hpp>
#include <optional>
#include <thread>
#include <vector>

#include "protoncpp/transport/transport.hpp"

namespace proton
{

using serial_device = std::pair<std::string, uint32_t>;

class SerialTransport : public Transport
{
public:
  static constexpr uint8_t FRAME_HEADER1 = 0x50;
  static constexpr uint8_t FRAME_HEADER2 = 0x52;
  static constexpr uint8_t LENGTH_OVERHEAD = 2;
  static constexpr uint8_t CRC16_OVERHEAD = 2;
  static constexpr uint8_t HEADER_OVERHEAD = sizeof(FRAME_HEADER1) + sizeof(FRAME_HEADER2) + LENGTH_OVERHEAD;
  static constexpr uint8_t FRAME_OVERHEAD = HEADER_OVERHEAD + CRC16_OVERHEAD;

  SerialTransport(serial_device device);

  proton_status_e connect() override;
  proton_status_e disconnect() override;
  proton_status_e read(uint8_t *buf, const size_t& len, size_t& bytes_read) override;
  proton_status_e write(const uint8_t *buf, const size_t& len, size_t& bytes_written) override;

  int initDevice(serial_device s, bool server, bool blocking);
private:
  ssize_t poll(uint8_t * buf, size_t len);
  bool wait();
  std::optional<std::vector<uint8_t>> buildPacket(const uint8_t *buf, const size_t& len);

  serial_device device;
  asio::io_context io_context_;
  asio::serial_port port_;
  std::thread io_thread_;
};

}

#endif  // INC_PROTONCPP_TRANSPORT_SERIAL_HPP_
