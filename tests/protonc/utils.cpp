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

#include "protonc/utils.hpp"

uint8_t read_buf[256];
uint8_t write_buf[256];
proton_buffer_t proton_producer_buffer = {write_buf, 256};
proton_buffer_t proton_consumer_buffer = {read_buf, 256};

bool PROTON_TRANSPORT__ConsumerConnect()
{
  return true;
}

bool PROTON_TRANSPORT__ConsumerDisconnect()
{
  return true;
}

size_t PROTON_TRANSPORT__ConsumerRead(uint8_t * buf, size_t len)
{
  memcpy(buf, proton_consumer_buffer.data, len);
  return len;
}

size_t PROTON_TRANSPORT__ConsumerWrite(const uint8_t * buf, size_t len)
{
  return len;
}

