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

#ifndef INC_PROTONCPP_TRANSPORT_UDP4_HPP_
#define INC_PROTONCPP_TRANSPORT_UDP4_HPP_

#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>


#include "protoncpp/transport/transport.hpp"

namespace proton
{

using socket_endpoint = std::pair<std::string, uint32_t>;

class Udp4Transport : public Transport
{
public:
  enum {
    SOCKET_NODE,
    SOCKET_PEER,
    SOCKET_COUNT
  } sockets;

  Udp4Transport(socket_endpoint node, socket_endpoint peer);

  proton_status_e connect() override;
  proton_status_e disconnect() override;
  proton_status_e read(uint8_t *buf, const size_t& len, size_t& bytes_read) override;
  proton_status_e write(const uint8_t *buf, const size_t& len, size_t& bytes_written) override;

  in_addr_t ipToInaddr(const std::string& ip);
  int initSocket(socket_endpoint s, bool server, bool blocking);

public:
  socket_endpoint socket_endpoints_[SOCKET_COUNT];
  int socket_[SOCKET_COUNT];
};

}

#endif  // INC_PROTONCPP_TRANSPORT_UDP4_HPP_
