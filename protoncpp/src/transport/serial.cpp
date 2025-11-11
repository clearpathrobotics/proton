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

SerialTransport::SerialTransport(serial_device node, serial_device peer)
    : node_(node), peer_(peer), node_port_(io_context_), peer_port_(io_context_) {}

Status SerialTransport::connect() {
  try {
    node_port_.open(node_.first);
    peer_port_.open(node_.first);
    // port_.set_option(asio::serial_port_base::baud_rate(115200));
    // port_.set_option(asio::serial_port_base::character_size(8));
    // port_.set_option(
    //     asio::serial_port_base::parity(asio::serial_port_base::parity::none));
    // port_.set_option(asio::serial_port_base::stop_bits(
    //     asio::serial_port_base::stop_bits::one));
    // port_.set_option(asio::serial_port_base::flow_control(
    //     asio::serial_port_base::flow_control::none));

    // Run io_context in separate thread
    io_thread_ = std::thread([this]() { io_context_.run(); });
    return Status::OK;
  } catch (const std::exception &e) {
    std::cerr << "Failed to open serial port: " << e.what() << "\n";
    return Status::CONNECTION_ERROR;
  }
}

Status SerialTransport::disconnect() {
  if (node_port_.is_open() || peer_port_.is_open()) {
    io_context_.stop();
    if (io_thread_.joinable()) {
      io_thread_.join();
    }
    node_port_.close();
    peer_port_.close();
  }

  return Status::OK;
}

std::vector<uint8_t> SerialTransport::buildPacket(const uint8_t *buf, const size_t& len)
{
  std::vector<uint8_t> packet(len + FRAME_OVERHEAD);
  // Fill header
  fillFrameHeader(packet.data(), len);
  // Insert payload
  std::copy(buf, buf + len, packet.data() + HEADER_OVERHEAD);
  // Fill CRC16
  fillCRC16(buf, len, packet.data() + HEADER_OVERHEAD + len);

  return packet;
}

Status SerialTransport::write(const uint8_t *buf, const size_t &len,
                              size_t &bytes_written) {
  if (!peer_port_.is_open())
  {
    return Status::CONNECTION_ERROR;
  }

  auto packet = buildPacket(buf, len);

  try {
    if (asio::write(peer_port_, asio::buffer(packet, packet.size())) != packet.size())
    {
      return Status::WRITE_ERROR;
    }
    else
    {
      bytes_written = packet.size();
      //std::cout << "Wrote " << bytes_written << " Bytes. Len: " << len << std::endl;
      return Status::OK;
    }
  } catch (const std::exception &e) {
    std::cerr << "Write error: " << e.what() << std::endl;
    return Status::WRITE_ERROR;
  }
}

Status SerialTransport::read(uint8_t *buf, const size_t &len,
                             size_t &bytes_read) {
  if (!peer_port_.is_open())
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
      asio::read(node_port_, asio::buffer(header, 1));
    } while (header[0] != FRAME_HEADER1);

    // Read the rest of the header
    if (asio::read(node_port_, asio::buffer(&header[1], HEADER_OVERHEAD - 1)) != HEADER_OVERHEAD - 1)
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
    if (asio::read(node_port_, asio::buffer(buf, payload_len)) != payload_len)
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
    if (asio::read(node_port_, asio::buffer(crc, CRC16_OVERHEAD)) != CRC16_OVERHEAD)
    {
      return Status::READ_ERROR;
    }
  }
  catch(std::system_error &e)
  {
    return Status::CONNECTION_ERROR;
  }

  bytes_read += CRC16_OVERHEAD;

  uint16_t received_crc = static_cast<uint16_t>(crc[0] | (crc[1] << 8));

  if (!checkFramedPayload(buf, payload_len, received_crc))
  {
    return Status::CRC16_ERROR;
  }

  return Status::OK;
}

// Status SerialTransport::connect() {
//   if (state_ != TransportState::DISCONNECTED)
//   {
//     return Status::INVALID_STATE_TRANSITION;
//   }

//   serial_port_ = open(device_.first.c_str(), O_RDWR | O_NOCTTY |
//   O_NONBLOCK);

//   if (serial_port_ == -1) {
//     std::cerr << "Error " << errno << " opening serial device " <<
//     device_.first
//               << ": " << strerror(errno) << std::endl;
//     return Status::CONNECTION_ERROR;
//   }

//   struct termios tty;

//   if (tcgetattr(serial_port_, &tty) != 0) {
//     std::cerr << "Error " << errno << "from tcgetattr: " <<
//     strerror(errno)
//               << std::endl;
//     return Status::CONNECTION_ERROR;
//   }

//   cfsetospeed(&tty, B1152000);
//   cfsetispeed(&tty, B1152000);

//     // Control modes
//   tty.c_cflag &= ~PARENB;    // No parity
//   tty.c_cflag &= ~CSTOPB;    // 1 stop bit
//   tty.c_cflag &= ~CSIZE;
//   tty.c_cflag |= CS8;        // 8 data bits
//   tty.c_cflag &= ~CRTSCTS;   // No hardware flow control
//   tty.c_cflag |= CREAD | CLOCAL; // Enable receiver, ignore modem lines

//   // Local modes - RAW mode
//   tty.c_lflag &= ~ICANON;    // Non-canonical mode
//   tty.c_lflag &= ~ECHO;
//   tty.c_lflag &= ~ECHOE;
//   tty.c_lflag &= ~ECHONL;
//   tty.c_lflag &= ~ISIG;

//   // Input modes
//   tty.c_iflag &= ~(IXON | IXOFF | IXANY); // No software flow control
//   tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL);

//   // Output modes - no processing
//   tty.c_oflag &= ~OPOST;
//   tty.c_oflag &= ~ONLCR;
//   tty.c_cc[VTIME] = 0;
//   tty.c_cc[VMIN] = 0;

//   tcsetattr(serial_port_, TCSANOW, &tty);

//   tcflush(serial_port_, TCIOFLUSH);

//   return Status::OK;
// }

// Status SerialTransport::disconnect() {
//   if (::close(serial_port_) != 0)
//   {
//     if (errno != EBADF)
//     {
//       std::cerr << "Error " << errno << " closing serial device " <<
//       device_.first
//               << ": " << strerror(errno) << std::endl;
//       return Status::CONNECTION_ERROR;
//     }
//   }

//   return Status::OK;
// }

// ssize_t SerialTransport::poll(uint8_t *buf, size_t len) {
//   struct pollfd fds[1];
//   fds[0].fd = serial_port_;
//   fds[0].events = POLLIN;

//   // Read header
//   ssize_t bytes_read = 0;
//   int ret = 0;

//   // Poll until len bytes have been read
//   while(bytes_read < len)
//   {
//     // Poll until data is available
//     int ret = ::poll(fds, 1, -1);

//     if (ret > 0)
//     {
//       if (fds[0].revents & POLLIN)
//       {
//         ret = ::read(serial_port_, buf, len - bytes_read);

//         if (ret < 0) {
//           return ret;
//         }
//         else
//         {
//           bytes_read += ret;
//         }
//       }

//       if(fds[0].revents & (POLLERR | POLLHUP | POLLNVAL))
//       {
//         // Error or device disconnected
//         std::cerr << "Device error or disconnected" << std::endl;
//         return -1;
//       }
//     }
//     else
//     {
//       std::cerr << "Poll timeout" << std::endl;
//       return ret;
//     }
//   }

//   return bytes_read;
// }

// bool SerialTransport::wait() {
//   fd_set write_fds;
//   FD_ZERO(&write_fds);
//   FD_SET(serial_port_, &write_fds);
//   int timeout_ms = 1000;

//   struct timeval timeout;
//   timeout.tv_sec = timeout_ms / 1000;
//   timeout.tv_usec = (timeout_ms % 1000) * 1000;

//   // Poll event occurs
//   if (::select(serial_port_ + 1, nullptr, &write_fds, nullptr, &timeout)
//   > 0)
//   {
//     return true;
//   }

//   return false;
// }

// void SerialTransport::flush()
// {
//   if (serial_port_ >= 0) {
//     tcflush(serial_port_, TCIFLUSH);
//   }
// }

// Status SerialTransport::read(uint8_t *buf, const size_t& len, size_t&
// bytes_read) {
//   if (buf == nullptr)
//   {
//     return Status::NULL_PTR;
//   }

//   if (len < HEADER_OVERHEAD)
//   {
//     return INSUFFICIENT_BUFFER;
//   }

//   if (!connected()) {
//     return Status::INVALID_STATE;
//   }

//   ssize_t count = 0;

//   std::array<uint8_t, 4> header;

//   // Synchronize to first frame header byte
//   while (header[0] != FRAME_HEADER1)
//   {
//     count = this->poll(header.data(), 1);

//     if (count == -1)
//     {
//       return Status::READ_ERROR;
//     }
//   }

//   // Receive the rest of the header
//   count = this->poll(&header[1], HEADER_OVERHEAD - 1);

//   if (count == -1)
//   {
//     return Status::READ_ERROR;
//   }

//   size_t payload_len = 0;
//   Status status = getPayloadLength(header.data(), payload_len);

//   if (status != Status::OK)
//   {
//     return status;
//   }

//   if (payload_len + CRC16_OVERHEAD > len)
//   {
//     return Status::INSUFFICIENT_BUFFER;
//   }

//   // Read payload
//   count = this->poll(buf, payload_len);

//   if (count != payload_len)
//   {
//     return Status::READ_ERROR;
//   }

//   // Read CRC
//   uint8_t crc[CRC16_OVERHEAD];

//   count = this->poll(crc, CRC16_OVERHEAD);

//   if (count != CRC16_OVERHEAD)
//   {
//     return Status::READ_ERROR;
//   }

//   //std::cout << "Calculated CRC: " << getCRC16(buf, payload_len) << "
//   Received: " << static_cast<uint16_t>(crc[0] | (crc[1] << 8)) <<
//   std::endl;

//   if (!checkFramedPayload(buf, payload_len, static_cast<uint16_t>(crc[0]
//   | (crc[1] << 8))))
//   {
//     return Status::CRC16_ERROR;
//   }

//  // std::cout << "Read OK" << std::endl;

//   bytes_read = payload_len;
//   return Status::OK;
// }

// Status SerialTransport::write(const uint8_t *buf, const size_t& len,
// size_t& bytes_written) {
//   if (!connected()) {
//     return Status::INVALID_STATE;
//   }

//   std::array<uint8_t, HEADER_OVERHEAD> header;
//   std::array<uint8_t, CRC16_OVERHEAD> crc;

//   if (!fillFrameHeader(header.data(), len))
//   {
//     return Status::INVALID_HEADER;
//   }

//   if (!fillCRC16(buf, len, crc.data()))
//   {
//     return Status::CRC16_ERROR;
//   }

//   // Write header
//   if (wait())
//   {
//     int ret = ::write(serial_port_, header.data(), header.size());

//     if (ret != HEADER_OVERHEAD) {
//       if (errno != EAGAIN)
//       {
//         std::cerr << "Write Error " << errno << ": " << strerror(errno)
//         << std::endl; return Status::WRITE_ERROR;
//       }

//       std::cerr << strerror(errno) << std::endl;

//       return Status::RESOURCE_TEMPORARILY_UNAVAILABLE;
//     }

//     // bytes_written = ret;
//     // return Status::OK;
//   }
//   else
//   {
//     return Status::RESOURCE_TEMPORARILY_UNAVAILABLE;
//   }

//   // Write payload
//   if (wait())
//   {
//     int ret = ::write(serial_port_, buf, len);

//     if (ret != len) {
//       if (errno != EAGAIN)
//       {
//         std::cerr << "Write Error " << errno << ": " << strerror(errno)
//         << std::endl; return Status::WRITE_ERROR;
//       }

//       std::cerr << strerror(errno) << std::endl;

//       return Status::RESOURCE_TEMPORARILY_UNAVAILABLE;
//     }

//     // bytes_written = ret;
//     //return Status::OK;
//   }
//   else
//   {
//     return Status::RESOURCE_TEMPORARILY_UNAVAILABLE;
//   }

//   // Write CRC
//   if (wait())
//   {
//     int ret = ::write(serial_port_, crc.data(), CRC16_OVERHEAD);

//     if (ret != CRC16_OVERHEAD) {
//       if (errno != EAGAIN)
//       {
//         std::cerr << "Write Error " << errno << ": " << strerror(errno)
//         << std::endl; return Status::WRITE_ERROR;
//       }

//       std::cerr << strerror(errno) << std::endl;

//       return Status::RESOURCE_TEMPORARILY_UNAVAILABLE;
//     }

//     // bytes_written = ret;
//     // return Status::OK;
//   }
//   else
//   {
//     return Status::RESOURCE_TEMPORARILY_UNAVAILABLE;
//   }

//   bytes_written = len;
//   return Status::OK;

//   // // Write payload
//   // wait();
//   // ret = ::write(serial_port_, buf, len);

//   // if (ret != len) {
//   //   if (errno != EAGAIN)
//   //   {
//   //     std::cerr << "Write Error " << errno << ": " << strerror(errno)
//   << std::endl;
//   //     return Status::WRITE_ERROR;
//   //   }

//   //   return Status::RESOURCE_TEMPORARILY_UNAVAILABLE;
//   // }

//   // // Write CRC16
//   // wait();
//   // ret = ::write(serial_port_, crc, CRC16_OVERHEAD);

//   // if (ret != CRC16_OVERHEAD)
//   // {
//   //   if (errno != EAGAIN)
//   //   {
//   //     std::cerr << "Write Error " << errno << ": " << strerror(errno)
//   << std::endl;
//   //     return Status::WRITE_ERROR;
//   //   }

//   //   return Status::RESOURCE_TEMPORARILY_UNAVAILABLE;
//   // }

// }

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

bool SerialTransport::fillFrameHeader(uint8_t *header,
                                      const uint16_t payload_len) {
  if (header == nullptr) {
    return false;
  }

  header[0] = FRAME_HEADER1;
  header[1] = FRAME_HEADER2;
  header[2] = (uint8_t)(payload_len & 0xFF);
  header[3] = (uint8_t)(payload_len >> 8);

  return true;
}

bool SerialTransport::fillCRC16(const uint8_t *payload,
                                const uint16_t payload_len, uint8_t *crc) {
  if (payload == NULL || crc == NULL) {
    return false;
  }

  uint16_t crc16 = getCRC16(payload, payload_len);
  // std::cout << "CRC: " << crc16 << std::endl;
  crc[0] = (uint8_t)(crc16 & 0xFF);
  crc[1] = (uint8_t)(crc16 >> 8);

  return true;
}

bool SerialTransport::checkFramedPayload(const uint8_t *payload,
                                         const size_t payload_len,
                                         const uint16_t frame_crc) {
  return getCRC16(payload, payload_len) == frame_crc;
}

Status SerialTransport::getPayloadLength(const uint8_t *header_buf,
                                         size_t &length) {
  if (header_buf[0] != FRAME_HEADER1 || header_buf[1] != FRAME_HEADER2) {
    return Status::INVALID_HEADER;
  }

  length = header_buf[2] | (header_buf[3] << 8);
  return Status::OK;
}
