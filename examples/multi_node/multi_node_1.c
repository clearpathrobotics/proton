#include "utils.h"
#include "proton__multi_node_node1.h"
#include <stdlib.h>

#define PROTON_MAX_MESSAGE_SIZE 1024

uint8_t node1_buf[PROTON_MAX_MESSAGE_SIZE];
uint8_t node2_buf[PROTON_MAX_MESSAGE_SIZE];
uint8_t node3_buf[PROTON_MAX_MESSAGE_SIZE];

proton_buffer_t proton_node1_buffer = {node1_buf, PROTON_MAX_MESSAGE_SIZE};
proton_buffer_t proton_node2_buffer = {node2_buf, PROTON_MAX_MESSAGE_SIZE};
proton_buffer_t proton_node3_buffer = {node3_buf, PROTON_MAX_MESSAGE_SIZE};

double rx2, tx2, rx3, tx3;

uint32_t last_node2_heartbeat, last_node3_heartbeat = 0;

int sock_send, sock_recv;
int serial_port;

typedef enum {
  CALLBACK_NODE_NAME,
  CALLBACK_HEARTBEAT_NODE2,
  CALLBACK_HEARTBEAT_NODE3,
  CALLBACK_COUNT
} callback_e;

uint32_t cb_counts[CALLBACK_COUNT];


void proton_bundle_node_name_callback() {
  cb_counts[CALLBACK_NODE_NAME]++;
}

void proton_bundle_node2_heartbeat_callback()
{
  cb_counts[CALLBACK_HEARTBEAT_NODE2]++;
  node1_node.peers[PROTON__PEER__NODE2].state = PROTON_NODE_ACTIVE;
  last_node2_heartbeat = time(NULL);
}

void proton_bundle_node3_heartbeat_callback()
{
  cb_counts[CALLBACK_HEARTBEAT_NODE3]++;
  node1_node.peers[PROTON__PEER__NODE3].state = PROTON_NODE_ACTIVE;
  last_node3_heartbeat = time(NULL);
}

void send_log(const char *file, const char* func, int line, uint8_t level, char *msg, ...) {
  strcpy(log_bundle.name, "node1");
  strcpy(log_bundle.file, file);
  strcpy(log_bundle.function, func);
  log_bundle.line = line;
  log_bundle.level = level;

  va_list args;
  va_start(args, msg);
  vsprintf(log_bundle.msg, msg, args);
  va_end(args);

  proton_bundle_send(PROTON__BUNDLE__LOG);
}

bool proton_node_node2_transport_connect() {
  sock_recv = socket_init(PROTON__NODE__NODE1__ENDPOINT__0__IPHL, PROTON__NODE__NODE1__ENDPOINT__0__PORT, true);
  sock_send = socket_init(PROTON__NODE__NODE2__ENDPOINT__0__IPHL, PROTON__NODE__NODE2__ENDPOINT__0__PORT, false);

  return (sock_recv >= 0 && sock_send >=0);
}

bool proton_node_node2_transport_disconnect() { return true; }

size_t proton_node_node2_transport_read(uint8_t *buf, size_t len) {
  int ret = recv(sock_recv, buf, len, 0);

  if (ret < 0) {
    return 0;
  }

  rx2 += ret;

  return ret;
}

size_t proton_node_node2_transport_write(const uint8_t *buf, size_t len) {
  int ret = send(sock_send, buf, len, 0);

  if (ret < 0) {
    return 0;
  }

  tx2 += ret;

  return ret;
}

bool proton_node_node3_transport_connect() {
  serial_port = serial_init(PROTON__NODE__NODE3__ENDPOINT__0__DEVICE);
  return serial_port >= 0;
}

bool proton_node_node3_transport_disconnect() { return true; }

size_t proton_node_node3_transport_read(uint8_t *buf, size_t len) {
  size_t bytes_read = serial_read(serial_port, buf, len);

  if (bytes_read > 0)
  {
    rx3 += bytes_read + PROTON_FRAME_OVERHEAD;
  }

  return bytes_read;
}

size_t proton_node_node3_transport_write(const uint8_t *buf, size_t len) {
  size_t bytes_written = serial_write(serial_port, buf, len);

  if (bytes_written > 0)
  {
    tx3 += bytes_written + PROTON_FRAME_OVERHEAD;
  }

  return bytes_written;
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
    LOG_INFO("1hz timer %ld", i++);

    if (time(NULL) - last_node2_heartbeat > PROTON__NODE__NODE2__HEARTBEAT__PERIOD / 1000)
    {
      node1_node.peers[PROTON__PEER__NODE2].state = PROTON_NODE_INACTIVE;
    }

    if (time(NULL) - last_node3_heartbeat > PROTON__NODE__NODE3__HEARTBEAT__PERIOD / 1000)
    {
      node1_node.peers[PROTON__PEER__NODE3].state = PROTON_NODE_INACTIVE;
    }

    msleep(1000);
  }
}

void *timer_10hz(void *arg) {
  uint32_t i = 0;
  while (1) {
    LOG_INFO("10hz timer %ld", i++);
    proton_bundle_send_heartbeat();
    msleep(100);
  }
}

void * stats(void *arg) {
  uint32_t i = 0;
  while (1) {
    printf("\033[2J\033[1;1H");
    printf("--------- Multi Node 1 C --------\r\n");
    printf("Node: %u\r\n", node1_node.state);
    printf("Peer: %s (%s)\r\n", PROTON__NODE__NODE2__NAME, PROTON__NODE__NODE2__ENDPOINT__0__TYPE);
    printf("  State: %u, Transport: %u\r\n", node1_node.peers[PROTON__PEER__NODE2].state, node1_node.peers[PROTON__PEER__NODE2].transport.state);
    printf("  Rx: %.3lf KB/s Tx: %.3lf KB/s\r\n", rx2 / 1000, tx2 / 1000);
    printf("Peer: %s (%s)\r\n", PROTON__NODE__NODE3__NAME, PROTON__NODE__NODE3__ENDPOINT__0__TYPE);
    printf("  State: %u, Transport: %u\r\n", node1_node.peers[PROTON__PEER__NODE3].state, node1_node.peers[PROTON__PEER__NODE3].transport.state);
    printf("  Rx: %.3lf KB/s Tx: %.3lf KB/s\r\n", rx3 / 1000, tx3 / 1000);
    printf("--- Received Bundles (hz) ---\r\n");
    printf("node_name: %d\r\n", cb_counts[CALLBACK_NODE_NAME]);
    printf("--- Received Heartbeats (hz) ---\r\n");
    printf("%s: %d\r\n", PROTON__NODE__NODE2__NAME, cb_counts[CALLBACK_HEARTBEAT_NODE2]);
    printf("%s: %d\r\n", PROTON__NODE__NODE3__NAME, cb_counts[CALLBACK_HEARTBEAT_NODE3]);
    printf("-----------------------------\r\n");

    rx2 = 0.0;
    tx2 = 0.0;
    rx3 = 0.0;
    tx3 = 0.0;

    for (uint8_t i = 0; i < CALLBACK_COUNT; i++)
    {
      cb_counts[i] = 0;
    }

    msleep(1000);
  }
}

void * spin_node2()
{
  proton_spin(&node1_node, PROTON__PEER__NODE2);
}

int main() {
  pthread_mutex_init(&node1_lock, NULL);
  pthread_mutex_init(&node2_lock, NULL);
  pthread_mutex_init(&node3_lock, NULL);

  printf("INIT %d\r\n", proton_init());

  pthread_t thread_10hz, thread_1hz, thread_stats, thread_node2;

  pthread_create(&thread_10hz, NULL, &timer_10hz, NULL);
  pthread_create(&thread_1hz, NULL, &timer_1hz, NULL);
  pthread_create(&thread_stats, NULL, &stats, NULL);
  pthread_create(&thread_node2, NULL, &spin_node2, NULL);

  proton_spin(&node1_node, PROTON__PEER__NODE3);

  pthread_join(thread_node2, NULL);
  pthread_join(thread_10hz, NULL);
  pthread_join(thread_1hz, NULL);
  pthread_join(thread_stats, NULL);

  return 0;
}
