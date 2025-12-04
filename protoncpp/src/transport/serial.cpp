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

#include "protoncpp/transport/serial.hpp"
#include <array>
#include <poll.h>
#include <string.h>
#include <vector>

#include <asio.hpp>

using namespace proton;

SerialTransport::SerialTransport(serial_device device)
    : device(device), port_(io_context_) {}

proton_status_e SerialTransport::connect() {
  try {
    port_.open(device.first);

    // Run io_context in separate thread
    io_thread_ = std::thread([this]() { io_context_.run(); });
    return PROTON_OK;
  } catch (const std::exception &e) {
    std::cerr << "Failed to open serial port: " << e.what() << "\n";
    return PROTON_CONNECT_ERROR;
  }
}

proton_status_e SerialTransport::disconnect() {
  if (port_.is_open()) {
    io_context_.stop();
    if (io_thread_.joinable()) {
      io_thread_.join();
    }
    port_.close();
  }

  return PROTON_OK;
}

std::optional<std::vector<uint8_t>> SerialTransport::buildPacket(const uint8_t *buf, const size_t& len)
{
  if (buf == nullptr)
  {
    return std::nullopt;
  }

  std::vector<uint8_t> packet(len + FRAME_OVERHEAD);
  // Fill header
  if (proton_fill_frame_header(packet.data(), len) != PROTON_OK)
  {
    return std::nullopt;
  }
  // Insert payload
  std::copy(buf, buf + len, packet.data() + HEADER_OVERHEAD);
  // Fill CRC16
  if (proton_fill_crc16(buf, len, packet.data() + HEADER_OVERHEAD + len) != PROTON_OK)
  {
    return std::nullopt;
  }

  return packet;
}

proton_status_e SerialTransport::write(const uint8_t *buf, const size_t &len,
                              size_t &bytes_written) {
  if (!port_.is_open())
  {
    return PROTON_CONNECT_ERROR;
  }

  auto ret = buildPacket(buf, len);

  if (!ret)
  {
    return PROTON_ERROR;
  }

  auto packet = ret.value();

  try {
    if (asio::write(port_, asio::buffer(packet, packet.size())) != packet.size())
    {
      return PROTON_WRITE_ERROR;
    }
    else
    {
      bytes_written = packet.size();
      return PROTON_OK;
    }
  } catch (const std::exception &e) {
    std::cerr << "Write error: " << e.what() << std::endl;
    return PROTON_WRITE_ERROR;
  }
}

proton_status_e SerialTransport::read(uint8_t *buf, const size_t &len,
                             size_t &bytes_read) {
  if (!port_.is_open())
  {
    return PROTON_CONNECT_ERROR;
  }

  uint16_t payload_len = 0;
  std::array<uint8_t, HEADER_OVERHEAD> header;
  std::array<uint8_t, CRC16_OVERHEAD> crc;

  // Read header of next packet
  try {
    // Synchronize to start of frame
    do
    {
      asio::read(port_, asio::buffer(header, 1));
    } while (header[0] != FRAME_HEADER1);

    // Read the rest of the header
    if (asio::read(port_, asio::buffer(&header[1], HEADER_OVERHEAD - 1)) != HEADER_OVERHEAD - 1)
    {
      return PROTON_READ_ERROR;
    }
  }
  catch(std::system_error &e)
  {
    return PROTON_READ_ERROR;
  }

  bytes_read += HEADER_OVERHEAD;

  if (proton_get_framed_payload_length(header.data(), &payload_len) != PROTON_OK) {
    return PROTON_INVALID_HEADER_ERROR;
  }

  if (len < payload_len)
  {
    return PROTON_INSUFFICIENT_BUFFER_ERROR;
  }

  // Read the payload
  try
  {
    if (asio::read(port_, asio::buffer(buf, payload_len)) != payload_len)
    {
      return PROTON_READ_ERROR;
    }
  }
  catch(std::system_error &e)
  {
    return PROTON_READ_ERROR;
  }

  bytes_read += payload_len;

  // Read CRC16
  try
  {
    if (asio::read(port_, asio::buffer(crc, CRC16_OVERHEAD)) != CRC16_OVERHEAD)
    {
      return PROTON_READ_ERROR;
    }
  }
  catch(std::system_error &e)
  {
    return PROTON_READ_ERROR;
  }

  bytes_read += CRC16_OVERHEAD;

  // Check that CRC16 matches
  return proton_check_framed_payload(buf, payload_len, static_cast<uint16_t>(crc[0] | (crc[1] << 8)));
}
