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
#include <string.h>
#include <vector>

using namespace proton;

SerialTransport::SerialTransport(serial_device device) : device_(device) {}

bool SerialTransport::connect() {
  if (connected_) {
    return true;
  }

  serial_port_ = open(device_.first.c_str(), O_RDWR | O_NOCTTY | O_SYNC);

  if (serial_port_ == -1) {
    std::cerr << "Error " << errno << " opening serial device " << device_.first
              << ": " << strerror(errno) << std::endl;
    return false;
  }

  std::cout << "opened fd " << serial_port_ << std::endl;

  struct termios tty;

  if (tcgetattr(serial_port_, &tty) != 0) {
    std::cerr << "Error " << errno << "from tcgetattr: " << strerror(errno)
              << std::endl;
    return false;
  }

  cfsetospeed(&tty, B921600);
  cfsetispeed(&tty, B921600);
  tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8; // 8-bit
  tty.c_cflag |= (CLOCAL | CREAD);            // enable receiver
  tcsetattr(serial_port_, TCSANOW, &tty);

  connected_ = true;

  return connected_;
}

bool SerialTransport::disconnect() {
  ::close(serial_port_);
  return true;
}

size_t SerialTransport::read(uint8_t *buf, size_t len) {
  if (!connected_) {
    return 0;
  }

  // Read header
  int ret = ::read(serial_port_, buf, HEADER_OVERHEAD);

  if (ret < 0) {
    return 0;
  }

  size_t payload_len = getPayloadLength(buf);

  if (payload_len == 0)
  {
    return 0;
  }

  // Read payload
  ret = ::read(serial_port_, buf, payload_len);

  if (ret < 0)
  {
    return 0;
  }

  uint8_t crc[2];

  // Read CRC
  ret = ::read(serial_port_, crc, CRC16_OVERHEAD);

  if (ret != CRC16_OVERHEAD || !checkFramedPayload(buf, payload_len, static_cast<uint16_t>(crc[0] | (crc[1] << 8))))
  {
    return 0;
  }

  return payload_len;
}

size_t SerialTransport::write(const uint8_t *buf, size_t len) {
  if (!connected_) {
    return 0;
  }

  uint8_t header[4];
  uint8_t crc[2];

  if (!fillFrameHeader(header, len) || !fillCRC16(buf, len, crc))
  {
    return 0;
  }

  // Write header
  int ret = ::write(serial_port_, header, HEADER_OVERHEAD);

  if (ret != HEADER_OVERHEAD) {
    return 0;
  }

  // Write payload
  ret = ::write(serial_port_, buf, len);

  if (ret != len) {
    return 0;
  }

  // Write CRC16
  ret = ::write(serial_port_, crc, CRC16_OVERHEAD);

  if (ret != CRC16_OVERHEAD) {
    return 0;
  }

  return len;
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

bool SerialTransport::fillFrameHeader(uint8_t * header, const uint16_t payload_len) {
  if (header == nullptr)
  {
    return false;
  }

  header[0] = FRAME_HEADER1;
  header[1] = FRAME_HEADER2;
  header[2] = (uint8_t)(payload_len & 0xFF);
  header[3] = (uint8_t)(payload_len >> 8);

  return true;
}

bool SerialTransport::fillCRC16(const uint8_t * payload, const uint16_t payload_len, uint8_t * crc)
{
  if (payload == NULL || crc == NULL)
  {
    return false;
  }

  uint16_t crc16 = getCRC16(payload, payload_len);
  crc[0] = (uint8_t)(crc16 & 0xFF);
  crc[1] = (uint8_t)(crc16 >> 8);

  return true;
}

bool SerialTransport::checkFramedPayload(const uint8_t *payload, const size_t payload_len, const uint16_t frame_crc) {
  return getCRC16(payload, payload_len) == frame_crc;
}

size_t SerialTransport::getPayloadLength(const uint8_t * header_buf)
{
  if (header_buf[0] != FRAME_HEADER1 || header_buf[1] != FRAME_HEADER2)
  {
    return 0;
  }

  return (header_buf[2] | header_buf[3] << 8);
}

