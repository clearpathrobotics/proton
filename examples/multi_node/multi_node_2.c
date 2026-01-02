#include "utils.h"
#include "proton__multi_node_node2.h"
#include <stdlib.h>

typedef enum {
  CALLBACK_LOG,
  CALLBACK_HEARTBEAT_NODE1,
  CALLBACK_HEARTBEAT_NODE3,
  CALLBACK_COUNT
} callback_e;

typedef struct {
  proton_node_t *node;
  proton_bundles_node2_t bundles;
  pthread_mutex_t node1_lock;
  pthread_mutex_t node2_lock;
  pthread_mutex_t node3_lock;
  uint32_t last_node1_heartbeat;
  uint32_t last_node3_heartbeat;
  uint32_t cb_counts[CALLBACK_COUNT];
  int sock1_send;
  int sock1_recv;
  int sock3_send;
  int sock3_recv;
  double rx1;
  double tx1;
  double rx3;
  double tx3;
} context_t;

void send_node_name(context_t * context) {
  proton_bundle_send(context->node, PROTON__BUNDLE__NODE_NAME);
}

void send_time(context_t * context) {
  context->bundles.time_bundle.seconds = time(NULL);
  proton_bundle_send(context->node, PROTON__BUNDLE__TIME);
}

void proton_bundle_log_callback(void * context) {
  if (context == NULL) return;
  context_t * c = (context_t *)context;
  c->cb_counts[CALLBACK_LOG]++;
}

void proton_bundle_node1_heartbeat_callback(void * context)
{
  if (context == NULL) return;
  context_t * c = (context_t *)context;
  c->cb_counts[CALLBACK_HEARTBEAT_NODE1]++;
  c->node->peers[PROTON__PEER__NODE1].state = PROTON_NODE_ACTIVE;
  c->last_node1_heartbeat = time(NULL);
}

void proton_bundle_node3_heartbeat_callback(void * context)
{
  if (context == NULL) return;
  context_t * c = (context_t *)context;
  c->cb_counts[CALLBACK_HEARTBEAT_NODE3]++;
  c->node->peers[PROTON__PEER__NODE3].state = PROTON_NODE_ACTIVE;
  c->last_node3_heartbeat = time(NULL);
}

proton_status_e proton_node_node1_transport_connect(void * context) {
  context_t * c = (context_t *)context;
  c->sock1_recv = socket_init(PROTON__NODE__NODE2__ENDPOINT__0__IPHL, PROTON__NODE__NODE2__ENDPOINT__0__PORT, true);
  c->sock1_send = socket_init(PROTON__NODE__NODE1__ENDPOINT__0__IPHL, PROTON__NODE__NODE1__ENDPOINT__0__PORT, false);

  return (c->sock1_recv >= 0 && c->sock1_send >=0) ? PROTON_OK : PROTON_CONNECT_ERROR;
}

proton_status_e proton_node_node1_transport_disconnect(void * context) {
  (void)context;
  return PROTON_OK;
}

proton_status_e proton_node_node1_transport_read(void * context, uint8_t *buf, size_t len, size_t * bytes_read) {
  context_t * c = (context_t *)context;
  int ret = recv(c->sock1_recv, buf, len, 0);

  if (ret < 0) {
    return PROTON_READ_ERROR;
  }

  c->rx1 += ret;

  *bytes_read = ret;
  return PROTON_OK;
}

proton_status_e proton_node_node1_transport_write(void * context, const uint8_t *buf, size_t len, size_t * bytes_written) {
  context_t * c = (context_t *)context;
  int ret = send(c->sock1_send, buf, len, 0);
  if (ret < 0) {
    return PROTON_WRITE_ERROR;
  }

  c->tx1 += ret;
  *bytes_written = ret;

  return PROTON_OK;
}

proton_status_e proton_node_node3_transport_connect(void * context) {
  context_t * c = (context_t *)context;
  c->sock3_recv = socket_init(PROTON__NODE__NODE2__ENDPOINT__1__IPHL, PROTON__NODE__NODE2__ENDPOINT__1__PORT, true);
  c->sock3_send = socket_init(PROTON__NODE__NODE3__ENDPOINT__1__IPHL, PROTON__NODE__NODE3__ENDPOINT__1__PORT, false);

  return (c->sock3_recv >= 0 && c->sock3_send >=0) ? PROTON_OK : PROTON_CONNECT_ERROR;
}

proton_status_e proton_node_node3_transport_disconnect(void * context) {
  (void)context;
  return PROTON_OK;
}

proton_status_e proton_node_node3_transport_read(void * context, uint8_t *buf, size_t len, size_t * bytes_read) {
  context_t * c = (context_t *)context;
  int ret = recv(c->sock3_recv, buf, len, 0);

  if (ret < 0) {
    return PROTON_READ_ERROR;
  }

  c->rx3 += ret;
  *bytes_read = ret;

  return PROTON_OK;
}

proton_status_e proton_node_node3_transport_write(void * context, const uint8_t *buf, size_t len, size_t * bytes_written) {
  context_t * c = (context_t *)context;
  int ret = send(c->sock3_send, buf, len, 0);
  if (ret < 0) {
    return PROTON_WRITE_ERROR;
  }

  c->tx3 += ret;
  *bytes_written = ret;

  return PROTON_OK;
}

proton_status_e proton_node_node1_lock(void * context) {
  if (context == NULL) return PROTON_NULL_PTR_ERROR;
  context_t * c = (context_t *)context;
  return pthread_mutex_lock(&c->node1_lock) == 0 ? PROTON_OK : PROTON_MUTEX_ERROR;
}

proton_status_e proton_node_node1_unlock(void * context) {
  if (context == NULL) return PROTON_NULL_PTR_ERROR;
  context_t * c = (context_t *)context;
  return pthread_mutex_unlock(&c->node1_lock) == 0 ? PROTON_OK : PROTON_MUTEX_ERROR;
}

proton_status_e proton_node_node2_lock(void * context) {
  if (context == NULL) return PROTON_NULL_PTR_ERROR;
  context_t * c = (context_t *)context;
  return pthread_mutex_lock(&c->node2_lock) == 0 ? PROTON_OK : PROTON_MUTEX_ERROR;
}

proton_status_e proton_node_node2_unlock(void * context) {
  if (context == NULL) return PROTON_NULL_PTR_ERROR;
  context_t * c = (context_t *)context;
  return pthread_mutex_unlock(&c->node2_lock) == 0 ? PROTON_OK : PROTON_MUTEX_ERROR;
}

proton_status_e proton_node_node3_lock(void * context) {
  if (context == NULL) return PROTON_NULL_PTR_ERROR;
  context_t * c = (context_t *)context;
  return pthread_mutex_lock(&c->node3_lock) == 0 ? PROTON_OK : PROTON_MUTEX_ERROR;
}

proton_status_e proton_node_node3_unlock(void * context) {
  if (context == NULL) return PROTON_NULL_PTR_ERROR;
  context_t * c = (context_t *)context;
  return pthread_mutex_unlock(&c->node3_lock) == 0 ? PROTON_OK : PROTON_MUTEX_ERROR;
}

void *timer_1hz(void *arg) {
  if (arg == NULL) return NULL;
  context_t * context = (context_t *)arg;
  uint32_t i = 0;
  while (1) {
    send_node_name(context);

    proton_bundle_send(context->node, PROTON_HEARTBEAT_ID);
    context->bundles.node2_heartbeat_bundle.heartbeat++;

    if (time(NULL) - context->last_node1_heartbeat > PROTON__NODE__NODE2__HEARTBEAT__PERIOD / 1000)
    {
      context->node->peers[PROTON__PEER__NODE1].state = PROTON_NODE_INACTIVE;
    }

    if (time(NULL) - context->last_node3_heartbeat > PROTON__NODE__NODE3__HEARTBEAT__PERIOD / 1000)
    {
      context->node->peers[PROTON__PEER__NODE3].state = PROTON_NODE_INACTIVE;
    }

    msleep(1000);
  }
}

void *timer_10hz(void *arg)
{
  if (arg == NULL) return NULL;
  context_t * context = (context_t *)arg;
  uint32_t i = 0;
  while (1) {
    send_time(context);
    msleep(100);
  }
}

void * stats(void *arg) {
  if (arg == NULL) return NULL;
  context_t * context = (context_t *)arg;
  uint32_t i = 0;
  while (1) {
    printf("\033[2J\033[1;1H");
    printf("--------- Multi Node 2 C --------\r\n");
    printf("Node: %u\r\n", context->node->state);
    printf("Peer: %s (%s)\r\n", PROTON__NODE__NODE1__NAME, PROTON__NODE__NODE1__ENDPOINT__0__TYPE);
    printf("  State: %u, Transport: %u\r\n", context->node->peers[PROTON__PEER__NODE1].state, context->node->peers[PROTON__PEER__NODE1].transport.state);
    printf("  Rx: %.3lf KB/s Tx: %.3lf KB/s\r\n", context->rx1 / 1000, context->tx1 / 1000);
    printf("Peer: %s (%s)\r\n", PROTON__NODE__NODE3__NAME, PROTON__NODE__NODE3__ENDPOINT__1__TYPE);
    printf("  State: %u, Transport: %u\r\n", context->node->peers[PROTON__PEER__NODE3].state, context->node->peers[PROTON__PEER__NODE3].transport.state);
    printf("  Rx: %.3lf KB/s Tx: %.3lf KB/s\r\n", context->rx3 / 1000, context->tx3 / 1000);
    printf("--- Received Bundles (hz) ---\r\n");
    printf("log: %d\r\n", context->cb_counts[CALLBACK_LOG]);
    printf("--- Received Heartbeats (hz) ---\r\n");
    printf("%s: %d\r\n", PROTON__NODE__NODE1__NAME, context->cb_counts[CALLBACK_HEARTBEAT_NODE1]);
    printf("%s: %d\r\n", PROTON__NODE__NODE3__NAME, context->cb_counts[CALLBACK_HEARTBEAT_NODE3]);
    printf("-----------------------------\r\n");

    context->rx1 = 0.0;
    context->tx1 = 0.0;
    context->rx3 = 0.0;
    context->tx3 = 0.0;

    for (uint8_t i = 0; i < CALLBACK_COUNT; i++)
    {
      context->cb_counts[i] = 0;
    }

    msleep(1000);
  }
}

void * spin_node1(void *arg)
{
  context_t * context = (context_t *)arg;
  proton_spin(context->node, PROTON__PEER__NODE1);
}

int main() {
  pthread_t thread_10hz, thread_1hz, thread_stats, thread_node1;

  proton_node_t node2_node = PROTON__NODE__NODE2__DEFAULT_VALUE;
  proton_peer_t node2_peers[PROTON__PEER__COUNT] = {
    PROTON__NODE__NODE1__PEER__DEFAULT_VALUE,
    PROTON__NODE__NODE3__PEER__DEFAULT_VALUE
  };

  uint8_t node1_buf[PROTON_MAX_MESSAGE_SIZE];
  uint8_t node2_buf[PROTON_MAX_MESSAGE_SIZE];
  uint8_t node3_buf[PROTON_MAX_MESSAGE_SIZE];

  proton_buffer_t proton_node1_buffer = {node1_buf, sizeof(node1_buf)};
  proton_buffer_t proton_node2_buffer = {node2_buf, sizeof(node2_buf)};
  proton_buffer_t proton_node3_buffer = {node3_buf, sizeof(node3_buf)};

  context_t context = {
    .node = &node2_node,
    .bundles = PROTON__BUNDLES__NODE2__DEFAULT_VALUE,
    .last_node1_heartbeat = 0,
    .last_node3_heartbeat = 0,
    .cb_counts = {0},
    .sock1_send = -1,
    .sock1_recv = -1,
    .sock3_send = -1,
    .sock3_recv = -1,
    .rx1 = 0.0,
    .tx1 = 0.0,
    .rx3 = 0.0,
    .tx3 = 0.0
  };

  // Init
  proton_bundles_node2_init(&context.bundles);

  pthread_mutex_init(&context.node1_lock, NULL);
  pthread_mutex_init(&context.node2_lock, NULL);
  pthread_mutex_init(&context.node3_lock, NULL);

  proton_peer_node1_init(&node2_peers[PROTON__PEER__NODE1], proton_node1_buffer);
  proton_peer_node3_init(&node2_peers[PROTON__PEER__NODE3], proton_node3_buffer);

  proton_node_node2_init(&node2_node, node2_peers, proton_node2_buffer, &context);

  strncpy(context.bundles.node_name_bundle.name, PROTON__NODE__NODE2__NAME, PROTON__BUNDLE__NODE_NAME__SIGNAL__NAME__CAPACITY);

  // Start threads
  pthread_create(&thread_10hz, NULL, &timer_10hz, &context);
  pthread_create(&thread_1hz, NULL, &timer_1hz, &context);
  pthread_create(&thread_stats, NULL, &stats, &context);
  pthread_create(&thread_node1, NULL, &spin_node1, &context);

  proton_spin(&node2_node, PROTON__PEER__NODE3);

  pthread_join(thread_node1, NULL);
  pthread_join(thread_10hz, NULL);
  pthread_join(thread_1hz, NULL);
  pthread_join(thread_stats, NULL);

  return 0;
}
