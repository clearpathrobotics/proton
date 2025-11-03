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

#include "protoncpp/status.hpp"

namespace proton {

class Transport {
public:

  enum State {
    DISCONNECTED,
    CONNECTED,
    ERROR
  };

  Transport() : state_(State::DISCONNECTED) {}

  virtual ~Transport()
  {}

  virtual Status connect() = 0;
  virtual Status disconnect() = 0;

  virtual Status read(uint8_t *buf, const size_t& len, size_t& bytes_read) = 0;
  virtual Status write(const uint8_t *buf, const size_t& len, size_t& bytes_written) = 0;

  State state() { return state_; }
  bool connected() { return state_ == State::CONNECTED; }

protected:
  State state_;
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
    return Status::ERROR;
  }

  Status disconnect() {
    if (transport_)
    {
      return transport_->disconnect();
    }
    return Status::ERROR;
  }

  Status read(uint8_t *buf, const size_t& len, size_t& bytes_read)
  {
    if (transport_)
    {
      return transport_->read(buf, len, bytes_read);
    }

    return Status::ERROR;
  }

  Status write(const uint8_t *buf, const size_t& len, size_t& bytes_written)
  {
    if (transport_)
    {
      return transport_->write(buf, len, bytes_written);
    }

    return Status::ERROR;
  }

public:
  std::unique_ptr<Transport> transport_;
};

} // namespace proton

#endif // INC_PROTONCPP_TRANSPORT_TRANSPORT_HPP_
