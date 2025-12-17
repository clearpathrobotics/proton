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

#ifndef INC_PROTONC_NODE_H_
#define INC_PROTONC_NODE_H_

#include "protonc/proton.h"

typedef struct proton_peer proton_peer_t;
typedef struct proton_node proton_node_t;

typedef void (*proton_callback_t)(void *);
typedef bool (*proton_transport_connect_t)(void);
typedef bool (*proton_transport_disconnect_t)(void);
typedef size_t (*proton_transport_read_t)(uint8_t *buf, size_t len);
typedef size_t (*proton_transport_write_t)(const uint8_t *buf, size_t len);
typedef proton_status_e (*proton_receive_t)(proton_node_t * node, size_t bytes_read);
typedef bool (*proton_mutex_lock_t)(void *);
typedef bool (*proton_mutex_unlock_t)(void *);

typedef uint64_t proton_peer_id_t;

typedef struct {
  proton_buffer_t buffer;
  proton_mutex_lock_t lock;
  proton_mutex_unlock_t unlock;
} proton_atomic_buffer_t;

typedef struct {
  proton_transport_state_e state;
  proton_transport_connect_t connect;
  proton_transport_disconnect_t disconnect;
  proton_transport_read_t read;
  proton_transport_write_t write;
} proton_transport_t;

typedef struct {
  bool enabled;
  uint32_t period;
} proton_heartbeat_t;

typedef struct proton_peer {
  proton_node_state_e state;
  proton_peer_id_t id;
  proton_heartbeat_t heartbeat;
  proton_transport_t transport;
  proton_receive_t receive;
  proton_atomic_buffer_t atomic_buffer;
  const char * name;
} proton_peer_t;

typedef struct proton_node {
  proton_node_state_e state;
  proton_heartbeat_t heartbeat;
  proton_atomic_buffer_t atomic_buffer;
  uint16_t peer_count;
  proton_peer_t * peers;
  const char * name;
  void * context;
} proton_node_t;

#define PROTON_TRANSPORT_DEFAULT_VALUE {PROTON_TRANSPORT_DISCONNECTED, NULL, NULL, NULL, NULL}
#define PROTON_HEARTBEAT_DEFAULT_VALUE {false, 0}
#define PROTON_ATOMIC_BUFFER_DEFAULT_VALUE {PROTON_BUFFER_DEFAULT_VALUE, NULL, NULL}

#define PROTON_PEER_DEFAULT(name) {PROTON_NODE_UNCONFIGURED, 0, PROTON_HEARTBEAT_DEFAULT_VALUE, PROTON_TRANSPORT_DEFAULT_VALUE, NULL, PROTON_ATOMIC_BUFFER_DEFAULT_VALUE, name}
#define PROTON_NODE_DEFAULT(name) {PROTON_NODE_UNCONFIGURED, PROTON_HEARTBEAT_DEFAULT_VALUE, PROTON_ATOMIC_BUFFER_DEFAULT_VALUE, 0, NULL, name, NULL}

#define TRANSPORT_VALID(transport) (transport.connect != NULL && transport.disconnect != NULL && transport.read != NULL && transport.write != NULL)

proton_status_e proton_init_peer(proton_peer_t * peer,
                                proton_peer_id_t id,
                                proton_heartbeat_t heartbeat,
                                proton_transport_t transport,
                                proton_receive_t receive_func,
                                proton_mutex_lock_t lock_func,
                                proton_mutex_unlock_t unlock_func,
                                proton_buffer_t read_buf);

proton_status_e proton_configure(proton_node_t * node,
                                 proton_heartbeat_t heartbeat,
                                 proton_mutex_lock_t lock_func,
                                 proton_mutex_unlock_t unlock_func,
                                 proton_buffer_t write_buf,
                                 proton_peer_t * peers,
                                 uint16_t peer_count,
                                 void * context);

proton_status_e proton_activate(proton_node_t * node);

proton_status_e proton_spin(proton_node_t *node, const uint8_t peer);
proton_status_e proton_spin_once(proton_node_t *node, const uint8_t peer);

#endif // INC_PROTONC_NODE_H_
