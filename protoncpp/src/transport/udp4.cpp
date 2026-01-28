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

#include "protoncpp/transport/udp4.hpp"
#include <string.h>

using namespace proton;

Udp4Transport::Udp4Transport(socket_endpoint node, socket_endpoint peer)
{
  socket_endpoints_[SOCKET_NODE] = node;
  socket_endpoints_[SOCKET_PEER] = peer;
  socket_[SOCKET_NODE] = -1;
  socket_[SOCKET_PEER] = -1;
}

in_addr_t Udp4Transport::ipToInaddr(const std::string& ip) {
  struct in_addr addr;
  if (inet_pton(AF_INET, ip.c_str(), &addr) != 1) {
      throw std::runtime_error("Invalid IPv4 address: " + ip);
  }
  return addr.s_addr;
}

int Udp4Transport::initSocket(socket_endpoint s, bool server, bool blocking)
{
  int sock = ::socket(AF_INET, SOCK_DGRAM, 0);

  if (sock == -1)
  {
    return sock;
  }

  struct sockaddr_in sock_addr;
  memset(&sock_addr, 0, sizeof(sock_addr));
  sock_addr.sin_family = AF_INET;
  sock_addr.sin_addr.s_addr = ipToInaddr(s.first);
  sock_addr.sin_port = ::htons(s.second);

  if (!blocking)
  {
    if(::fcntl(sock, F_SETFL, ::fcntl(sock, F_GETFL) | O_NONBLOCK) < 0) {
      return -1;
    }
  }

  if (server)
  {
    int one = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    if (::bind(sock, (struct sockaddr *)&sock_addr, sizeof(sock_addr)) != 0)
    {
      std::cerr << "Bind error: {" << s.first << ", " << s.second << "}" << std::endl;
      return -1;
    }
  }
  else
  {
    if (::connect(sock, (struct sockaddr *)&sock_addr, sizeof(sock_addr)) != 0)
    {
      std::cout << "Connect error: {" << s.first << ", " << s.second << "}" << std::endl;
      return -1;
    }
  }

  return sock;
}

proton_status_e Udp4Transport::connect()
{
  if (state_ != PROTON_TRANSPORT_DISCONNECTED)
  {
    return PROTON_INVALID_STATE_TRANSITION_ERROR;
  }

  if (socket_[SOCKET_NODE] == -1)
  {
    socket_[SOCKET_NODE] = initSocket(socket_endpoints_[SOCKET_NODE], true, true);
  }

  if (socket_[SOCKET_PEER] == -1)
  {
    socket_[SOCKET_PEER] = initSocket(socket_endpoints_[SOCKET_PEER], false, false);
  }


  if (socket_[SOCKET_NODE] == -1 || socket_[SOCKET_PEER] == -1)
  {
    return PROTON_CONNECT_ERROR;
  }

  return PROTON_OK;
}

proton_status_e Udp4Transport::disconnect()
{
  return PROTON_OK;
}

proton_status_e Udp4Transport::read(uint8_t *buf, const size_t& len, size_t& bytes_read)
{
  if (!connected())
  {
    return PROTON_INVALID_STATE_ERROR;
  }

  if (buf == nullptr)
  {
    return PROTON_NULL_PTR_ERROR;
  }

  ssize_t ret = ::recv(socket_[SOCKET_NODE], buf, len, 0);

  if (ret < 0)
  {
    return PROTON_READ_ERROR;
  }

  bytes_read = ret;
  return PROTON_OK;
}

proton_status_e Udp4Transport::write(const uint8_t *buf, const size_t& len, size_t& bytes_written)
{
  if (!connected())
  {
    return PROTON_INVALID_STATE_ERROR;
  }

  ssize_t ret = ::send(socket_[SOCKET_PEER], buf, len, 0);

  if (ret != len)
  {
    std::cout << "Write error" << std::endl;
    return PROTON_WRITE_ERROR;
  }

  bytes_written = ret;
  return PROTON_OK;
}
