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

  uint8_t header_buf[HEADER_OVERHEAD];

  int ret = ::read(serial_port_, header_buf, HEADER_OVERHEAD);

  if (ret < 0) {
    return 0;
  }

  size_t payload_len = getPayloadLength(header_buf);

  if (payload_len == 0)
  {
    return 0;
  }

  std::vector<uint8_t> payload_buf(payload_len + CRC16_OVERHEAD);

  ret = ::read(serial_port_, payload_buf.data(), payload_len + CRC16_OVERHEAD);

  if (ret == payload_len + CRC16_OVERHEAD)
  {
    return getPayload(payload_buf.data(), ret, buf);
  }

  return 0;
}

size_t SerialTransport::write(const uint8_t *buf, size_t len) {
  if (!connected_) {
    return 0;
  }

  std::vector<uint8_t> framed_buf(len + FRAME_OVERHEAD);

  size_t framed_len = framePayload(buf, len, framed_buf.data(), len + FRAME_OVERHEAD);

  if (framed_len != len + FRAME_OVERHEAD)
  {
    std::cout << "framed_len invalid" << std::endl;
    return 0;
  }

  int ret = ::write(serial_port_, framed_buf.data(), framed_len);

  if (ret <= 0) {
    return 0;
  }

  return ret;
}

uint16_t SerialTransport::crc16_ccitt(const uint8_t *data, size_t len) {
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

size_t SerialTransport::framePayload(const uint8_t *payload, size_t payload_len,
                                     uint8_t *out_buf, size_t out_max) {
  size_t frame_len = FRAME_OVERHEAD + payload_len;

  if (frame_len > out_max)
  {
    return 0;
  }

  out_buf[0] = FRAME_HEADER1;
  out_buf[1] = FRAME_HEADER2;
  out_buf[2] = (uint8_t)(payload_len & 0xFF);
  out_buf[3] = (uint8_t)(payload_len >> 8);

  memcpy(&out_buf[4], payload, payload_len);

  uint16_t crc = crc16_ccitt(payload, payload_len);
  out_buf[4 + payload_len] = (uint8_t)(crc & 0xFF);
  out_buf[4 + payload_len + 1] = (uint8_t)(crc >> 8);

  return frame_len;
}

size_t SerialTransport::getPayload(const uint8_t * in_buf, size_t in_len, uint8_t * payload)
{
  // Missing CRC16
  if (in_len < CRC16_OVERHEAD)
  {
    return 0;
  }

  size_t payload_len = in_len - CRC16_OVERHEAD;
  uint16_t in_crc = (in_buf[payload_len] | in_buf[payload_len + 1] << 8);
  uint16_t crc = crc16_ccitt(in_buf, payload_len);

  if (crc != in_crc)
  {
    std::cerr << "Invalid CRC " << crc  << " != " << in_crc << std::endl;
    return 0;
  }

  memcpy(payload, in_buf, payload_len);

  return payload_len;
}

size_t SerialTransport::getPayloadLength(const uint8_t * header_buf)
{
  if (header_buf[0] != FRAME_HEADER1 || header_buf[1] != FRAME_HEADER2)
  {
    //std::cerr << "Invalid headers [" << std::hex << header_buf[0] << ", " << header_buf[1] << "]" << std::endl;
    return 0;
  }

  return (header_buf[2] | header_buf[3] << 8);
}

