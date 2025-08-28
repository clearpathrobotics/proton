#include "protoncpp/proton.hpp"
#include <iostream>
#include <bits/stdc++.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <thread>
#include <chrono>

#define PROTON_MAX_MESSAGE_SIZE 1024

uint8_t write_buf_[PROTON_MAX_MESSAGE_SIZE];
uint8_t read_buf_[PROTON_MAX_MESSAGE_SIZE];

double rx, tx;

int sock_send, sock_recv;
proton::Node node;

int socket_init()
{
  struct sockaddr_in servaddr;
  sock_send = socket(AF_INET, SOCK_DGRAM, 0);

  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl((in_addr_t)0x7f000001);
  servaddr.sin_port = htons(11416);

  if (connect(sock_send, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0)
  {
    printf("connect error\r\n");
    return 1;
  }

  printf("Send Socket connected\r\n");

  struct sockaddr_in servaddr2;
  sock_recv = socket(AF_INET, SOCK_DGRAM, 0);

  memset(&servaddr2, 0, sizeof(servaddr2));
  servaddr2.sin_family = AF_INET;
  servaddr2.sin_addr.s_addr = htonl((in_addr_t)0x7f000001);
  servaddr2.sin_port = htons(11417);

  // Put the socket in non-blocking mode:
  if(fcntl(sock_recv, F_SETFL, fcntl(sock_recv, F_GETFL) | O_NONBLOCK) < 0) {
    printf("Set non-blocking error\r\n");
    return 2;
  }

  if (bind(sock_recv, (struct sockaddr *)&servaddr2, sizeof(servaddr2)) != 0)
  {
    printf("bind error\r\n");
    return 3;
  }

  printf("Receive Socket bound\r\n");

  return 0;
}

size_t PROTON_TRANSPORT__PcRead(uint8_t * buf, size_t len)
{
  int ret = recv(sock_recv, buf, len, 0);

  if (ret < 0)
  {
    return 0;
  }

  return ret;
}

size_t PROTON_TRANSPORT__PcWrite(const uint8_t *buf, size_t len)
{
  int ret = send(sock_send, buf, len, 0);

  if (ret < 0)
  {
    return 0;
  }

  return ret;
}

void send_bundle(const std::string& bundle_name)
{
  auto bundle = node.getBundle(bundle_name).bundle.get();

  if (bundle->SerializeToArray(write_buf_, PROTON_MAX_MESSAGE_SIZE))
  {
    size_t bytes_written = PROTON_TRANSPORT__PcWrite(write_buf_, bundle->ByteSize());

    if (bytes_written > 0)
    {
      tx += bytes_written;
    }
  }
}

void update_lights()
{
  node.getBundle("cmd_lights").getSignal("front_left_light").setValue<proton::bytes>({rand() % 255, rand() % 255, rand() % 255});
  node.getBundle("cmd_lights").getSignal("front_right_light").setValue<proton::bytes>({rand() % 255, rand() % 255, rand() % 255});
  node.getBundle("cmd_lights").getSignal("rear_left_light").setValue<proton::bytes>({rand() % 255, rand() % 255, rand() % 255});
  node.getBundle("cmd_lights").getSignal("rear_right_light").setValue<proton::bytes>({rand() % 255, rand() % 255, rand() % 255});

  send_bundle("cmd_lights");
}

void update_fans()
{
  node.getBundle("cmd_fans").getSignal("fan_speeds").setValue<proton::bytes>({rand() % 255, rand() % 255, rand() % 255, rand() % 255, rand() % 255, rand() % 255, rand() % 255, rand() % 255});
  send_bundle("cmd_fans");
}

void run_send_thread()
{
  uint32_t i = 0;
  while(1)
  {
    if (i % 1000 == 0)
    {
      update_fans();
    }

    if (i % 100 == 0)
    {

    }

    if (i % 50 == 0)
    {
      update_lights();
    }

    if (i % 20 == 0)
    {

    }

    i++;
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}

void run_stats_thread()
{
  while(1)
  {
    std::cout << "Rx: " << rx / 1000 << " KB/s " << "Tx: " << tx / 1000 << " KB/s" << std::endl;
    rx = 0;
    tx = 0;
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}


int main()
{
  printf("~~~~~~~ A300 node ~~~~~~~\r\n");

  node = proton::Node("/home/rkreinin/proto_ws/src/proton/protoncpp/tests/a300/config/a300.yaml");

  proton::Bundle bundle;

  socket_init();

  std::thread stats_thread(run_stats_thread);
  std::thread send_thread(run_send_thread);

  while(1)
  {
    size_t bytes_read = PROTON_TRANSPORT__PcRead(read_buf_, PROTON_MAX_MESSAGE_SIZE);
    if (bytes_read > 0)
    {
      rx += bytes_read;
      node.receiveBundle(read_buf_, bytes_read);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  return 0;
}

