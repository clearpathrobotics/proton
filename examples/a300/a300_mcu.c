#include "utils.h"
#include "proton__a300_mcu.h"
#include <stdlib.h>

uint8_t write_buf_[PROTON_MAX_MESSAGE_SIZE];
uint8_t read_buf_[PROTON_MAX_MESSAGE_SIZE];

double rx, tx;

uint32_t last_pc_heartbeat = 0;

proton_buffer_t proton_mcu_buffer = {write_buf_, PROTON_MAX_MESSAGE_SIZE};
proton_buffer_t proton_pc_buffer = {read_buf_, PROTON_MAX_MESSAGE_SIZE};

int sock_send, sock_recv;

typedef struct {
  proton_bundles_mcu_t bundles;
} context_t;

typedef enum {
  CALLBACK_CMD_FANS,
  CALLBACK_DISPLAY_STATUS,
  CALLBACK_CMD_LIGHTS,
  CALLBACK_BATTERY,
  CALLBACK_PINOUT_COMMAND,
  CALLBACK_CMD_SHUTDOWN,
  CALLBACK_CLEAR_NEEDS_RESET,
  CALLBACK_COUNT
} callback_e;

uint32_t cb_counts[CALLBACK_COUNT];

void proton_bundle_cmd_fans_callback() {
  cb_counts[CALLBACK_CMD_FANS]++;
}

void proton_bundle_display_status_callback() {
  cb_counts[CALLBACK_DISPLAY_STATUS]++;
}

void proton_bundle_cmd_lights_callback() {
  cb_counts[CALLBACK_CMD_LIGHTS]++;
}

void proton_bundle_battery_callback() {
  cb_counts[CALLBACK_BATTERY]++;
}

void proton_bundle_pinout_command_callback() {
  cb_counts[CALLBACK_PINOUT_COMMAND]++;
}

void proton_bundle_pc_heartbeat_callback()
{
  // printf("Heartbeat received %u\r\n", pc_heartbeat_bundle.heartbeat);
  // mcu_node.peers[PROTON__PEER__PC].state = PROTON_NODE_ACTIVE;
  // last_pc_heartbeat = time(NULL);
}

void proton_bundle_cmd_shutdown_callback() {
  // cb_counts[CALLBACK_CMD_SHUTDOWN]++;
  // strcpy(cmd_shutdown_response_bundle.message, "SHUTTING DOWN");
  // cmd_shutdown_response_bundle.success = true;

  // proton_bundle_send(PROTON__BUNDLE__CMD_SHUTDOWN_RESPONSE);

  exit(0);
}

void proton_bundle_clear_needs_reset_callback() {
  // cb_counts[CALLBACK_CLEAR_NEEDS_RESET]++;
  // stop_status_bundle.needs_reset = false;

  // strcpy(clear_needs_reset_response_bundle.message, "Needs Reset Cleared");
  // clear_needs_reset_response_bundle.success = true;

  proton_bundle_send(PROTON__BUNDLE__CLEAR_NEEDS_RESET_RESPONSE);
}

void send_log(const char *file, const char* func, int line, uint8_t level, char *msg, ...) {
  // strcpy(log_bundle.name, "A300_proton");
  // strcpy(log_bundle.file, file);
  // strcpy(log_bundle.function, func);
  // log_bundle.line = line;
  // log_bundle.level = level;

  // va_list args;
  // va_start(args, msg);
  // vsprintf(log_bundle.msg, msg, args);
  // va_end(args);

  proton_bundle_send(PROTON__BUNDLE__LOG);
}

void update_power(proton_bundle_power_t * power_bundle) {
  if (!power_bundle)
  {
    return;
  }

  for (uint8_t i = 0; i < PROTON__BUNDLE__POWER__SIGNAL__MEASURED_VOLTAGES__LENGTH;
       i++) {
    power_bundle->measured_currents[i] = rand_float();
    power_bundle->measured_voltages[i] = rand_float();
  }

  proton_bundle_send(PROTON__BUNDLE__POWER);
}

void update_temperature(proton_bundle_temperature_t * temperature_bundle) {
  if (!temperature_bundle)
  {
    return;
  }

  for (uint8_t i = 0; i < PROTON__BUNDLE__TEMPERATURE__SIGNAL__TEMPERATURES__LENGTH;
       i++) {
    temperature_bundle->temperatures[i] = rand_float();
  }

  proton_bundle_send(PROTON__BUNDLE__TEMPERATURE);
}

void update_status(proton_bundle_status_t * status_bundle, uint32_t s) {
  if (!status_bundle)
  {
    return;
  }

  status_bundle->connection_uptime_sec = s;
  status_bundle->connection_uptime_nanosec = rand_uint32();

  status_bundle->mcu_uptime_sec = s;
  status_bundle->mcu_uptime_nanosec = rand_uint32();

  proton_bundle_send(PROTON__BUNDLE__STATUS);
}

void update_emergency_stop(proton_bundle_emergency_stop_t * emergency_stop_bundle) {
  if (!emergency_stop_bundle)
  {
    return;
  }

  emergency_stop_bundle->data = !emergency_stop_bundle->data;

  proton_bundle_send(PROTON__BUNDLE__EMERGENCY_STOP);
}

void update_stop_status(proton_bundle_stop_status_t * stop_status_bundle) {
  if (!stop_status_bundle)
  {
    return;
  }

  proton_bundle_send(PROTON__BUNDLE__STOP_STATUS);
}

void update_alerts(proton_bundle_alerts_t * alerts_bundle) {
  if (!alerts_bundle)
  {
    return;
  }

  strcpy(alerts_bundle->data, "E124,E100");

  proton_bundle_send(PROTON__BUNDLE__ALERTS);
}

void update_pinout_state(proton_bundle_pinout_state_t * pinout_state_bundle) {
  if (!pinout_state_bundle)
  {
    return;
  }

  pinout_state_bundle->rails[0] = true;

  for (uint8_t i = 0; i < PROTON__BUNDLE__PINOUT_STATE__SIGNAL__OUTPUTS__LENGTH; i++) {
    pinout_state_bundle->outputs[i] = rand_bool();
  }

  for (uint8_t i = 0; i < PROTON__BUNDLE__PINOUT_STATE__SIGNAL__OUTPUT_PERIODS__LENGTH;
       i++) {
    pinout_state_bundle->output_periods[i] = rand_uint32();
  }

  proton_bundle_send(PROTON__BUNDLE__PINOUT_STATE);
}

bool proton_node_pc_transport_connect() {
  sock_recv = socket_init(PROTON__NODE__MCU__ENDPOINT__0__IPHL, PROTON__NODE__MCU__ENDPOINT__0__PORT, true);
  sock_send = socket_init(PROTON__NODE__PC__ENDPOINT__0__IPHL, PROTON__NODE__PC__ENDPOINT__0__PORT, false);

  return (sock_recv >= 0 && sock_send >=0);
}

bool proton_node_pc_transport_disconnect() { return true; }

size_t proton_node_pc_transport_read(uint8_t *buf, size_t len) {
  int ret = recv(sock_recv, buf, len, 0);

  if (ret < 0) {
    return 0;
  }

  rx += ret;

  return ret;
}

size_t proton_node_pc_transport_write(const uint8_t *buf, size_t len) {
  int ret = send(sock_send, buf, len, 0);

  if (ret < 0) {
    return 0;
  }

  tx += ret;

  return ret;
}

pthread_mutex_t mcu_lock, pc_lock;

bool proton_node_mcu_lock() { return pthread_mutex_lock(&mcu_lock) == 0; }
bool proton_node_mcu_unlock() { return pthread_mutex_unlock(&mcu_lock) == 0; }
bool proton_node_pc_lock() { return pthread_mutex_lock(&pc_lock) == 0; }
bool proton_node_pc_unlock() { return pthread_mutex_unlock(&pc_lock) == 0; }

void *timer_1hz(void *arg) {
  uint32_t i = 0;

  context_t * context = (context_t *)arg;

  while (1) {
    LOG_INFO("1hz timer %ld", i++);
    update_status(&context->bundles.status_bundle, i);
    update_emergency_stop(&context->bundles.emergency_stop_bundle);
    update_stop_status(&context->bundles.stop_status_bundle);
    update_alerts(&context->bundles.alerts_bundle);
    msleep(1000);

    if (time(NULL) - last_pc_heartbeat > PROTON__NODE__PC__HEARTBEAT__PERIOD / 1000)
    {
      mcu_node.peers[PROTON__PEER__PC].state = PROTON_NODE_INACTIVE;
    }
  }

  return NULL;
}

void *timer_10hz(void *arg) {
  uint32_t i = 0;

  context_t * context = (context_t *)arg;

  while (1) {
    LOG_INFO("10hz timer %ld", i++);
    update_power(&context->bundles.power_bundle);
    update_temperature(&context->bundles.temperature_bundle);
    update_pinout_state(&context->bundles.pinout_state_bundle);
    proton_bundle_send_heartbeat();
    msleep(100);
  }

  return NULL;
}

void * stats(void *arg) {
  uint32_t i = 0;
  while (1) {
    printf("\033[2J\033[1;1H");
    printf("--------- A300 MCU C --------\r\n");
    printf("Node: %u\r\n", mcu_node.state);
    printf("Peer: %s\r\n", PROTON__NODE__PC__NAME);
    printf("  State: %u, Transport: %u\r\n", mcu_node.peers[PROTON__PEER__PC].state, mcu_node.peers[PROTON__PEER__PC].transport.state);
    printf("Rx: %.3lf KB/s Tx: %.3lf KB/s\r\n", rx / 1000, tx / 1000);
    printf("--- Received Bundles (hz) ---\r\n");
    printf("cmd_fans: %d\r\n", cb_counts[CALLBACK_CMD_FANS]);
    printf("display_status: %d\r\n", cb_counts[CALLBACK_DISPLAY_STATUS]);
    printf("cmd_lights: %d\r\n", cb_counts[CALLBACK_CMD_LIGHTS]);
    printf("battery: %d\r\n", cb_counts[CALLBACK_BATTERY]);
    printf("pinout_command: %d\r\n", cb_counts[CALLBACK_PINOUT_COMMAND]);
    printf("cmd_shutdown: %d\r\n", cb_counts[CALLBACK_CMD_SHUTDOWN]);
    printf("clear_needs_reset: %d\r\n", cb_counts[CALLBACK_CLEAR_NEEDS_RESET]);
    printf("-----------------------------\r\n");

    rx = 0.0;
    tx = 0.0;

    for (uint8_t i = 0; i < CALLBACK_COUNT; i++)
    {
      cb_counts[i] = 0;
    }

    msleep(1000);
  }

  return NULL;
}

int main() {
  printf("~~~~~~~ A300 node ~~~~~~~\r\n");

  context_t context = {
    .bundles = PROTON__BUNDLES__MCU__DEFAULT_VALUE
  };

  proton_bundles_mcu_init(&context.bundles);

  pthread_mutex_init(&mcu_lock, NULL);
  pthread_mutex_init(&pc_lock, NULL);

  printf("INIT %d\r\n", proton_init());

  strcpy(context.bundles.status_bundle.firmware_version, "3.0.0");
  strcpy(context.bundles.status_bundle.hardware_id, "A300");
  context.bundles.stop_status_bundle.needs_reset = true;

  pthread_t thread_10hz, thread_1hz, thread_stats;

  pthread_create(&thread_10hz, NULL, &timer_10hz, &context);
  pthread_create(&thread_1hz, NULL, &timer_1hz, &context);
  pthread_create(&thread_stats, NULL, &stats, &context);

  proton_spin(&mcu_node, PROTON__PEER__PC);

  pthread_join(thread_10hz, NULL);
  pthread_join(thread_1hz, NULL);
  pthread_join(thread_stats, NULL);

  return 0;
}
