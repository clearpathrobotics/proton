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

proton_status_e proton_node_consumer_transport_connect(void * context)
{
  return PROTON_OK;
}

proton_status_e proton_node_consumer_transport_disconnect(void * context)
{
  return PROTON_OK;
}

proton_status_e proton_node_consumer_transport_read(void * context, uint8_t * buf, size_t len, size_t * bytes_read)
{
  *bytes_read = len;
  return PROTON_OK;
}

proton_status_e proton_node_consumer_transport_write(void * context, const uint8_t * buf, size_t len, size_t * bytes_written)
{
  *bytes_written = len;
  return PROTON_OK;
}

