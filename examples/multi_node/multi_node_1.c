/**
 * @file multi_node_1.c
 * @brief Node 1 implementation for multi-node example
 *
 * This file implements Node 1 of a multi-node example system using Proton.
 * Node 1 communicates with Node 2 (via UDP socket) and Node 3 (via serial port).
 *
 * @details
 * - Manages bundle callbacks for node naming and heartbeat monitoring
 * - Implements transport layer callbacks for socket and serial communication
 * - Uses mutex-based thread synchronization for thread-safe peer access
 * - Monitors peer connectivity through heartbeat timeouts
 * - Provides real-time statistics on throughput and message rates
 */

#include "utils.h"
#include "proton__multi_node_node1.h"
#include <stdlib.h>

/**
 * @brief Enumeration of callbacks for bundle reception.
 *
 */
typedef enum {
  CALLBACK_NODE_NAME,
  CALLBACK_HEARTBEAT_NODE2,
  CALLBACK_HEARTBEAT_NODE3,
  CALLBACK_COUNT
} callback_e;

/**
 * @brief Context structure for Node 1 operations.
 *
 */
typedef struct {
  proton_node_t *node;
  proton_bundles_node1_t bundles;
  pthread_mutex_t node1_lock;
  pthread_mutex_t node2_lock;
  pthread_mutex_t node3_lock;
  uint32_t last_node2_heartbeat;
  uint32_t last_node3_heartbeat;
  uint32_t cb_counts[CALLBACK_COUNT];
  int sock_send;
  int sock_recv;
  int serial_port;
  double rx2;
  double tx2;
  double rx3;
  double tx3;
} context_t;

/**
 * @brief Node name callback handler
 *
 * @param context Pointer to the context_t structure
 */
void proton_bundle_node_name_callback(void * context) {
  if (context == NULL) return;
  context_t * c = (context_t *)context;
  c->cb_counts[CALLBACK_NODE_NAME]++;
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
 * @brief Node 3 heartbeat callback handler
 *
 * @param context Pointer to the context_t structure
 */
void proton_bundle_node3_heartbeat_callback(void * context)
{
  if (context == NULL) return;
  context_t * c = (context_t *)context;
  c->cb_counts[CALLBACK_HEARTBEAT_NODE3]++;
  c->node->peers[PROTON__PEER__NODE3].state = PROTON_NODE_ACTIVE;
  c->last_node3_heartbeat = time(NULL);
}

/**
 * @brief Sends a log message from Node 1
 *
 * @param context Pointer to the context_t structure
 * @param file Source file name
 * @param func Function name
 * @param line Line number
 * @param level Log level
 * @param msg Log message format string
 * @param ... Additional arguments for the log message
 */
void send_log(void * context, const char *file, const char* func, int line, uint8_t level, char *msg, ...) {
  if (context == NULL) return;
  context_t * c = (context_t *)context;
  proton_bundle_log_t * log_bundle = &c->bundles.log_bundle;
  strcpy(log_bundle->name, "node1");
  strcpy(log_bundle->file, file);
  strcpy(log_bundle->function, func);
  log_bundle->line = line;
  log_bundle->level = level;

  va_list args;
  va_start(args, msg);
  vsprintf(log_bundle->msg, msg, args);
  va_end(args);

  proton_bundle_send(c->node, PROTON__BUNDLE__LOG);
}

/**
 * @brief Connect transport for Node 2 (UDP)
 *
 * @param context Pointer to the context_t structure
 * @return proton_status_e Status of the operation
 */
proton_status_e proton_node_node2_transport_connect(void * context) {
  if (context == NULL) return PROTON_NULL_PTR_ERROR;
  context_t * c = (context_t *)context;

  c->sock_recv = socket_init(PROTON__NODE__NODE1__ENDPOINT__0__IPHL, PROTON__NODE__NODE1__ENDPOINT__0__PORT, true);
  c->sock_send = socket_init(PROTON__NODE__NODE2__ENDPOINT__0__IPHL, PROTON__NODE__NODE2__ENDPOINT__0__PORT, false);

  return (c->sock_recv >= 0 && c->sock_send >= 0) ? PROTON_OK : PROTON_CONNECT_ERROR;
}

/**
 * @brief Disconnect transport for Node 2 (UDP)
 *
 * @param context Pointer to the context_t structure
 * @return proton_status_e Status of the operation
 */
proton_status_e proton_node_node2_transport_disconnect(void * context) {
  (void)context;
  return PROTON_OK;
}

/**
 * @brief Read data from Node 2 transport (UDP)
 *
 * @param context Pointer to the context_t structure
 * @param buf Buffer to read data into
 * @param len Maximum number of bytes to read
 * @param bytes_read Pointer to store number of bytes actually read
 * @return proton_status_e Status of the operation
 */
proton_status_e proton_node_node2_transport_read(void * context, uint8_t *buf, size_t len, size_t * bytes_read) {
  if (context == NULL) return PROTON_NULL_PTR_ERROR;
  context_t * c = (context_t *)context;

  int ret = recv(c->sock_recv, buf, len, 0);

  if (ret < 0) {
    return PROTON_READ_ERROR;
  }

  c->rx2 += ret;
  *bytes_read = ret;

  return PROTON_OK;
}

/**
 * @brief Write data to Node 2 transport (UDP)
 *
 * @param context Pointer to the context_t structure
 * @param buf Buffer containing data to write
 * @param len Number of bytes to write
 * @param bytes_written Pointer to store number of bytes actually written
 * @return proton_status_e Status of the operation
 */
proton_status_e proton_node_node2_transport_write(void * context, const uint8_t *buf, size_t len, size_t * bytes_written) {
  if (context == NULL) return PROTON_NULL_PTR_ERROR;
  context_t * c = (context_t *)context;

  int ret = send(c->sock_send, buf, len, 0);

  if (ret < 0) {
    return PROTON_WRITE_ERROR;
  }

  c->tx2 += ret;
  *bytes_written = ret;
  return PROTON_OK;
}

/**
 * @brief Connect transport for Node 3 (Serial)
 *
 * @param context Pointer to the context_t structure
 * @return proton_status_e Status of the operation
 */
proton_status_e proton_node_node3_transport_connect(void * context) {
  if (context == NULL) return PROTON_NULL_PTR_ERROR;
  context_t * c = (context_t *)context;

  c->serial_port = serial_init(PROTON__NODE__NODE3__ENDPOINT__0__DEVICE);

  return c->serial_port >= 0 ? PROTON_OK : PROTON_CONNECT_ERROR;
}

/**
 * @brief Disconnect transport for Node 3 (Serial)
 *
 * @param context Pointer to the context_t structure
 * @return proton_status_e Status of the operation
 */
proton_status_e proton_node_node3_transport_disconnect(void * context) {
  (void)context;
  return PROTON_OK;
}

/**
 * @brief Read data from Node 3 transport (Serial)
 *
 * @param context Pointer to the context_t structure
 * @param buf Buffer to read data into
 * @param len Maximum number of bytes to read
 * @param bytes_read Pointer to store number of bytes actually read
 * @return proton_status_e Status of the operation
 */
proton_status_e proton_node_node3_transport_read(void * context, uint8_t *buf, size_t len, size_t * bytes_read) {
  if (context == NULL) return PROTON_NULL_PTR_ERROR;
  context_t * c = (context_t *)context;

  size_t bytes = serial_read(c->serial_port, buf, len);

  if (bytes > 0) {
    c->rx3 += bytes + PROTON_FRAME_OVERHEAD;
  }

  *bytes_read = bytes;
  return PROTON_OK;
}

/**
 * @brief Write data to Node 3 transport (Serial)
 *
 * @param context Pointer to the context_t structure
 * @param buf Buffer containing data to write
 * @param len Number of bytes to write
 * @param bytes_written Pointer to store number of bytes actually written
 * @return proton_status_e Status of the operation
 */
proton_status_e proton_node_node3_transport_write(void * context, const uint8_t *buf, size_t len, size_t * bytes_written) {
  if (context == NULL) return PROTON_NULL_PTR_ERROR;
  context_t * c = (context_t *)context;

  size_t bytes = serial_write(c->serial_port, buf, len);

  if (bytes > 0) {
    c->tx3 += bytes + PROTON_FRAME_OVERHEAD;
  }

  *bytes_written = bytes;
  return PROTON_OK;
}

/**
 * @brief Locks Node 1 mutex
 *
 * @param context Pointer to the context_t structure
 * @return proton_status_e Status of the operation
 */
proton_status_e proton_node_node1_lock(void * context) {
  if (context == NULL) {
    return PROTON_NULL_PTR_ERROR;
  }
  context_t * c = (context_t *)context;

  return pthread_mutex_lock(&c->node1_lock) == 0 ? PROTON_OK : PROTON_MUTEX_ERROR;
}

/**
 * @brief Unlocks Node 1 mutex
 *
 * @param context Pointer to the context_t structure
 * @return proton_status_e Status of the operation
 */
proton_status_e proton_node_node1_unlock(void * context) {
  if (context == NULL) {
    return PROTON_NULL_PTR_ERROR;
  }
  context_t * c = (context_t *)context;

  return pthread_mutex_unlock(&c->node1_lock) == 0 ? PROTON_OK : PROTON_MUTEX_ERROR;
}

/**
 * @brief Locks Node 2 mutex
 *
 * @param context Pointer to the context_t structure
 * @return proton_status_e Status of the operation
 */
proton_status_e proton_node_node2_lock(void * context) {
  if (context == NULL) {
    return PROTON_NULL_PTR_ERROR;
  }
  context_t * c = (context_t *)context;

  return pthread_mutex_lock(&c->node2_lock) == 0 ? PROTON_OK : PROTON_MUTEX_ERROR;
}

/**
 * @brief Unlocks Node 2 mutex
 *
 * @param context Pointer to the context_t structure
 * @return proton_status_e Status of the operation
 */
proton_status_e proton_node_node2_unlock(void * context) {
  if (context == NULL) {
    return PROTON_NULL_PTR_ERROR;
  }
  context_t * c = (context_t *)context;
  return pthread_mutex_unlock(&c->node2_lock) == 0 ? PROTON_OK : PROTON_MUTEX_ERROR;
}

/**
 * @brief Locks Node 3 mutex
 *
 * @param context Pointer to the context_t structure
 * @return proton_status_e Status of the operation
 */
proton_status_e proton_node_node3_lock(void * context) {
  if (context == NULL) {
    return PROTON_NULL_PTR_ERROR;
  }
  context_t * c = (context_t *)context;
  return pthread_mutex_lock(&c->node3_lock) == 0 ? PROTON_OK : PROTON_MUTEX_ERROR;
}

/**
 * @brief Unlocks Node 3 mutex
 *
 * @param context Pointer to the context_t structure
 * @return proton_status_e Status of the operation
 */
proton_status_e proton_node_node3_unlock(void * context) {
  if (context == NULL) {
    return PROTON_NULL_PTR_ERROR;
  }
  context_t * c = (context_t *)context;
  return pthread_mutex_unlock(&c->node3_lock) == 0 ? PROTON_OK : PROTON_MUTEX_ERROR;
}

/**
 * @brief 1 Hz timer thread function
 *
 * @param arg Pointer to the context_t structure
 * @return void*
 */
void *timer_1hz(void *arg) {
  if (arg == NULL) return NULL;
  context_t * context = (context_t *)arg;
  uint32_t i = 0;
  while (1) {
    LOG_INFO(context, "1hz timer %lu", i++);

    if (time(NULL) - context->last_node2_heartbeat > PROTON__NODE__NODE2__HEARTBEAT__PERIOD / 1000)
    {
      context->node->peers[PROTON__PEER__NODE2].state = PROTON_NODE_INACTIVE;
    }

    if (time(NULL) - context->last_node3_heartbeat > PROTON__NODE__NODE3__HEARTBEAT__PERIOD / 1000)
    {
      context->node->peers[PROTON__PEER__NODE3].state = PROTON_NODE_INACTIVE;
    }

    msleep(1000);
  }
  return NULL;
}

/**
 * @brief 10 Hz timer thread function
 *
 * @param arg Pointer to the context_t structure
 * @return void*
 */
void *timer_10hz(void *arg) {
  if (arg == NULL) return NULL;
  context_t * context = (context_t *)arg;
  uint32_t i = 0;
  while (1) {
    LOG_INFO(context, "10hz timer %lu", i++);
    proton_bundle_send(context->node, PROTON_HEARTBEAT_ID);
    context->bundles.node1_heartbeat_bundle.heartbeat++;
    msleep(100);
  }
  return NULL;
}

/**
 * @brief Statistics thread function
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
    printf("--------- Multi Node 1 C --------\r\n");
    printf("Node: %u\r\n", context->node->state);
    printf("Peer: %s (%s)\r\n", PROTON__NODE__NODE2__NAME, PROTON__NODE__NODE2__ENDPOINT__0__TYPE);
    printf("  State: %u, Transport: %u\r\n", context->node->peers[PROTON__PEER__NODE2].state, context->node->peers[PROTON__PEER__NODE2].transport.state);
    printf("  Rx: %.3lf KB/s Tx: %.3lf KB/s\r\n", context->rx2 / 1000, context->tx2 / 1000);
    printf("Peer: %s (%s)\r\n", PROTON__NODE__NODE3__NAME, PROTON__NODE__NODE3__ENDPOINT__0__TYPE);
    printf("  State: %u, Transport: %u\r\n", context->node->peers[PROTON__PEER__NODE3].state, context->node->peers[PROTON__PEER__NODE3].transport.state);
    printf("  Rx: %.3lf KB/s Tx: %.3lf KB/s\r\n", context->rx3 / 1000, context->tx3 / 1000);
    printf("--- Received Bundles (hz) ---\r\n");
    printf("node_name: %d\r\n", context->cb_counts[CALLBACK_NODE_NAME]);
    printf("--- Received Heartbeats (hz) ---\r\n");
    printf("%s: %d\r\n", PROTON__NODE__NODE2__NAME, context->cb_counts[CALLBACK_HEARTBEAT_NODE2]);
    printf("%s: %d\r\n", PROTON__NODE__NODE3__NAME, context->cb_counts[CALLBACK_HEARTBEAT_NODE3]);
    printf("-----------------------------\r\n");

    context->rx2 = 0.0;
    context->tx2 = 0.0;
    context->rx3 = 0.0;
    context->tx3 = 0.0;

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
void * spin_node2(void * arg)
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
  pthread_t thread_10hz, thread_1hz, thread_stats, thread_node2;

  proton_node_t node1_node = PROTON__NODE__NODE1__DEFAULT_VALUE;
  proton_peer_t node1_peers[PROTON__PEER__COUNT] = {
    PROTON__NODE__NODE2__PEER__DEFAULT_VALUE,
    PROTON__NODE__NODE3__PEER__DEFAULT_VALUE
  };

  uint8_t node1_buf[PROTON_MAX_MESSAGE_SIZE];
  uint8_t node2_buf[PROTON_MAX_MESSAGE_SIZE];
  uint8_t node3_buf[PROTON_MAX_MESSAGE_SIZE];

  proton_buffer_t proton_node1_buffer = {node1_buf, sizeof(node1_buf)};
  proton_buffer_t proton_node2_buffer = {node2_buf, sizeof(node2_buf)};
  proton_buffer_t proton_node3_buffer = {node3_buf, sizeof(node3_buf)};

  context_t context = {
    .node = &node1_node,
    .bundles = PROTON__BUNDLES__NODE1__DEFAULT_VALUE,
    .last_node2_heartbeat = 0,
    .last_node3_heartbeat = 0,
    .sock_send = -1,
    .sock_recv = -1,
    .serial_port = -1,
    .rx2 = 0.0,
    .tx2 = 0.0,
    .rx3 = 0.0,
    .tx3 = 0.0
  };

  // Init
  proton_bundles_node1_init(&context.bundles);

  pthread_mutex_init(&context.node1_lock, NULL);
  pthread_mutex_init(&context.node2_lock, NULL);
  pthread_mutex_init(&context.node3_lock, NULL);

  proton_peer_node2_init(&node1_peers[PROTON__PEER__NODE2], proton_node2_buffer);
  proton_peer_node3_init(&node1_peers[PROTON__PEER__NODE3], proton_node3_buffer);

  proton_node_node1_init(&node1_node, node1_peers, proton_node1_buffer, &context);

  // Start threads
  pthread_create(&thread_10hz, NULL, &timer_10hz, &context);
  pthread_create(&thread_1hz, NULL, &timer_1hz, &context);
  pthread_create(&thread_stats, NULL, &stats, &context);
  pthread_create(&thread_node2, NULL, &spin_node2, &context);

  proton_spin(context.node, PROTON__PEER__NODE3);

  pthread_join(thread_node2, NULL);
  pthread_join(thread_10hz, NULL);
  pthread_join(thread_1hz, NULL);
  pthread_join(thread_stats, NULL);

  return 0;
}
