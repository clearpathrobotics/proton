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

proton_buffer_t proton_producer_read_buffer;
proton_buffer_t proton_producer_write_buffer;

bool PROTON_TRANSPORT__ProducerConnect()
{
  return true;
}

bool PROTON_TRANSPORT__ProducerDisconnect()
{
  return true;
}

size_t PROTON_TRANSPORT__ProducerRead(uint8_t * buf, size_t len)
{
  memcpy(buf, proton_producer_write_buffer.data, len);
  return len;
}

size_t PROTON_TRANSPORT__ProducerWrite(const uint8_t * buf, size_t len)
{
  return len;
}

