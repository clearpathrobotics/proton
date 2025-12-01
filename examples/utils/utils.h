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

#ifndef INC_PROTONC_EXAMPLES_UTILS_H_
#define INC_PROTONC_EXAMPLES_UTILS_H_

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <stdarg.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <pthread.h>

void send_log(const char *file, const char* func, int line, uint8_t level, char *msg, ...);
int msleep(long msec);
int socket_init(uint32_t ip, uint32_t port, bool server);
int serial_init(const char * device);
size_t serial_read(int serial_port, uint8_t *buf, size_t len);
size_t serial_write(int serial_port, const uint8_t *buf, size_t len);

float rand_float();
double rand_double();
uint32_t rand_uint32();
uint8_t rand_uint8();
bool rand_bool();
char rand_char();

#define LOG_DEBUG(message, ...)                                                \
  send_log(__FILE_NAME__, __func__, __LINE__, 10U, message, ##__VA_ARGS__)
#define LOG_INFO(message, ...)                                                 \
  send_log(__FILE_NAME__, __func__, __LINE__, 20U, message, ##__VA_ARGS__)
#define LOG_WARNING(message, ...)                                              \
  send_log(__FILE_NAME__, __func__, __LINE__, 30U, message, ##__VA_ARGS__)
#define LOG_ERROR(message, ...)                                                \
  send_log(__FILE_NAME__, __func__, __LINE__, 40U, message, ##__VA_ARGS__)
#define LOG_FATAL(message, ...)                                                \
  send_log(__FILE_NAME__, __func__, __LINE__, 50U, message, ##__VA_ARGS__)


#endif  // INC_PROTONC_EXAMPLES_UTILS_H_