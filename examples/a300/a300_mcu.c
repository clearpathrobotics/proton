#include "utils.h"
#include "proton__a300_mcu.h"
#include <stdlib.h>

double rx, tx;

int sock_send, sock_recv;

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

typedef struct {
  proton_node_t * node;
  proton_bundles_mcu_t bundles;
  pthread_mutex_t mcu_lock;
  pthread_mutex_t pc_lock;
  uint32_t last_pc_heartbeat;
  uint32_t cb_counts[CALLBACK_COUNT];
} context_t;


void proton_bundle_cmd_fans_callback(void * context) {
  context_t * c = (context_t *)context;
  c->cb_counts[CALLBACK_CMD_FANS]++;
  printf("Received cmd_fans\r\n");
}

void proton_bundle_display_status_callback(void * context) {
  context_t * c = (context_t *)context;
  c->cb_counts[CALLBACK_DISPLAY_STATUS]++;
}

void proton_bundle_cmd_lights_callback(void * context) {
  context_t * c = (context_t *)context;
  c->cb_counts[CALLBACK_CMD_LIGHTS]++;
}

void proton_bundle_battery_callback(void * context) {
  context_t * c = (context_t *)context;
  c->cb_counts[CALLBACK_BATTERY]++;
}

void proton_bundle_pinout_command_callback(void * context) {
  context_t * c = (context_t *)context;
  c->cb_counts[CALLBACK_PINOUT_COMMAND]++;
}

void proton_bundle_pc_heartbeat_callback(void * context)
{
  context_t * c = (context_t *)context;
  printf("Heartbeat received %u\r\n", c->bundles.pc_heartbeat_bundle.heartbeat);
  c->node->peers[PROTON__PEER__PC].state = PROTON_NODE_ACTIVE;
  c->last_pc_heartbeat = time(NULL);
}

void proton_bundle_cmd_shutdown_callback(void * context) {
  context_t * c = (context_t *)context;
  c->cb_counts[CALLBACK_CMD_SHUTDOWN]++;
  strcpy(c->bundles.cmd_shutdown_response_bundle.message, "SHUTTING DOWN");
  c->bundles.cmd_shutdown_response_bundle.success = true;

  proton_bundle_send(c->node, PROTON__BUNDLE__CMD_SHUTDOWN_RESPONSE);

  exit(0);
}

void proton_bundle_clear_needs_reset_callback(void * context) {
  context_t * c = (context_t *)context;
  c->cb_counts[CALLBACK_CLEAR_NEEDS_RESET]++;
  c->bundles.stop_status_bundle.needs_reset = false;

  strcpy(c->bundles.clear_needs_reset_response_bundle.message, "Needs Reset Cleared");
  c->bundles.clear_needs_reset_response_bundle.success = true;

  proton_bundle_send(c->node, PROTON__BUNDLE__CLEAR_NEEDS_RESET_RESPONSE);
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

  //proton_bundle_send(&mcu_node, PROTON__BUNDLE__LOG);
}

void update_power(proton_node_t * node, proton_bundle_power_t * power_bundle) {
  if (!power_bundle)
  {
    return;
  }

  for (uint8_t i = 0; i < PROTON__BUNDLE__POWER__SIGNAL__MEASURED_VOLTAGES__LENGTH;
       i++) {
    power_bundle->measured_currents[i] = rand_float();
    power_bundle->measured_voltages[i] = rand_float();
  }

  proton_bundle_send(node, PROTON__BUNDLE__POWER);
}

void update_temperature(proton_node_t * node, proton_bundle_temperature_t * temperature_bundle) {
  if (!temperature_bundle)
  {
    return;
  }

  for (uint8_t i = 0; i < PROTON__BUNDLE__TEMPERATURE__SIGNAL__TEMPERATURES__LENGTH;
       i++) {
    temperature_bundle->temperatures[i] = rand_float();
  }

  proton_bundle_send(node, PROTON__BUNDLE__TEMPERATURE);
}

void update_status(proton_node_t * node, proton_bundle_status_t * status_bundle, uint32_t s) {
  if (!status_bundle)
  {
    return;
  }

  status_bundle->connection_uptime_sec = s;
  status_bundle->connection_uptime_nanosec = rand_uint32();

  status_bundle->mcu_uptime_sec = s;
  status_bundle->mcu_uptime_nanosec = rand_uint32();

  proton_bundle_send(node, PROTON__BUNDLE__STATUS);
}

void update_emergency_stop(proton_node_t * node, proton_bundle_emergency_stop_t * emergency_stop_bundle) {
  if (!emergency_stop_bundle)
  {
    return;
  }

  emergency_stop_bundle->data = !emergency_stop_bundle->data;

  proton_bundle_send(node, PROTON__BUNDLE__EMERGENCY_STOP);
}

void update_stop_status(proton_node_t * node, proton_bundle_stop_status_t * stop_status_bundle) {
  if (!stop_status_bundle)
  {
    return;
  }

  proton_bundle_send(node, PROTON__BUNDLE__STOP_STATUS);
}

void update_alerts(proton_node_t * node, proton_bundle_alerts_t * alerts_bundle) {
  if (!alerts_bundle)
  {
    return;
  }

  strcpy(alerts_bundle->data, "E124,E100");

  proton_bundle_send(node, PROTON__BUNDLE__ALERTS);
}

void update_pinout_state(proton_node_t * node, proton_bundle_pinout_state_t * pinout_state_bundle) {
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

  proton_bundle_send(node, PROTON__BUNDLE__PINOUT_STATE);
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


bool proton_node_mcu_lock(void * context) {
  if (context == NULL)
  {
    return false;
  }

  context_t * c = (context_t *)context;
  return pthread_mutex_lock(&c->mcu_lock) == 0;
}

bool proton_node_mcu_unlock(void * context) {
  if (context == NULL)
  {
    return false;
  }

  context_t * c = (context_t *)context;
  return pthread_mutex_unlock(&c->mcu_lock) == 0;
}

bool proton_node_pc_lock(void * context) {
  if (context == NULL)
  {
    return false;
  }

  context_t * c = (context_t *)context;
  return pthread_mutex_lock(&c->pc_lock) == 0;
}

bool proton_node_pc_unlock(void * context) {
  if (context == NULL)
  {
    return false;
  }

  context_t * c = (context_t *)context;
  return pthread_mutex_unlock(&c->pc_lock) == 0;
}

void *timer_1hz(void *arg) {
  if (arg == NULL)
  {
    return NULL;
  }

  uint32_t i = 0;
  context_t * context = (context_t *)arg;

  while (1) {
    LOG_INFO("1hz timer %ld", i++);
    update_status(context->node, &context->bundles.status_bundle, i);
    update_emergency_stop(context->node, &context->bundles.emergency_stop_bundle);
    update_stop_status(context->node, &context->bundles.stop_status_bundle);
    update_alerts(context->node, &context->bundles.alerts_bundle);
    msleep(1000);

    if (time(NULL) - context->last_pc_heartbeat > PROTON__NODE__PC__HEARTBEAT__PERIOD / 1000)
    {
      context->node->peers[PROTON__PEER__PC].state = PROTON_NODE_INACTIVE;
    }
  }

  return NULL;
}

void *timer_10hz(void *arg) {
  if (arg == NULL)
  {
    return NULL;
  }

  uint32_t i = 0;
  context_t * context = (context_t *)arg;

  while (1) {
    LOG_INFO("10hz timer %ld", i++);
    update_power(context->node, &context->bundles.power_bundle);
    update_temperature(context->node, &context->bundles.temperature_bundle);
    update_pinout_state(context->node, &context->bundles.pinout_state_bundle);
    proton_bundle_send(context->node, PROTON_HEARTBEAT_ID);
    msleep(100);
  }

  return NULL;
}

void * stats(void *arg) {
  if (arg == NULL)
  {
    return NULL;
  }

  uint32_t i = 0;
  context_t * context = (context_t *)arg;

  while (1) {
    printf("\033[2J\033[1;1H");
    printf("--------- A300 MCU C --------\r\n");
    printf("Node: %u\r\n", context->node->state);
    printf("Peer: %s\r\n", PROTON__NODE__PC__NAME);
    printf("  State: %u, Transport: %u\r\n", context->node->peers[PROTON__PEER__PC].state, context->node->peers[PROTON__PEER__PC].transport.state);
    printf("Rx: %.3lf KB/s Tx: %.3lf KB/s\r\n", rx / 1000, tx / 1000);
    printf("--- Received Bundles (hz) ---\r\n");
    printf("cmd_fans: %d\r\n", context->cb_counts[CALLBACK_CMD_FANS]);
    printf("display_status: %d\r\n", context->cb_counts[CALLBACK_DISPLAY_STATUS]);
    printf("cmd_lights: %d\r\n", context->cb_counts[CALLBACK_CMD_LIGHTS]);
    printf("battery: %d\r\n", context->cb_counts[CALLBACK_BATTERY]);
    printf("pinout_command: %d\r\n", context->cb_counts[CALLBACK_PINOUT_COMMAND]);
    printf("cmd_shutdown: %d\r\n", context->cb_counts[CALLBACK_CMD_SHUTDOWN]);
    printf("clear_needs_reset: %d\r\n", context->cb_counts[CALLBACK_CLEAR_NEEDS_RESET]);
    printf("-----------------------------\r\n");

    rx = 0.0;
    tx = 0.0;

    for (uint8_t i = 0; i < CALLBACK_COUNT; i++)
    {
      context->cb_counts[i] = 0;
    }

    msleep(1000);
  }

  return NULL;
}

int main() {
  printf("~~~~~~~ A300 node ~~~~~~~\r\n");
  pthread_t thread_10hz, thread_1hz, thread_stats;

  // Node
  proton_node_t mcu_node;

  // Peers
  proton_peer_t mcu_peers[PROTON__PEER__COUNT];

  // Buffers
  uint8_t mcu_buffer[PROTON_MAX_MESSAGE_SIZE];
  uint8_t pc_buffer[PROTON_MAX_MESSAGE_SIZE];
  proton_buffer_t proton_mcu_buffer = {mcu_buffer, sizeof(mcu_buffer)};
  proton_buffer_t proton_pc_buffer = {pc_buffer, sizeof(pc_buffer)};

  // Context
  context_t context = {
    .node = &mcu_node,
    .bundles = PROTON__BUNDLES__MCU__DEFAULT_VALUE,
    .last_pc_heartbeat = 0
  };

  // Init
  printf("Bundles init %d\r\n", proton_bundles_mcu_init(&context.bundles));

  pthread_mutex_init(&context.mcu_lock, NULL);
  pthread_mutex_init(&context.pc_lock, NULL);

  printf("Peer init %d\r\n", proton_peer_pc_init(&mcu_peers[PROTON__PEER__PC], proton_pc_buffer));

  printf("Node init %d\r\n", proton_node_mcu_init(&mcu_node, mcu_peers, proton_mcu_buffer, &context));

  strcpy(context.bundles.status_bundle.firmware_version, "3.0.0");
  strcpy(context.bundles.status_bundle.hardware_id, "A300");
  context.bundles.stop_status_bundle.needs_reset = true;

  // Start threads
  pthread_create(&thread_10hz, NULL, &timer_10hz, &context);
  pthread_create(&thread_1hz, NULL, &timer_1hz, &context);
  pthread_create(&thread_stats, NULL, &stats, &context);

  // Spin
  proton_spin(&mcu_node, PROTON__PEER__PC);

  pthread_join(thread_10hz, NULL);
  pthread_join(thread_1hz, NULL);
  pthread_join(thread_stats, NULL);

  return 0;
}
