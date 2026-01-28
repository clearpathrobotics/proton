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

