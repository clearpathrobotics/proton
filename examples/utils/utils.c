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

#include "utils.h"


__attribute__((weak)) void send_log(const char *file, const char* func, int line, uint8_t level, char *msg, ...)
{}

int msleep(long msec) {
  struct timespec ts;
  int res;

  if (msec < 0) {
    return -1;
  }

  ts.tv_sec = msec / 1000;
  ts.tv_nsec = (msec % 1000) * 1000000;

  do {
    res = nanosleep(&ts, &ts);
  } while (res);

  return res;
}

int socket_init(uint32_t ip, uint32_t port, bool server) {
  struct sockaddr_in sockaddr;
  int sock = socket(AF_INET, SOCK_DGRAM, 0);

  memset(&sockaddr, 0, sizeof(sockaddr));
  sockaddr.sin_family = AF_INET;
  sockaddr.sin_addr.s_addr = htonl((in_addr_t)ip);
  sockaddr.sin_port = htons(port);

  if (server)
  {
    // Put the socket in non-blocking mode:
    if (fcntl(sock, F_SETFL, fcntl(sock, F_GETFL) | O_NONBLOCK) < 0) {
      printf("Set non-blocking error\r\n");
      return -2;
    }

    if (bind(sock, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) != 0) {
      printf("bind error\r\n");
      return -3;
    }
  }
  else
  {
    if (connect(sock, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) != 0) {
      printf("connect error\r\n");
      return -1;
    }
  }

  return sock;
}

int serial_init(const char * device)
{
  int serial_port = open(device, O_RDWR | O_NOCTTY | O_SYNC);

  if (serial_port == -1) {
    printf("Error opening serial device\r\n");
    return -1;
  }

  struct termios tty;

  if (tcgetattr(serial_port, &tty) != 0) {
    return -1;
  }

  cfsetospeed(&tty, B921600);
  cfsetispeed(&tty, B921600);
  tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8; // 8-bit
  tty.c_cflag |= (CLOCAL | CREAD);            // enable receiver
  tcsetattr(serial_port, TCSANOW, &tty);

  return 0;
}

float rand_float()
{
  return (float)rand();
}

double rand_double()
{
  return (double)rand();
}

uint32_t rand_uint32()
{
  return (uint32_t)rand();
}

uint8_t rand_uint8()
{
  return (uint8_t)(rand() % 255);
}

bool rand_bool()
{
  return (bool)(rand() % 2);
}

char rand_char()
{
  static const char alphanum[] =
      "0123456789"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz";
  return alphanum[rand() % (sizeof(alphanum) - 1)];
}