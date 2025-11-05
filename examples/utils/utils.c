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
#include "protonc/proton.h"

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

  printf("Opened serial device %d\r\n", serial_port);

  return serial_port;
}

size_t serial_read(int serial_port, uint8_t *buf, size_t len) {
  // Read header first
  int ret = read(serial_port, buf, PROTON_FRAME_HEADER_OVERHEAD);

  if (ret < 0) {
    return 0;
  }

  // Get payload length from header
  uint16_t payload_len;
  if (PROTON_GetFramedPayloadLength(buf, &payload_len) != PROTON_OK)
  {
    return 0;
  }

  // Invalid header
  if (payload_len == 0)
  {
    return 0;
  }

  // Read payload
  ret = read(serial_port, buf, payload_len);

  if (ret != payload_len)
  {
    return 0;
  }

  uint8_t crc[2];

  ret = read(serial_port, crc, PROTON_FRAME_CRC_OVERHEAD);

  // Check for valid CRC16
  if (ret != PROTON_FRAME_CRC_OVERHEAD || PROTON_CheckFramedPayload(buf, payload_len, (uint16_t)(crc[0] | (crc[1] << 8))) != PROTON_OK)
  {
    return 0;
  }

  return payload_len;
}

size_t serial_write(int serial_port, const uint8_t *buf, size_t len) {
  uint8_t header[4];
  uint8_t crc[2];

  if (PROTON_FillFrameHeader(header, len) != PROTON_OK)
  {
    return 0;
  }

  if (PROTON_FillCRC16(buf, len, crc) != PROTON_OK)
  {
    return 0;
  }

  // Write header
  int ret = write(serial_port, header, PROTON_FRAME_HEADER_OVERHEAD);

  if (ret != PROTON_FRAME_HEADER_OVERHEAD) {
    return 0;
  }

  // Write payload
  ret = write(serial_port, buf, len);

  if (ret != len) {
    return 0;
  }

  // Write CRC16
  ret = write(serial_port, crc, PROTON_FRAME_CRC_OVERHEAD);

  if (ret != PROTON_FRAME_CRC_OVERHEAD) {
    return 0;
  }

  return len;
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