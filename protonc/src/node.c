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

#include "protonc/node.h"
#include <stdio.h>

/**
 * @brief Initialize a Proton peer.
 *
 * @param peer Peer to initialize
 * @param id Peer ID
 * @param heartbeat Heartbeat configuration
 * @param transport Transport configuration
 * @param receive_func Receive function
 * @param lock_func Lock function
 * @param unlock_func Unlock function
 * @param buffer Read buffer
 * @return proton_status_e Initialization status
 */
proton_status_e proton_init_peer(proton_peer_t * peer,
                                proton_peer_id_t id,
                                proton_heartbeat_t heartbeat,
                                proton_transport_t transport,
                                proton_receive_t receive_func,
                                proton_mutex_lock_t lock_func,
                                proton_mutex_unlock_t unlock_func,
                                proton_buffer_t buffer)
{
  if (peer && TRANSPORT_VALID(transport) && receive_func && lock_func && unlock_func && buffer.data)
  {
    if (heartbeat.enabled && heartbeat.period == 0)
    {
      return PROTON_ERROR;
    }

    peer->id = id;
    peer->heartbeat = heartbeat;
    peer->transport = transport;
    peer->receive = receive_func;
    peer->atomic_buffer.lock = lock_func;
    peer->atomic_buffer.unlock = unlock_func;
    peer->atomic_buffer.buffer = buffer;
    peer->state = PROTON_NODE_INACTIVE;

    return PROTON_OK;
  }

  return PROTON_NULL_PTR_ERROR;
}

/**
 * @brief Configure a Proton node.
 *
 * @param node Node to configure
 * @param heartbeat Heartbeat configuration
 * @param lock_func Lock function
 * @param unlock_func Unlock function
 * @param buffer Write buffer
 * @param peers Peers array
 * @param peer_count Number of peers
 * @param context User-defined context
 * @return proton_status_e Configuration status
 */
proton_status_e proton_configure(proton_node_t * node,
                                 proton_heartbeat_t heartbeat,
                                 proton_mutex_lock_t lock_func,
                                 proton_mutex_unlock_t unlock_func,
                                 proton_buffer_t buffer,
                                 proton_peer_t * peers,
                                 uint16_t peer_count,
                                 void * context) {
  if (node && lock_func && unlock_func && buffer.data && peers && peer_count > 0) {
    node->peer_count = peer_count;
    node->peers = peers;
    node->heartbeat = heartbeat;
    node->atomic_buffer.buffer = buffer;
    node->atomic_buffer.lock = lock_func;
    node->atomic_buffer.unlock = unlock_func;
    node->state = PROTON_NODE_INACTIVE;
    node->context = context;

    for (uint16_t p = 0; p < peer_count; p++)
    {
      if (peers[p].state == PROTON_NODE_UNCONFIGURED)
      {
        return PROTON_INVALID_STATE_ERROR;
      }

      peers[p].transport.state = PROTON_TRANSPORT_DISCONNECTED;
    }

    return PROTON_OK;
  }

  return PROTON_NULL_PTR_ERROR;
}

/**
 * @brief Activate a Proton node.
 *
 * @param node Node to activate
 * @return proton_status_e Activation status
 */
proton_status_e proton_activate(proton_node_t *node)
{
  if (node == NULL)
  {
    return PROTON_NULL_PTR_ERROR;
  }

  if (node->state == PROTON_NODE_UNCONFIGURED)
  {
    return PROTON_INVALID_STATE_ERROR;
  }

  node->state = PROTON_NODE_ACTIVE;
  return PROTON_OK;
}

/**
 * @brief Spin a proton node for a specific peer indefinitely.
 *
 * @param node Node to spin
 * @param peer Peer index
 * @return proton_status_e Spin status
 */
proton_status_e proton_spin(proton_node_t *node, const uint8_t peer) {
  proton_status_e status;
  while (1) {
    status = proton_spin_once(node, peer);

    if (status != PROTON_OK) {
      PROTON_PRINT("Spin error %u\r\n", status);
    }
  }

  return PROTON_ERROR;
}

/**
 * @brief Spin a proton node for a specific peer once.
 *
 * @param node Node to spin
 * @param peer Peer index
 * @return proton_status_e Spin status
 */
proton_status_e proton_spin_once(proton_node_t *node, const uint8_t peer) {
  if (node == NULL || node->peers == NULL) {
    return PROTON_NULL_PTR_ERROR;
  }

  if (node->state != PROTON_NODE_ACTIVE)
  {
    return PROTON_INVALID_STATE_ERROR;
  }

  if (peer >= node->peer_count)
  {
    return PROTON_ERROR;
  }

  proton_peer_t * peer_handle = &node->peers[peer];

  if (peer_handle == NULL)
  {
    return PROTON_ERROR;
  }

  size_t bytes_read = 0;
  proton_status_e status = PROTON_OK;

  switch(peer_handle->transport.state)
  {
    case PROTON_TRANSPORT_DISCONNECTED:
    {
      status = peer_handle->transport.connect(node->context);
      if (status == PROTON_OK)
      {
        peer_handle->transport.state = PROTON_TRANSPORT_CONNECTED;
      }
      else
      {
        peer_handle->transport.state = PROTON_TRANSPORT_ERROR;
      }
      break;
    }

    case PROTON_TRANSPORT_CONNECTED:
    {
      // Lock atomic buffer
      status = peer_handle->atomic_buffer.lock(node->context);
      if (status != PROTON_OK)
      {
        PROTON_PRINT("Mutex lock error\r\n");
        return status;
      }

      // Read from peer transport
      status = peer_handle->transport.read(
                            node->context,
                            peer_handle->atomic_buffer.buffer.data,
                            peer_handle->atomic_buffer.buffer.len,
                            &bytes_read);

      if (status == PROTON_OK && bytes_read > 0) {
        // Receive bundle from read data
        if (peer_handle->receive(node, bytes_read) != PROTON_OK) {
          // Unlock atomic buffer
          if (!peer_handle->atomic_buffer.unlock(node->context))
          {
            PROTON_PRINT("Mutex unlock error\r\n");
            return PROTON_MUTEX_ERROR;
          }

          return PROTON_READ_ERROR;
        }
      }
      // Unlock atomic buffer
      status = peer_handle->atomic_buffer.unlock(node->context);
      if (status != PROTON_OK)
      {
        PROTON_PRINT("Mutex unlock error\r\n");
        return status;
      }

      break;
    }

    case PROTON_TRANSPORT_ERROR:
    {
      status = peer_handle->transport.disconnect(node->context);
      if (status == PROTON_OK)
      {
        peer_handle->transport.state = PROTON_TRANSPORT_DISCONNECTED;
      }
      break;
    }

    default:
    {
      return PROTON_ERROR;
    }
  }

  return status;
}
