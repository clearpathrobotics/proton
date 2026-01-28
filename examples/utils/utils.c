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

#include "utils.h"
#include "protonc/proton.h"

/**
 * @brief Define print function for proton debugging logs
 *
 * @param format
 * @return int
 */
int proton_print(const char * format, ...)
{
  va_list args;

  va_start(args, format);
  int ret = vprintf(format, args);
  va_end(args);

  return ret;
}

__attribute__((weak)) void send_log(void * context, const char *file, const char* func, int line, uint8_t level, char *msg, ...)
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
      PROTON_PRINT("Set non-blocking error\r\n");
      return -2;
    }

    if (bind(sock, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) != 0) {
      PROTON_PRINT("bind error\r\n");
      return -3;
    }
  }
  else
  {
    if (connect(sock, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) != 0) {
      PROTON_PRINT("connect error\r\n");
      return -1;
    }
  }

  return sock;
}

int serial_init(const char * device)
{
  int serial_port = open(device, O_RDWR | O_NOCTTY | O_SYNC);

  if (serial_port == -1) {
    PROTON_PRINT("Error opening serial device\r\n");
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

  PROTON_PRINT("Opened serial device %d\r\n", serial_port);

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
  if (proton_get_framed_payload_length(buf, &payload_len) != PROTON_OK)
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
  if (ret != PROTON_FRAME_CRC_OVERHEAD || proton_check_framed_payload(buf, payload_len, (uint16_t)(crc[0] | (crc[1] << 8))) != PROTON_OK)
  {
    return 0;
  }

  return payload_len;
}

size_t serial_write(int serial_port, const uint8_t *buf, size_t len) {
  uint8_t header[4];
  uint8_t crc[2];

  if (proton_fill_frame_header(header, len) != PROTON_OK)
  {
    return 0;
  }

  if (proton_fill_crc16(buf, len, crc) != PROTON_OK)
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