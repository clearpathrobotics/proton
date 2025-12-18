/**
 * @file a300_mcu.c
 * @brief A300 MCU application implementing Proton node communication.
 *
 * This module implements an MCU node for the A300 system that communicates with a PC
 * via the Proton protocol. It handles multiple bundle callbacks for commands and status
 * updates, manages threading for periodic tasks (1Hz and 10Hz timers), and maintains
 * transport communication with PC peer.
 *
 * Key features:
 * - Callback handlers for various bundle types (fans, lights, battery, pinout, shutdown)
 * - Periodic update functions for power, temperature, status, and emergency stop data
 * - Socket-based transport layer for PC communication
 * - Thread-safe operations using mutexes for MCU and PC resources
 * - Real-time monitoring and statistics collection
 * - PC heartbeat monitoring with automatic peer state management
 *
 * The application spawns three worker threads:
 * - 1Hz timer: Updates status, emergency stop, and alerts at 1Hz frequency
 * - 10Hz timer: Updates power, temperature, and pinout state at 10Hz frequency
 * - Stats thread: Displays real-time statistics including throughput and callback counts
 *
 */

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

/**
 * @brief Callback for fan command bundle reception.
 * @param context Pointer to context_t structure.
 */
void proton_bundle_cmd_fans_callback(void * context) {
  context_t * c = (context_t *)context;
  c->cb_counts[CALLBACK_CMD_FANS]++;
}

/**
 * @brief Callback for display status bundle reception.
 * @param context Pointer to context_t structure.
 */
void proton_bundle_display_status_callback(void * context) {
  context_t * c = (context_t *)context;
  c->cb_counts[CALLBACK_DISPLAY_STATUS]++;
}

/**
 * @brief Callback for light command bundle reception.
 * @param context Pointer to context_t structure.
 */
void proton_bundle_cmd_lights_callback(void * context) {
  context_t * c = (context_t *)context;
  c->cb_counts[CALLBACK_CMD_LIGHTS]++;
}

/**
 * @brief Callback for battery bundle reception.
 * @param context Pointer to context_t structure.
 */
void proton_bundle_battery_callback(void * context) {
  context_t * c = (context_t *)context;
  c->cb_counts[CALLBACK_BATTERY]++;
}

/**
 * @brief Callback for pinout command bundle reception.
 * @param context Pointer to context_t structure.
 */
void proton_bundle_pinout_command_callback(void * context) {
  context_t * c = (context_t *)context;
  c->cb_counts[CALLBACK_PINOUT_COMMAND]++;
}

/**
 * @brief Callback for PC heartbeat bundle reception.
 * @param context Pointer to context_t structure.
 */
void proton_bundle_pc_heartbeat_callback(void * context)
{
  context_t * c = (context_t *)context;
  printf("Heartbeat received %u\r\n", c->bundles.pc_heartbeat_bundle.heartbeat);
  c->node->peers[PROTON__PEER__PC].state = PROTON_NODE_ACTIVE;
  c->last_pc_heartbeat = time(NULL);
}

/**
 * @brief Callback for shutdown command bundle reception.
 * @param context Pointer to context_t structure.
 */
void proton_bundle_cmd_shutdown_callback(void * context) {
  context_t * c = (context_t *)context;
  c->cb_counts[CALLBACK_CMD_SHUTDOWN]++;
  strcpy(c->bundles.cmd_shutdown_response_bundle.message, "SHUTTING DOWN");
  c->bundles.cmd_shutdown_response_bundle.success = true;

  proton_bundle_send(c->node, PROTON__BUNDLE__CMD_SHUTDOWN_RESPONSE);

  exit(0);
}

/**
 * @brief Callback for clear needs reset bundle reception.
 * @param context Pointer to context_t structure.
 */
void proton_bundle_clear_needs_reset_callback(void * context) {
  context_t * c = (context_t *)context;
  c->cb_counts[CALLBACK_CLEAR_NEEDS_RESET]++;
  c->bundles.stop_status_bundle.needs_reset = false;

  strcpy(c->bundles.clear_needs_reset_response_bundle.message, "Needs Reset Cleared");
  c->bundles.clear_needs_reset_response_bundle.success = true;

  proton_bundle_send(c->node, PROTON__BUNDLE__CLEAR_NEEDS_RESET_RESPONSE);
}

/**
 * @brief Sends a log message bundle.
 * @param context Pointer to context_t structure.
 * @param file Source file name.
 * @param func Function name.
 * @param line Line number.
 * @param level Log level.
 * @param msg Format string for message.
 * @param ... Variable arguments for format string.
 */
void send_log(void * context, const char *file, const char* func, int line, uint8_t level, char *msg, ...) {
  context_t * c = (context_t *)context;
  proton_bundle_log_t * log_bundle = &c->bundles.log_bundle;
  strcpy(log_bundle->name, "A300_proton");
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
 * @brief Updates and sends power bundle data.
 * @param node Pointer to proton_node_t structure.
 * @param power_bundle Pointer to power bundle to update.
 */
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

/**
 * @brief Updates and sends temperature bundle data.
 * @param node Pointer to proton_node_t structure.
 * @param temperature_bundle Pointer to temperature bundle to update.
 */
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

/**
 * @brief Updates and sends status bundle data.
 * @param node Pointer to proton_node_t structure.
 * @param status_bundle Pointer to status bundle to update.
 * @param s Uptime in seconds.
 */
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

/**
 * @brief Updates and sends emergency stop bundle data.
 * @param node Pointer to proton_node_t structure.
 * @param emergency_stop_bundle Pointer to emergency stop bundle to update.
 */
void update_emergency_stop(proton_node_t * node, proton_bundle_emergency_stop_t * emergency_stop_bundle) {
  if (!emergency_stop_bundle)
  {
    return;
  }

  emergency_stop_bundle->data = !emergency_stop_bundle->data;

  proton_bundle_send(node, PROTON__BUNDLE__EMERGENCY_STOP);
}

/**
 * @brief Updates and sends stop status bundle data.
 * @param node Pointer to proton_node_t structure.
 * @param stop_status_bundle Pointer to stop status bundle to update.
 */
void update_stop_status(proton_node_t * node, proton_bundle_stop_status_t * stop_status_bundle) {
  if (!stop_status_bundle)
  {
    return;
  }

  proton_bundle_send(node, PROTON__BUNDLE__STOP_STATUS);
}

/**
 * @brief Updates and sends alerts bundle data.
 * @param node Pointer to proton_node_t structure.
 * @param alerts_bundle Pointer to alerts bundle to update.
 */
void update_alerts(proton_node_t * node, proton_bundle_alerts_t * alerts_bundle) {
  if (!alerts_bundle)
  {
    return;
  }

  strcpy(alerts_bundle->data, "E124,E100");

  proton_bundle_send(node, PROTON__BUNDLE__ALERTS);
}

/**
 * @brief Updates and sends pinout state bundle data.
 * @param node Pointer to proton_node_t structure.
 * @param pinout_state_bundle Pointer to pinout state bundle to update.
 */
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

/**
 * @brief Establishes PC transport connection.
 * @return True if connection successful, false otherwise.
 */
bool proton_node_pc_transport_connect() {
  sock_recv = socket_init(PROTON__NODE__MCU__ENDPOINT__0__IPHL, PROTON__NODE__MCU__ENDPOINT__0__PORT, true);
  sock_send = socket_init(PROTON__NODE__PC__ENDPOINT__0__IPHL, PROTON__NODE__PC__ENDPOINT__0__PORT, false);

  return (sock_recv >= 0 && sock_send >=0);
}

/**
 * @brief Closes PC transport connection.
 * @return True if disconnection successful, false otherwise.
 */
bool proton_node_pc_transport_disconnect() { return true; }

/**
 * @brief Reads data from PC transport.
 * @param buf Buffer to read data into.
 * @param len Maximum number of bytes to read.
 * @return Number of bytes read.
 */
size_t proton_node_pc_transport_read(uint8_t *buf, size_t len) {
  int ret = recv(sock_recv, buf, len, 0);

  if (ret < 0) {
    return 0;
  }

  rx += ret;

  return ret;
}

/**
 * @brief Writes data to PC transport.
 * @param buf Buffer containing data to write.
 * @param len Number of bytes to write.
 * @return Number of bytes written.
 */
size_t proton_node_pc_transport_write(const uint8_t *buf, size_t len) {
  int ret = send(sock_send, buf, len, 0);

  if (ret < 0) {
    return 0;
  }

  tx += ret;

  return ret;
}

/**
 * @brief Locks the MCU mutex.
 * @param context Pointer to context_t structure.
 * @return True if lock successful, false otherwise.
 */
bool proton_node_mcu_lock(void * context) {
  if (context == NULL)
  {
    return false;
  }

  context_t * c = (context_t *)context;
  return pthread_mutex_lock(&c->mcu_lock) == 0;
}

/**
 * @brief Unlocks the MCU mutex.
 * @param context Pointer to context_t structure.
 * @return True if unlock successful, false otherwise.
 */
bool proton_node_mcu_unlock(void * context) {
  if (context == NULL)
  {
    return false;
  }

  context_t * c = (context_t *)context;
  return pthread_mutex_unlock(&c->mcu_lock) == 0;
}

/**
 * @brief Locks the PC mutex.
 * @param context Pointer to context_t structure.
 * @return True if lock successful, false otherwise.
 */
bool proton_node_pc_lock(void * context) {
  if (context == NULL)
  {
    return false;
  }

  context_t * c = (context_t *)context;
  return pthread_mutex_lock(&c->pc_lock) == 0;
}

/**
 * @brief Unlocks the PC mutex.
 * @param context Pointer to context_t structure.
 * @return True if unlock successful, false otherwise.
 */
bool proton_node_pc_unlock(void * context) {
  if (context == NULL)
  {
    return false;
  }

  context_t * c = (context_t *)context;
  return pthread_mutex_unlock(&c->pc_lock) == 0;
}

/**
 * @brief 1 Hz timer thread routine.
 * @param arg Thread argument (pointer to context_t structure).
 * @return NULL.
 */
void *timer_1hz(void *arg) {
  if (arg == NULL)
  {
    return NULL;
  }

  uint32_t i = 0;
  context_t * context = (context_t *)arg;

  while (1) {
    LOG_INFO(context, "1hz timer %ld", i++);
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

/**
 * @brief 10 Hz timer thread routine.
 * @param arg Thread argument (pointer to context_t structure).
 * @return NULL.
 */
void *timer_10hz(void *arg) {
  if (arg == NULL)
  {
    return NULL;
  }

  uint32_t i = 0;
  context_t * context = (context_t *)arg;

  while (1) {
    LOG_INFO(context, "10hz timer %ld", i++);
    update_power(context->node, &context->bundles.power_bundle);
    update_temperature(context->node, &context->bundles.temperature_bundle);
    update_pinout_state(context->node, &context->bundles.pinout_state_bundle);
    proton_bundle_send(context->node, PROTON_HEARTBEAT_ID);
    msleep(100);
  }

  return NULL;
}

/**
 * @brief Statistics display thread routine.
 * @param arg Thread argument (pointer to context_t structure).
 * @return NULL.
 */
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

/**
 * @brief Main entry point for A300 MCU application.
 * @return Exit code.
 */
int main() {
  pthread_t thread_10hz, thread_1hz, thread_stats;

  // Node
  proton_node_t mcu_node = PROTON__NODE__MCU__DEFAULT_VALUE;

  // Peers
  proton_peer_t mcu_peers[PROTON__PEER__COUNT] = {PROTON__NODE__PC__PEER__DEFAULT_VALUE};

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
  proton_bundles_mcu_init(&context.bundles);

  pthread_mutex_init(&context.mcu_lock, NULL);
  pthread_mutex_init(&context.pc_lock, NULL);

  proton_peer_pc_init(&mcu_peers[PROTON__PEER__PC], proton_pc_buffer);
  proton_node_mcu_init(&mcu_node, mcu_peers, proton_mcu_buffer, &context);

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
