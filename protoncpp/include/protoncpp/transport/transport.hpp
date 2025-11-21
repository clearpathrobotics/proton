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

  void setState(TransportState state) { state_ = state; }
  TransportState getState() { return state_; }
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

  TransportState getTransportState() {
    if (transport_)
    {
      return transport_->getState();
    }

    return TransportState::ERROR;
  }

  Status connect() {
    if (transport_)
    {
      Status status = transport_->connect();
      if (status != Status::OK)
      {
        onError(status);
      }
      else
      {
        transport_->setState(TransportState::CONNECTED);
      }

      return status;
    }
    return Status::NULL_PTR;
  }

  Status disconnect() {
    if (transport_)
    {
      Status status = transport_->disconnect();

      if (status != Status::OK)
      {
        onError(status);
      }
      else
      {
        transport_->setState(TransportState::DISCONNECTED);
      }

      return status;
    }
    return Status::NULL_PTR;
  }

  Status read(uint8_t *buf, const size_t& len, size_t& bytes_read)
  {
    if (transport_)
    {
      Status status = transport_->read(buf, len, bytes_read);
      if (status != Status::OK)
      {
        onError(status);
      }

      return status;
    }

    return Status::NULL_PTR;
  }

  Status write(const uint8_t *buf, const size_t& len, size_t& bytes_written)
  {
    if (transport_)
    {
      Status status = transport_->write(buf, len, bytes_written);
      if (status != Status::OK)
      {
        onError(status);
      }

      return status;
    }

    return Status::NULL_PTR;
  }

  void onError(Status error)
  {
    switch(error)
    {
      case Status::OK:
      {
        return;
      }

      case Status::ERROR:
      case Status::READ_ERROR:
      case Status::WRITE_ERROR:
      case Status::CONNECTION_ERROR:
      {
        std::cout << "Transport Error " << error << std::endl;
        transport_->setState(TransportState::ERROR);
        break;
      }

      case Status::NULL_PTR:
      {
        throw std::runtime_error("NULL POINTER ERROR");
      }

      default:
      {
        break;
      }
    }
  }

protected:
  std::unique_ptr<Transport> transport_;
};

} // namespace proton

#endif // INC_PROTONCPP_TRANSPORT_TRANSPORT_HPP_
