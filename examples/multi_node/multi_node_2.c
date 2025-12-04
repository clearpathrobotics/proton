#include "utils.h"
#include "proton__multi_node_node2.h"
#include <stdlib.h>



uint8_t node1_buf[PROTON_MAX_MESSAGE_SIZE];
uint8_t node2_buf[PROTON_MAX_MESSAGE_SIZE];
uint8_t node3_buf[PROTON_MAX_MESSAGE_SIZE];

proton_buffer_t proton_node1_buffer = {node1_buf, PROTON_MAX_MESSAGE_SIZE};
proton_buffer_t proton_node2_buffer = {node2_buf, PROTON_MAX_MESSAGE_SIZE};
proton_buffer_t proton_node3_buffer = {node3_buf, PROTON_MAX_MESSAGE_SIZE};

double rx1, tx1, rx3, tx3;

uint32_t last_node1_heartbeat, last_node3_heartbeat = 0;

int sock_send1, sock_recv1;
int sock_send3, sock_recv3;

typedef enum {
  CALLBACK_LOG,
  CALLBACK_HEARTBEAT_NODE1,
  CALLBACK_HEARTBEAT_NODE3,
  CALLBACK_COUNT
} callback_e;

uint32_t cb_counts[CALLBACK_COUNT];

void send_node_name() {
  proton_bundle_send(PROTON__BUNDLE__NODE_NAME);
}

void send_time() {
  time_bundle.seconds = time(NULL);
  proton_bundle_send(PROTON__BUNDLE__TIME);
}

void proton_bundle_log_callback() {
  cb_counts[CALLBACK_LOG]++;
}

void proton_bundle_node1_heartbeat_callback()
{
  cb_counts[CALLBACK_HEARTBEAT_NODE1]++;
  node2_node.peers[PROTON__PEER__NODE1].state = PROTON_NODE_ACTIVE;
  last_node1_heartbeat = time(NULL);
}

void proton_bundle_node3_heartbeat_callback()
{
  cb_counts[CALLBACK_HEARTBEAT_NODE3]++;
  node2_node.peers[PROTON__PEER__NODE3].state = PROTON_NODE_ACTIVE;
  last_node3_heartbeat = time(NULL);
}

bool proton_node_node1_transport_connect() {
  sock_recv1 = socket_init(PROTON__NODE__NODE2__ENDPOINT__0__IPHL, PROTON__NODE__NODE2__ENDPOINT__0__PORT, true);
  sock_send1 = socket_init(PROTON__NODE__NODE1__ENDPOINT__0__IPHL, PROTON__NODE__NODE1__ENDPOINT__0__PORT, false);

  return (sock_recv1 >= 0 && sock_send1 >=0);
}

bool proton_node_node1_transport_disconnect() { return true; }

size_t proton_node_node1_transport_read(uint8_t *buf, size_t len) {
  int ret = recv(sock_recv1, buf, len, 0);

  if (ret < 0) {
    return 0;
  }

  rx1 += ret;

  return ret;
}

size_t proton_node_node1_transport_write(const uint8_t *buf, size_t len) {
  int ret = send(sock_send1, buf, len, 0);

  if (ret < 0) {
    return 0;
  }

  tx1 += ret;

  return ret;
}

bool proton_node_node3_transport_connect() {
  sock_recv3 = socket_init(PROTON__NODE__NODE2__ENDPOINT__1__IPHL, PROTON__NODE__NODE2__ENDPOINT__1__PORT, true);
  sock_send3 = socket_init(PROTON__NODE__NODE3__ENDPOINT__1__IPHL, PROTON__NODE__NODE3__ENDPOINT__1__PORT, false);

  return (sock_recv3 >= 0 && sock_send3 >=0);
}

bool proton_node_node3_transport_disconnect() { return true; }

size_t proton_node_node3_transport_read(uint8_t *buf, size_t len) {
  int ret = recv(sock_recv3, buf, len, 0);

  if (ret < 0) {
    return 0;
  }

  rx3 += ret;

  return ret;
}

size_t proton_node_node3_transport_write(const uint8_t *buf, size_t len) {
  int ret = send(sock_send3, buf, len, 0);

  if (ret < 0) {
    return 0;
  }

  tx3 += ret;

  return ret;
}

pthread_mutex_t node1_lock, node2_lock, node3_lock;

bool proton_node_node1_lock() { return pthread_mutex_lock(&node1_lock) == 0; }
bool proton_node_node1_unlock() { return pthread_mutex_unlock(&node1_lock) == 0; }
bool proton_node_node2_lock() { return pthread_mutex_lock(&node2_lock) == 0; }
bool proton_node_node2_unlock() { return pthread_mutex_unlock(&node2_lock) == 0; }
bool proton_node_node3_lock() { return pthread_mutex_lock(&node3_lock) == 0; }
bool proton_node_node3_unlock() { return pthread_mutex_unlock(&node3_lock) == 0; }

void *timer_1hz(void *arg) {
  uint32_t i = 0;
  while (1) {
    send_node_name();
    proton_bundle_send_heartbeat();

    if (time(NULL) - last_node1_heartbeat > PROTON__NODE__NODE2__HEARTBEAT__PERIOD / 1000)
    {
      node2_node.peers[PROTON__PEER__NODE1].state = PROTON_NODE_INACTIVE;
    }

    if (time(NULL) - last_node3_heartbeat > PROTON__NODE__NODE3__HEARTBEAT__PERIOD / 1000)
    {
      node2_node.peers[PROTON__PEER__NODE3].state = PROTON_NODE_INACTIVE;
    }

    msleep(1000);
  }
}

void *timer_10hz(void *arg) {
  uint32_t i = 0;
  while (1) {
    send_time();
    msleep(100);
  }
}

void * stats(void *arg) {
  uint32_t i = 0;
  while (1) {
    printf("\033[2J\033[1;1H");
    printf("--------- Multi Node 2 C --------\r\n");
    printf("Node: %u\r\n", node2_node.state);
    printf("Peer: %s (%s)\r\n", PROTON__NODE__NODE1__NAME, PROTON__NODE__NODE1__ENDPOINT__0__TYPE);
    printf("  State: %u, Transport: %u\r\n", node2_node.peers[PROTON__PEER__NODE1].state, node2_node.peers[PROTON__PEER__NODE1].transport.state);
    printf("  Rx: %.3lf KB/s Tx: %.3lf KB/s\r\n", rx1 / 1000, tx1 / 1000);
    printf("Peer: %s (%s)\r\n", PROTON__NODE__NODE3__NAME, PROTON__NODE__NODE3__ENDPOINT__1__TYPE);
    printf("  State: %u, Transport: %u\r\n", node2_node.peers[PROTON__PEER__NODE3].state, node2_node.peers[PROTON__PEER__NODE3].transport.state);
    printf("  Rx: %.3lf KB/s Tx: %.3lf KB/s\r\n", rx3 / 1000, tx3 / 1000);
    printf("--- Received Bundles (hz) ---\r\n");
    printf("log: %d\r\n", cb_counts[CALLBACK_LOG]);
    printf("--- Received Heartbeats (hz) ---\r\n");
    printf("%s: %d\r\n", PROTON__NODE__NODE1__NAME, cb_counts[CALLBACK_HEARTBEAT_NODE1]);
    printf("%s: %d\r\n", PROTON__NODE__NODE3__NAME, cb_counts[CALLBACK_HEARTBEAT_NODE3]);
    printf("-----------------------------\r\n");

    rx1 = 0.0;
    tx1 = 0.0;
    rx3 = 0.0;
    tx3 = 0.0;

    for (uint8_t i = 0; i < CALLBACK_COUNT; i++)
    {
      cb_counts[i] = 0;
    }

    msleep(1000);
  }
}

void * spin_node1()
{
  proton_spin(&node2_node, PROTON__PEER__NODE1);
}

int main() {
  pthread_mutex_init(&node1_lock, NULL);
  pthread_mutex_init(&node2_lock, NULL);
  pthread_mutex_init(&node3_lock, NULL);

  printf("INIT %d\r\n", proton_init());

  strncpy(node_name_bundle.name, PROTON__NODE__NODE2__NAME, PROTON__BUNDLE__NODE_NAME__SIGNAL__NAME__CAPACITY);

  pthread_t thread_10hz, thread_1hz, thread_stats, thread_node1;

  pthread_create(&thread_10hz, NULL, &timer_10hz, NULL);
  pthread_create(&thread_1hz, NULL, &timer_1hz, NULL);
  pthread_create(&thread_stats, NULL, &stats, NULL);
  pthread_create(&thread_node1, NULL, &spin_node1, NULL);

  proton_spin(&node2_node, PROTON__PEER__NODE3);

  pthread_join(thread_node1, NULL);
  pthread_join(thread_10hz, NULL);
  pthread_join(thread_1hz, NULL);
  pthread_join(thread_stats, NULL);

  return 0;
}
