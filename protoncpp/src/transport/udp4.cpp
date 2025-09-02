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

#include "protoncpp/transport/udp4.hpp"
#include <string.h>

using namespace proton;

Udp4Transport::Udp4Transport(socket_endpoint target, socket_endpoint peer)
{
  socket_endpoints_[SOCKET_TARGET] = target;
  socket_endpoints_[SOCKET_PEER] = peer;
  socket_[SOCKET_TARGET] = -1;
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
    std::cout << "Binding server " << std::hex << s.first << ":" << std::dec << s.second << std::endl;
    int one = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    if (::bind(sock, (struct sockaddr *)&sock_addr, sizeof(sock_addr)) != 0)
    {
      std::cerr << "Bind error" << std::endl;
      return -1;
    }
  }
  else
  {
    std::cout << "Connecting to " << std::hex << s.first << ":" << std::dec << s.second << std::endl;
    if (::connect(sock, (struct sockaddr *)&sock_addr, sizeof(sock_addr)) != 0)
    {
      std::cout << "Connect error" << std::endl;
      return -1;
    }
  }

  std::cout << "Init socket " << sock << std::endl;

  return sock;
}


bool Udp4Transport::connect()
{
  if (connected_)
  {
    return true;
  }

  if (socket_[SOCKET_TARGET] == -1)
  {
    socket_[SOCKET_TARGET] = initSocket(socket_endpoints_[SOCKET_TARGET], true, false);
  }

  if (socket_[SOCKET_PEER] == -1)
  {
    socket_[SOCKET_PEER] = initSocket(socket_endpoints_[SOCKET_PEER], false, false);
  }

  connected_ = socket_[SOCKET_TARGET] != -1 && socket_[SOCKET_PEER] != -1;

  return connected_;
}

bool Udp4Transport::disconnect()
{
  return true;
}

size_t Udp4Transport::read(uint8_t * buf, size_t len)
{
  if (!connected_)
  {
    return 0;
  }

  int ret = ::recv(socket_[SOCKET_TARGET], buf, len, 0);

  if (ret < 0)
  {
    return 0;
  }

  return ret;
}

size_t Udp4Transport::write(const uint8_t * buf, size_t len)
{
  if (!connected_)
  {
    return 0;
  }

  int ret = send(socket_[SOCKET_PEER], buf, len, 0);

  if (ret < 0)
  {
    std::cerr << "write err " << ret << std::endl;
    return 0;
  }

  return ret;
}
