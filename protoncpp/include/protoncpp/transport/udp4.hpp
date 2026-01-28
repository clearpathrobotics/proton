/*
 * Copyright 2026 Rockwell Automation Technologies, Inc., All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @author Roni Kreinin (roni.kreinin@rockwellautomation.com)
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
