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
#include <memory>

#include "protoncpp/common.hpp"

namespace proton {

class Transport {
public:
  Transport() : state_(TransportState::DISCONNECTED) {}

  virtual ~Transport()
  {}

  virtual Status connect() = 0;
  virtual Status disconnect() = 0;

  virtual Status read(uint8_t *buf, const size_t& len, size_t& bytes_read) = 0;
  virtual Status write(const uint8_t *buf, const size_t& len, size_t& bytes_written) = 0;

  TransportState state() { return state_; }
  bool connected() { return state_ == TransportState::CONNECTED; }

protected:
  TransportState state_;
};

class TransportManager {
public:
  void setTransport(std::unique_ptr<Transport> t) {
    transport_ = std::move(t);
  }

  bool connected() {
    if (transport_)
    {
      return transport_->connected();
    }
    return false;
  }

  Status connect() {
    if (transport_)
    {
      return transport_->connect();
    }
    return Status::NULL_PTR;
  }

  Status disconnect() {
    if (transport_)
    {
      return transport_->disconnect();
    }
    return Status::NULL_PTR;
  }

  Status read(uint8_t *buf, const size_t& len, size_t& bytes_read)
  {
    if (transport_)
    {
      return transport_->read(buf, len, bytes_read);
    }

    return Status::NULL_PTR;
  }

  Status write(const uint8_t *buf, const size_t& len, size_t& bytes_written)
  {
    if (transport_)
    {
      return transport_->write(buf, len, bytes_written);
    }

    return Status::NULL_PTR;
  }

protected:
  std::unique_ptr<Transport> transport_;
};

} // namespace proton

#endif // INC_PROTONCPP_TRANSPORT_TRANSPORT_HPP_
