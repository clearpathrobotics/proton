#include "proton__a300_pc.h"
#include "utils.h"



uint8_t write_buf_[PROTON_MAX_MESSAGE_SIZE];
uint8_t read_buf_[PROTON_MAX_MESSAGE_SIZE];

double rx, tx;

uint32_t last_mcu_heartbeat = 0;

proton_buffer_t proton_pc_buffer = {write_buf_, PROTON_MAX_MESSAGE_SIZE};
proton_buffer_t proton_mcu_buffer = {read_buf_, PROTON_MAX_MESSAGE_SIZE};

int sock_send, sock_recv;

#define MAX_LOGS 100

char logs[MAX_LOGS][PROTON__BUNDLE__LOG__SIGNAL__MSG__CAPACITY];
uint8_t log_index = 0;

typedef enum {
  CALLBACK_LOG,
  CALLBACK_STATUS,
  CALLBACK_POWER,
  CALLBACK_ESTOP,
  CALLBACK_TEMPERATURE,
  CALLBACK_STOP_STATUS,
  CALLBACK_PINOUT_STATE,
  CALLBACK_ALERTS,
  CALLBACK_COUNT
} callback_e;

uint32_t cb_counts[CALLBACK_COUNT];

void proton_bundle_log_callback() {
  cb_counts[CALLBACK_LOG]++;
  strncpy(logs[log_index++], log_bundle.msg,
          PROTON__BUNDLE__LOG__SIGNAL__MSG__CAPACITY);
  memset(log_bundle.msg, '\0', PROTON__BUNDLE__LOG__SIGNAL__MSG__CAPACITY);
}

void proton_bundle_status_callback() {
  cb_counts[CALLBACK_STATUS]++;
}

void proton_bundle_power_callback() { cb_counts[CALLBACK_POWER]++; }

void proton_bundle_emergency_stop_callback() { cb_counts[CALLBACK_ESTOP]++; }

void proton_bundle_temperature_callback() { cb_counts[CALLBACK_TEMPERATURE]++; }

void proton_bundle_stop_status_callback() { cb_counts[CALLBACK_STOP_STATUS]++; }

void proton_bundle_pinout_state_callback() { cb_counts[CALLBACK_PINOUT_STATE]++; }

void proton_bundle_alerts_callback() { cb_counts[CALLBACK_ALERTS]++; }

void proton_bundle_mcu_heartbeat_callback()
{
  printf("Heartbeat received %u\r\n", mcu_heartbeat_bundle.heartbeat);
  pc_node.peers[PROTON__PEER__MCU].state = PROTON_NODE_ACTIVE;
  last_mcu_heartbeat = time(NULL);
}

void update_lights() {
  for (uint8_t i = 0; i < PROTON__BUNDLE__CMD_LIGHTS__SIGNAL__LIGHTS__LENGTH; i++) {
    for (uint8_t j = 0; j < PROTON__BUNDLE__CMD_LIGHTS__SIGNAL__LIGHTS__CAPACITY; j++) {
      cmd_lights_bundle.lights[i][j] = rand_uint8();
    }
  }

  proton_bundle_send(PROTON__BUNDLE__CMD_LIGHTS);
}

void update_fans() {
  for (uint8_t i = 0; i < PROTON__BUNDLE__CMD_FANS__SIGNAL__FANS__CAPACITY; i++) {
    cmd_fans_bundle.fans[i] = rand_uint8();
  }

  proton_bundle_send(PROTON__BUNDLE__CMD_FANS);
}

void update_display_status() {
  strncpy(display_status_bundle.string1, "TEST_STRING",
          PROTON__BUNDLE__DISPLAY_STATUS__SIGNAL__STRING1__CAPACITY);
  strncpy(display_status_bundle.string2, "TEST_STRING2",
          PROTON__BUNDLE__DISPLAY_STATUS__SIGNAL__STRING2__CAPACITY);

  proton_bundle_send(PROTON__BUNDLE__DISPLAY_STATUS);
}

void update_battery() {
  battery_bundle.percentage = rand_float();
  proton_bundle_send(PROTON__BUNDLE__BATTERY);
}

void update_pinout_command() {
  for (uint8_t i = 0; i < PROTON__BUNDLE__PINOUT_COMMAND__SIGNAL__RAILS__LENGTH; i++) {
    pinout_command_bundle.rails[i] = rand_bool();
  }
  for (uint8_t i = 0; i < PROTON__BUNDLE__PINOUT_COMMAND__SIGNAL__OUTPUTS__LENGTH; i++) {
    pinout_command_bundle.outputs[i] = rand_uint32();
  }

  proton_bundle_send(PROTON__BUNDLE__PINOUT_COMMAND);
}

bool proton_node_mcu_transport_connect() {
  sock_recv = socket_init(PROTON__NODE__PC__ENDPOINT__0__IPHL, PROTON__NODE__PC__ENDPOINT__0__PORT, true);
  sock_send = socket_init(PROTON__NODE__MCU__ENDPOINT__0__IPHL, PROTON__NODE__MCU__ENDPOINT__0__PORT, false);

  return (sock_recv >= 0 && sock_send >= 0);
}

bool proton_node_mcu_transport_disconnect() { return true; }

size_t proton_node_mcu_transport_read(uint8_t *buf, size_t len) {
  int ret = recv(sock_recv, buf, len, 0);

  if (ret < 0) {
    return 0;
  }

  rx += ret;

  return ret;
}

size_t proton_node_mcu_transport_write(const uint8_t *buf, size_t len) {
  int ret = send(sock_send, buf, len, 0);

  if (ret < 0) {
    return 0;
  }

  tx += ret;

  return ret;
}

pthread_mutex_t pc_lock, mcu_lock;

bool proton_node_mcu_lock() { return pthread_mutex_lock(&mcu_lock) == 0; }
bool proton_node_mcu_unlock() { return pthread_mutex_unlock(&mcu_lock) == 0; }
bool proton_node_pc_lock() { return pthread_mutex_lock(&pc_lock) == 0; }
bool proton_node_pc_unlock() { return pthread_mutex_unlock(&pc_lock) == 0; }

void *timer_1hz(void *arg) {
  uint32_t i = 0;
  while (1) {
    update_fans();
    update_display_status();
    update_battery();
    update_pinout_command();
    proton_bundle_send_heartbeat();
    msleep(1000);

    if (time(NULL) - last_mcu_heartbeat > PROTON__NODE__MCU__HEARTBEAT__PERIOD / 1000)
    {
      pc_node.peers[PROTON__PEER__MCU].state = PROTON_NODE_INACTIVE;
    }
  }
}

void *timer_20hz(void *arg) {
  uint32_t i = 0;
  while (1) {
    update_lights();
    msleep(50);
  }
}

void *stats(void *arg) {
  uint32_t i = 0;
  while (1) {
    printf("\033[2J\033[1;1H");
    printf("--------- A300 PC C ---------\r\n");
    printf("Node: %u\r\n", pc_node.state);
    printf("Peer: %s\r\n", PROTON__NODE__MCU__NAME);
    printf("  State: %u, Transport: %u\r\n", pc_node.peers[PROTON__PEER__MCU].state, pc_node.peers[PROTON__PEER__MCU].transport.state);
    printf("Rx: %.3lf KB/s Tx: %.3lf KB/s\r\n", rx / 1000, tx / 1000);
    printf("--- Received Bundles (hz) ---\r\n");
    printf("log: %d\r\n", cb_counts[CALLBACK_LOG]);
    printf("status: %d\r\n", cb_counts[CALLBACK_STATUS]);
    printf("power: %d\r\n", cb_counts[CALLBACK_POWER]);
    printf("emergency_stop: %d\r\n", cb_counts[CALLBACK_ESTOP]);
    printf("temperature: %d\r\n", cb_counts[CALLBACK_TEMPERATURE]);
    printf("stop_status: %d\r\n", cb_counts[CALLBACK_STOP_STATUS]);
    printf("pinout_state: %d\r\n", cb_counts[CALLBACK_PINOUT_STATE]);
    printf("alerts: %d\r\n", cb_counts[CALLBACK_ALERTS]);
    printf("----------- Logs ------------\r\n");

    for (uint8_t i = 0; i < log_index; i++) {
      printf("%s\r\n", logs[i]);
    }

    printf("-----------------------------\r\n");

    memset(logs, '\0', sizeof(logs));
    log_index = 0;

    rx = 0.0;
    tx = 0.0;

    for (uint8_t i = 0; i < CALLBACK_COUNT; i++) {
      cb_counts[i] = 0;
    }

    msleep(1000);
  }
}

int main() {
  printf("~~~~~~~ A300 node ~~~~~~~\r\n");

  pthread_mutex_init(&pc_lock, NULL);
  pthread_mutex_init(&mcu_lock, NULL);

  proton_init();

  pthread_t thread_20hz, thread_1hz, thread_stats;

  pthread_create(&thread_20hz, NULL, &timer_20hz, NULL);
  pthread_create(&thread_1hz, NULL, &timer_1hz, NULL);
  pthread_create(&thread_stats, NULL, &stats, NULL);

  proton_spin(&pc_node, PROTON__PEER__MCU);

  pthread_join(thread_20hz, NULL);
  pthread_join(thread_1hz, NULL);
  pthread_join(thread_stats, NULL);

  return 0;
}
