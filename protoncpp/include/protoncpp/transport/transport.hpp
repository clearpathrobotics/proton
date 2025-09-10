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

namespace proton {

class Transport {
public:
  Transport() : connected_(false) {}

  virtual ~Transport()
  {}

  virtual bool connect() = 0;
  virtual bool disconnect() = 0;

  virtual size_t read(uint8_t *buf, size_t len) = 0;
  virtual size_t write(const uint8_t *buf, size_t len) = 0;

  bool connected() { return connected_; }

protected:
  bool connected_;
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

  bool connect() {
    if (transport_)
    {
      return transport_->connect();
    }
    return false;
  }

  bool disconnect() {
    if (transport_)
    {
      return transport_->disconnect();
    }
    return false;
  }

  size_t read(uint8_t *buf, size_t len)
  {
    if (transport_)
    {
      return transport_->read(buf, len);
    }

    return 0;
  }

  size_t write(const uint8_t *buf, size_t len)
  {
    if (transport_)
    {
      return transport_->write(buf, len);
    }

    return 0;
  }

public:
  std::unique_ptr<Transport> transport_;

};

} // namespace proton

#endif // INC_PROTONCPP_TRANSPORT_TRANSPORT_HPP_
