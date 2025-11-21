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

Status SerialTransport::connect() {
  try {
    port_.open(device.first);

    // Run io_context in separate thread
    io_thread_ = std::thread([this]() { io_context_.run(); });
    return Status::OK;
  } catch (const std::exception &e) {
    std::cerr << "Failed to open serial port: " << e.what() << "\n";
    return Status::CONNECTION_ERROR;
  }
}

Status SerialTransport::disconnect() {
  if (port_.is_open()) {
    io_context_.stop();
    if (io_thread_.joinable()) {
      io_thread_.join();
    }
    port_.close();
  }

  return Status::OK;
}

std::optional<std::vector<uint8_t>> SerialTransport::buildPacket(const uint8_t *buf, const size_t& len)
{
  if (buf == nullptr)
  {
    return std::nullopt;
  }

  std::vector<uint8_t> packet(len + FRAME_OVERHEAD);
  // Fill header
  if (fillFrameHeader(packet.data(), len) != Status::OK)
  {
    return std::nullopt;
  }
  // Insert payload
  std::copy(buf, buf + len, packet.data() + HEADER_OVERHEAD);
  // Fill CRC16
  if (fillCRC16(buf, len, packet.data() + HEADER_OVERHEAD + len) != Status::OK)
  {
    return std::nullopt;
  }

  return packet;
}

Status SerialTransport::write(const uint8_t *buf, const size_t &len,
                              size_t &bytes_written) {
  if (!port_.is_open())
  {
    return Status::CONNECTION_ERROR;
  }

  auto ret = buildPacket(buf, len);

  if (!ret)
  {
    return Status::ERROR;
  }

  auto packet = ret.value();

  try {
    if (asio::write(port_, asio::buffer(packet, packet.size())) != packet.size())
    {
      return Status::WRITE_ERROR;
    }
    else
    {
      bytes_written = packet.size();
      return Status::OK;
    }
  } catch (const std::exception &e) {
    std::cerr << "Write error: " << e.what() << std::endl;
    return Status::WRITE_ERROR;
  }
}

Status SerialTransport::read(uint8_t *buf, const size_t &len,
                             size_t &bytes_read) {
  if (!port_.is_open())
  {
    return Status::CONNECTION_ERROR;
  }

  size_t payload_len = 0;
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
      return Status::READ_ERROR;
    }
  }
  catch(std::system_error &e)
  {
    return Status::CONNECTION_ERROR;
  }

  bytes_read += HEADER_OVERHEAD;

  if (getPayloadLength(header.data(), payload_len) != Status::OK) {
    return Status::INVALID_HEADER;
  }

  if (len < payload_len)
  {
    return Status::INSUFFICIENT_BUFFER;
  }

  // Read the payload
  try
  {
    if (asio::read(port_, asio::buffer(buf, payload_len)) != payload_len)
    {
      return Status::READ_ERROR;
    }
  }
  catch(std::system_error &e)
  {
    return Status::CONNECTION_ERROR;
  }

  bytes_read += payload_len;

  // Read CRC16
  try
  {
    if (asio::read(port_, asio::buffer(crc, CRC16_OVERHEAD)) != CRC16_OVERHEAD)
    {
      return Status::READ_ERROR;
    }
  }
  catch(std::system_error &e)
  {
    return Status::CONNECTION_ERROR;
  }

  bytes_read += CRC16_OVERHEAD;

  // Check that CRC16 matches
  return checkFramedPayload(buf, payload_len, static_cast<uint16_t>(crc[0] | (crc[1] << 8)));
}

uint16_t SerialTransport::getCRC16(const uint8_t *data, size_t len) {
  uint16_t crc = 0xFFFF;
  for (size_t i = 0; i < len; i++) {
    crc ^= (uint16_t)data[i] << 8;
    for (int j = 0; j < 8; j++) {
      if (crc & 0x8000)
        crc = (crc << 1) ^ 0x1021;
      else
        crc <<= 1;
    }
  }
  return crc;
}

Status SerialTransport::fillFrameHeader(uint8_t *header,
                                         const uint16_t payload_len) {
  if (header == nullptr) {
    return Status::NULL_PTR;
  }

  header[0] = FRAME_HEADER1;
  header[1] = FRAME_HEADER2;
  header[2] = (uint8_t)(payload_len & 0xFF);
  header[3] = (uint8_t)(payload_len >> 8);

  return Status::OK;
}

Status SerialTransport::fillCRC16(const uint8_t *payload,
                                const uint16_t payload_len, uint8_t *crc) {
  if (payload == NULL || crc == NULL) {
    return Status::NULL_PTR;
  }

  uint16_t crc16 = getCRC16(payload, payload_len);
  crc[0] = (uint8_t)(crc16 & 0xFF);
  crc[1] = (uint8_t)(crc16 >> 8);

  return Status::OK;
}

Status SerialTransport::checkFramedPayload(const uint8_t *payload,
                                           const size_t payload_len,
                                           const uint16_t frame_crc) {
  if (payload == nullptr)
  {
    return Status::NULL_PTR;
  }

  if (getCRC16(payload, payload_len) == frame_crc)
  {
    return Status::OK;
  }

  return Status::CRC16_ERROR;
}

Status SerialTransport::getPayloadLength(const uint8_t *header_buf,
                                         size_t &length) {
  if (header_buf[0] != FRAME_HEADER1 || header_buf[1] != FRAME_HEADER2) {
    return Status::INVALID_HEADER;
  }

  length = header_buf[2] | (header_buf[3] << 8);
  return Status::OK;
}
