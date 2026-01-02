/**
 * @file multi_node_3.c
 * @brief Node 3 implementation for multi-node example
 *
 * This file implements Node 3 of a multi-node communication example using Proton.
 * Node 3 communicates with Node 2 (via UDP socket) and Node 1 (via serial port).
 *
 * @details
 * The system uses three main threads:
 * - timer_1hz: Sends heartbeats and monitors peer connectivity (1Hz)
 * - stats: Displays network statistics and callback counts (1Hz)
 * - spin_node2: Handles Node 2 communication spin loop
 *
 * Node 3 also maintains the main spin loop for Node 1 communication in the main thread.
 *
 * @note Requires pthread, socket, and serial communication libraries
 * @note Proton framework dependent for bundle definitions and state management
 */

#include "utils.h"
#include "proton__multi_node_node3.h"
#include <stdlib.h>

/**
 * @brief Enumeration of callbacks for bundle reception.
 *
 */
typedef enum {
  CALLBACK_LOG,
  CALLBACK_TIME,
  CALLBACK_HEARTBEAT_NODE1,
  CALLBACK_HEARTBEAT_NODE2,
  CALLBACK_COUNT
} callback_e;

/**
 * @brief Context structure for Node 3 operations.
 *
 */
typedef struct {
  proton_node_t *node;
  proton_bundles_node3_t bundles;
  pthread_mutex_t node1_lock;
  pthread_mutex_t node2_lock;
  pthread_mutex_t node3_lock;
  uint32_t last_node1_heartbeat;
  uint32_t last_node2_heartbeat;
  uint32_t cb_counts[CALLBACK_COUNT];
  int sock_send;
  int sock_recv;
  int serial_port;
  double rx1;
  double tx1;
  double rx2;
  double tx2;
} context_t;

/**
 * @brief Send node name bundle
 *
 * @param context Pointer to the context_t structure
 */
void send_node_name(context_t * context) {
  if (context == NULL) return;
  proton_bundle_send(context->node, PROTON__BUNDLE__NODE_NAME);
}

/**
 * @brief Send time bundle
 *
 * @param context Pointer to the context_t structure
 */
void proton_bundle_log_callback(void * context) {
  if (context == NULL) return;
  context_t * c = (context_t *)context;
  c->cb_counts[CALLBACK_LOG]++;
}

/**
 * @brief Time bundle callback handler
 *
 * @param context Pointer to the context_t structure
 */
void proton_bundle_time_callback(void * context) {
  if (context == NULL) return;
  context_t * c = (context_t *)context;
  c->cb_counts[CALLBACK_TIME]++;
}

/**
 * @brief Node 2 heartbeat callback handler
 *
 * @param context Pointer to the context_t structure
 */
void proton_bundle_node2_heartbeat_callback(void * context)
{
  if (context == NULL) return;
  context_t * c = (context_t *)context;
  c->cb_counts[CALLBACK_HEARTBEAT_NODE2]++;
  c->node->peers[PROTON__PEER__NODE2].state = PROTON_NODE_ACTIVE;
  c->last_node2_heartbeat = time(NULL);
}

/**
 * @brief Node 1 heartbeat callback handler
 *
 * @param context Pointer to the context_t structure
 */
void proton_bundle_node1_heartbeat_callback(void * context)
{
  if (context == NULL) return;
  context_t * c = (context_t *)context;
  c->cb_counts[CALLBACK_HEARTBEAT_NODE1]++;
  c->node->peers[PROTON__PEER__NODE1].state = PROTON_NODE_ACTIVE;
  c->last_node1_heartbeat = time(NULL);
}

/**
 * @brief Node 2 transport connect
 *
 * @param context context pointer
 * @return proton_status_e return status
 */
proton_status_e proton_node_node2_transport_connect(void * context) {
  context_t * c = (context_t *)context;
  c->sock_recv = socket_init(PROTON__NODE__NODE3__ENDPOINT__1__IPHL, PROTON__NODE__NODE3__ENDPOINT__1__PORT, true);
  c->sock_send = socket_init(PROTON__NODE__NODE2__ENDPOINT__1__IPHL, PROTON__NODE__NODE2__ENDPOINT__1__PORT, false);

  return (c->sock_recv >= 0 && c->sock_send >=0) ? PROTON_OK : PROTON_CONNECT_ERROR;
}

/**
 * @brief Node 2 transport disconnect
 *
 * @param context context pointer
 * @return proton_status_e return status
 */
proton_status_e proton_node_node2_transport_disconnect(void * context) {
  (void)context;
  return PROTON_OK;
}

/**
 * @brief Node 2 transport read
 *
 * @param context context pointer
 * @param buf buffer pointer
 * @param len buffer length
 * @param bytes_read pointer to store bytes read
 * @return proton_status_e return status
 */
proton_status_e proton_node_node2_transport_read(void * context, uint8_t *buf, size_t len, size_t * bytes_read) {
  context_t * c = (context_t *)context;
  int ret = recv(c->sock_recv, buf, len, 0);

  if (ret < 0) {
    return PROTON_READ_ERROR;
  }

  *bytes_read = ret;
  c->rx2 += ret;

  return PROTON_OK;
}

/**
 * @brief Node 2 transport write
 *
 * @param context context pointer
 * @param buf buffer pointer
 * @param len buffer length
 * @param bytes_written pointer to store bytes written
 * @return proton_status_e return status
 */
proton_status_e proton_node_node2_transport_write(void * context, const uint8_t *buf, size_t len, size_t * bytes_written) {
  context_t * c = (context_t *)context;
  int ret = send(c->sock_send, buf, len, 0);
  if (ret < 0) {
    return PROTON_WRITE_ERROR;
  }

  *bytes_written = ret;
  c->tx2 += ret;

  return PROTON_OK;
}

/**
 * @brief Node 1 transport connect
 *
 * @param context context pointer
 * @return proton_status_e return status
 */
proton_status_e proton_node_node1_transport_connect(void * context) {
  context_t * c = (context_t *)context;
  c->serial_port = serial_init(PROTON__NODE__NODE1__ENDPOINT__1__DEVICE);
  return c->serial_port >= 0 ? PROTON_OK : PROTON_CONNECT_ERROR;
}

/**
 * @brief Node 1 transport disconnect
 *
 * @param context context pointer
 * @return proton_status_e return status
 */
proton_status_e proton_node_node1_transport_disconnect(void * context) {
  (void)context;
  return PROTON_OK;
}

/**
 * @brief Node 1 transport read
 *
 * @param context context pointer
 * @param buf buffer pointer
 * @param len buffer length
 * @param bytes_read pointer to store bytes read
 * @return proton_status_e return status
 */
proton_status_e proton_node_node1_transport_read(void * context, uint8_t *buf, size_t len, size_t * bytes_read) {
  context_t * c = (context_t *)context;
  size_t read = serial_read(c->serial_port, buf, len);

  if (read > 0)
  {
    c->rx1 += read + PROTON_FRAME_OVERHEAD;
  }
  else
  {
    return PROTON_READ_ERROR;
  }

  *bytes_read = read;

  return PROTON_OK;
}

/**
 * @brief Node 1 transport write
 *
 * @param context context pointer
 * @param buf buffer pointer
 * @param len buffer length
 * @param bytes_written pointer to store bytes written
 * @return proton_status_e return status
 */
proton_status_e proton_node_node1_transport_write(void * context, const uint8_t *buf, size_t len, size_t * bytes_written) {
  context_t * c = (context_t *)context;
  size_t written = serial_write(c->serial_port, buf, len);

  if (written > 0)
  {
    c->tx1 += written + PROTON_FRAME_OVERHEAD;
  }
  else
  {
    return PROTON_WRITE_ERROR;
  }

  *bytes_written = written;

  return PROTON_OK;
}

/**
 * @brief Lock Node 1 mutex
 *
 * @param context Pointer to the context_t structure
 */
proton_status_e proton_node_node1_lock(void * context) {
  if (context == NULL) return PROTON_NULL_PTR_ERROR;
  context_t * c = (context_t *)context;
  return pthread_mutex_lock(&c->node1_lock) == 0 ? PROTON_OK : PROTON_MUTEX_ERROR;
}

/**
 * @brief Unlock Node 1 mutex
 *
 * @param context Pointer to the context_t structure
 */
proton_status_e proton_node_node1_unlock(void * context) {
  if (context == NULL) return PROTON_NULL_PTR_ERROR;
  context_t * c = (context_t *)context;
  return pthread_mutex_unlock(&c->node1_lock) == 0 ? PROTON_OK : PROTON_MUTEX_ERROR;
}

/**
 * @brief Lock Node 2 mutex
 *
 * @param context Pointer to the context_t structure
 */
proton_status_e proton_node_node2_lock(void * context) {
  if (context == NULL) return PROTON_NULL_PTR_ERROR;
  context_t * c = (context_t *)context;
  return pthread_mutex_lock(&c->node2_lock) == 0 ? PROTON_OK : PROTON_MUTEX_ERROR;
}

/**
 * @brief Unlock Node 2 mutex
 *
 * @param context Pointer to the context_t structure
 */
proton_status_e proton_node_node2_unlock(void * context) {
  if (context == NULL) return PROTON_NULL_PTR_ERROR;
  context_t * c = (context_t *)context;
  return pthread_mutex_unlock(&c->node2_lock) == 0 ? PROTON_OK : PROTON_MUTEX_ERROR;
}

/**
 * @brief Lock Node 3 mutex
 *
 * @param context Pointer to the context_t structure
 */
proton_status_e proton_node_node3_lock(void * context) {
  if (context == NULL) return PROTON_NULL_PTR_ERROR;
  context_t * c = (context_t *)context;
  return pthread_mutex_lock(&c->node3_lock) == 0 ? PROTON_OK : PROTON_MUTEX_ERROR;
}

/**
 * @brief Unlock Node 3 mutex
 *
 * @param context Pointer to the context_t structure
 */
proton_status_e proton_node_node3_unlock(void * context) {
  if (context == NULL) return PROTON_NULL_PTR_ERROR;
  context_t * c = (context_t *)context;
  return pthread_mutex_unlock(&c->node3_lock) == 0 ? PROTON_OK : PROTON_MUTEX_ERROR;
}

/**
 * @brief 1Hz timer thread function
 *
 * @param arg Pointer to the context_t structure
 * @return void*
 */
void *timer_1hz(void *arg) {
  if (arg == NULL) return NULL;
  context_t * context = (context_t *)arg;
  uint32_t i = 0;

  while (1) {
    send_node_name(context);
    proton_bundle_send(context->node, PROTON_HEARTBEAT_ID);
    context->bundles.node3_heartbeat_bundle.heartbeat++;
    if (time(NULL) - context->last_node2_heartbeat > PROTON__NODE__NODE2__HEARTBEAT__PERIOD / 1000)
    {
      context->node->peers[PROTON__PEER__NODE2].state = PROTON_NODE_INACTIVE;
    }

    if (time(NULL) - context->last_node1_heartbeat > PROTON__NODE__NODE1__HEARTBEAT__PERIOD / 1000)
    {
      context->node->peers[PROTON__PEER__NODE1].state = PROTON_NODE_INACTIVE;
    }

    msleep(1000);
  }

  return NULL;
}

/**
 * @brief Stats thread function
 *
 * @param arg Pointer to the context_t structure
 * @return void*
 */
void * stats(void *arg) {
  if (arg == NULL) return NULL;
  context_t * context = (context_t *)arg;
  uint32_t i = 0;
  while (1) {
    printf("\033[2J\033[1;1H");
    printf("--------- Multi Node 3 C --------\r\n");
    printf("Node: %u\r\n", context->node->state);
    printf("Peer: %s (%s)\r\n", PROTON__NODE__NODE1__NAME, PROTON__NODE__NODE1__ENDPOINT__1__TYPE);
    printf("  State: %u, Transport: %u\r\n", context->node->peers[PROTON__PEER__NODE1].state, context->node->peers[PROTON__PEER__NODE1].transport.state);
    printf("  Rx: %.3lf KB/s Tx: %.3lf KB/s\r\n", context->rx1 / 1000, context->tx1 / 1000);
    printf("Peer: %s (%s)\r\n", PROTON__NODE__NODE2__NAME, PROTON__NODE__NODE2__ENDPOINT__1__TYPE);
    printf("  State: %u, Transport: %u\r\n", context->node->peers[PROTON__PEER__NODE2].state, context->node->peers[PROTON__PEER__NODE2].transport.state);
    printf("  Rx: %.3lf KB/s Tx: %.3lf KB/s\r\n", context->rx2 / 1000, context->tx2 / 1000);
    printf("--- Received Bundles (hz) ---\r\n");
    printf("log: %d\r\n", context->cb_counts[CALLBACK_LOG]);
    printf("time: %d\r\n", context->cb_counts[CALLBACK_TIME]);
    printf("--- Received Heartbeats (hz) ---\r\n");
    printf("%s: %d\r\n", PROTON__NODE__NODE1__NAME, context->cb_counts[CALLBACK_HEARTBEAT_NODE1]);
    printf("%s: %d\r\n", PROTON__NODE__NODE2__NAME, context->cb_counts[CALLBACK_HEARTBEAT_NODE2]);
    printf("-----------------------------\r\n");

    context->rx1 = 0.0;
    context->tx1 = 0.0;
    context->rx2 = 0.0;
    context->tx2 = 0.0;


    for (uint8_t i = 0; i < CALLBACK_COUNT; i++)
    {
      context->cb_counts[i] = 0;
    }

    msleep(1000);
  }

  return NULL;
}

/**
 * @brief Node 2 spin thread function
 *
 * @param arg Pointer to the context_t structure
 * @return void*
 */
void * spin_node2(void *arg)
{
  if (arg == NULL) return NULL;
  context_t * context = (context_t *)arg;
  proton_spin(context->node, PROTON__PEER__NODE2);

  return NULL;
}

/**
 * @brief Main function
 *
 * @return int Exit status
 */
int main() {
  pthread_t thread_1hz, thread_stats, thread_node2;

  proton_node_t node3_node = PROTON__NODE__NODE3__DEFAULT_VALUE;
  proton_peer_t node3_peers[PROTON__PEER__COUNT] = {
    PROTON__NODE__NODE1__PEER__DEFAULT_VALUE,
    PROTON__NODE__NODE2__PEER__DEFAULT_VALUE
  };

  uint8_t node1_buf[PROTON_MAX_MESSAGE_SIZE];
  uint8_t node2_buf[PROTON_MAX_MESSAGE_SIZE];
  uint8_t node3_buf[PROTON_MAX_MESSAGE_SIZE];

  proton_buffer_t proton_node1_buffer = {node1_buf, sizeof(node1_buf)};
  proton_buffer_t proton_node2_buffer = {node2_buf, sizeof(node2_buf)};
  proton_buffer_t proton_node3_buffer = {node3_buf, sizeof(node3_buf)};

  context_t context = {
    .node = &node3_node,
    .bundles = PROTON__BUNDLES__NODE3__DEFAULT_VALUE,
    .last_node1_heartbeat = 0,
    .last_node2_heartbeat = 0,
    .sock_recv = -1,
    .sock_send = -1,
    .serial_port = -1,
    .rx1 = 0.0,
    .tx1 = 0.0,
    .rx2 = 0.0,
    .tx2 = 0.0
  };


  // Init
  proton_bundles_node3_init(&context.bundles);

  pthread_mutex_init(&context.node1_lock, NULL);
  pthread_mutex_init(&context.node2_lock, NULL);
  pthread_mutex_init(&context.node3_lock, NULL);

  proton_peer_node1_init(&node3_peers[PROTON__PEER__NODE1], proton_node1_buffer);
  proton_peer_node2_init(&node3_peers[PROTON__PEER__NODE2], proton_node2_buffer);

  proton_node_node3_init(&node3_node, node3_peers, proton_node3_buffer, &context);

  pthread_create(&thread_1hz, NULL, &timer_1hz, &context);
  pthread_create(&thread_stats, NULL, &stats, &context);
  pthread_create(&thread_node2, NULL, &spin_node2, &context);

  proton_spin(&node3_node, PROTON__PEER__NODE1);

  pthread_join(thread_node2, NULL);
  pthread_join(thread_1hz, NULL);
  pthread_join(thread_stats, NULL);

  return 0;
}
