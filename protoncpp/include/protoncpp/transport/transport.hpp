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

#include "proton/common.h"

namespace proton {

class Transport {
public:
  Transport() : state_(PROTON_TRANSPORT_DISCONNECTED) {}

  virtual ~Transport()
  {}

  virtual proton_status_e connect() = 0;
  virtual proton_status_e disconnect() = 0;

  virtual proton_status_e read(uint8_t *buf, const size_t& len, size_t& bytes_read) = 0;
  virtual proton_status_e write(const uint8_t *buf, const size_t& len, size_t& bytes_written) = 0;

  void setState(proton_transport_state_e state) { state_ = state; }
  proton_transport_state_e getState() { return state_; }
  bool connected() { return state_ == PROTON_TRANSPORT_CONNECTED; }

protected:
  proton_transport_state_e state_;
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

  proton_transport_state_e getTransportState() {
    if (transport_)
    {
      return transport_->getState();
    }

    return PROTON_TRANSPORT_ERROR;
  }

  proton_status_e connect() {
    if (transport_)
    {
      proton_status_e status = transport_->connect();
      if (status != PROTON_OK)
      {
        onError(status);
      }
      else
      {
        transport_->setState(PROTON_TRANSPORT_CONNECTED);
      }

      return status;
    }
    return PROTON_NULL_PTR_ERROR;
  }

  proton_status_e disconnect() {
    if (transport_)
    {
      proton_status_e status = transport_->disconnect();

      if (status != PROTON_OK)
      {
        onError(status);
      }
      else
      {
        transport_->setState(PROTON_TRANSPORT_DISCONNECTED);
      }

      return status;
    }
    return PROTON_NULL_PTR_ERROR;
  }

  proton_status_e read(uint8_t *buf, const size_t& len, size_t& bytes_read)
  {
    if (transport_)
    {
      proton_status_e status = transport_->read(buf, len, bytes_read);
      if (status != PROTON_OK)
      {
        onError(status);
      }
      else
      {
        rx_ += bytes_read;
      }

      return status;
    }

    return PROTON_NULL_PTR_ERROR;
  }

  proton_status_e write(const uint8_t *buf, const size_t& len, size_t& bytes_written)
  {
    if (transport_)
    {
      proton_status_e status = transport_->write(buf, len, bytes_written);
      if (status != PROTON_OK)
      {
        onError(status);
      }
      else
      {
        tx_ += bytes_written;
      }

      return status;
    }

    return PROTON_NULL_PTR_ERROR;
  }

  void onError(proton_status_e error)
  {
    switch(error)
    {
      case PROTON_OK:
      {
        return;
      }

      case PROTON_ERROR:
      case PROTON_READ_ERROR:
      case PROTON_WRITE_ERROR:
      case PROTON_CONNECT_ERROR:
      case PROTON_DISCONNECT_ERROR:
      {
        std::cout << "Transport Error " << error << std::endl;
        transport_->setState(PROTON_TRANSPORT_ERROR);
        break;
      }

      case PROTON_NULL_PTR_ERROR:
      {
        throw std::runtime_error("NULL POINTER ERROR");
      }

      default:
      {
        break;
      }
    }
  }

  uint64_t getRx() { return rx_; }
  uint64_t getTx() { return tx_; }

  void resetRx() { rx_ = 0; }
  void resetTx() { tx_ = 0; }

private:
  std::unique_ptr<Transport> transport_;
  uint64_t rx_, tx_;
};

} // namespace proton

#endif // INC_PROTONCPP_TRANSPORT_TRANSPORT_HPP_
