/**
 * @file a300_pc.c
 * @brief PC-side communication handler for A300 robot system.
 *
 * This module implements the PC node for the Proton communication framework,
 * handling bidirectional communication with the MCU via UDP sockets. It manages
 * periodic bundle transmissions, receives telemetry data, and maintains system
 * statistics and status monitoring.
 *
 * @details
 * The implementation uses:
 * - UDP sockets for MCU transport communication
 * - POSIX threads for concurrent timer operations (1Hz, 20Hz)
 * - Pthread mutexes for thread-safe access to shared resources
 * - Callback functions for handling received bundles
 *
 * Key responsibilities:
 * - Initialize and manage Proton node and peer structures
 * - Establish socket connections for MCU communication
 * - Implement transport layer read/write operations
 * - Execute periodic tasks via dedicated threads:
 *   * 1Hz: Fan, display, battery, pinout updates and heartbeat
 *   * 20Hz: Light command updates
 *   * Stats: Display real-time system metrics
 * - Track bundle reception counters and log messages
 * - Monitor MCU heartbeat for connection health
 */
#include "proton__a300_pc.h"
#include "utils.h"

double rx, tx;
int sock_send, sock_recv;

#define MAX_LOGS 100

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

typedef struct {
  proton_node_t *node;
  proton_bundles_pc_t bundles;
  pthread_mutex_t mcu_lock;
  pthread_mutex_t pc_lock;
  uint32_t last_mcu_heartbeat;
  uint32_t cb_counts[CALLBACK_COUNT];
  char logs[MAX_LOGS][PROTON__BUNDLE__LOG__SIGNAL__MSG__CAPACITY];
  uint8_t log_index;
} context_t;

/**
 * @brief Callback for log bundle reception
 * @param context Pointer to context_t structure
 *
 * Stores received log messages and increments callback counter.
 */
void proton_bundle_log_callback(void *context) {
  context_t *c = (context_t *)context;
  c->cb_counts[CALLBACK_LOG]++;
  strncpy(c->logs[c->log_index++], c->bundles.log_bundle.msg,
          PROTON__BUNDLE__LOG__SIGNAL__MSG__CAPACITY);
  memset(c->bundles.log_bundle.msg, '\0', PROTON__BUNDLE__LOG__SIGNAL__MSG__CAPACITY);
}

/**
 * @brief Callback for status bundle reception
 * @param context Pointer to context_t structure
 *
 * Increments the status callback counter.
 */
void proton_bundle_status_callback(void *context) {
  context_t *c = (context_t *)context;
  c->cb_counts[CALLBACK_STATUS]++;
}

/**
 * @brief Callback for power bundle reception
 * @param context Pointer to context_t structure
 *
 * Increments the power callback counter.
 */
void proton_bundle_power_callback(void *context) {
  context_t *c = (context_t *)context;
  c->cb_counts[CALLBACK_POWER]++;
}

/**
 * @brief Callback for emergency stop bundle reception
 * @param context Pointer to context_t structure
 *
 * Increments the emergency stop callback counter.
 */
void proton_bundle_emergency_stop_callback(void *context) {
  context_t *c = (context_t *)context;
  c->cb_counts[CALLBACK_ESTOP]++;
}

/**
 * @brief Callback for temperature bundle reception
 * @param context Pointer to context_t structure
 *
 * Increments the temperature callback counter.
 */
void proton_bundle_temperature_callback(void *context) {
  context_t *c = (context_t *)context;
  c->cb_counts[CALLBACK_TEMPERATURE]++;
}

/**
 * @brief Callback for stop status bundle reception
 * @param context Pointer to context_t structure
 *
 * Increments the stop status callback counter.
 */
void proton_bundle_stop_status_callback(void *context) {
  context_t *c = (context_t *)context;
  c->cb_counts[CALLBACK_STOP_STATUS]++;
}

/**
 * @brief Callback for pinout state bundle reception
 * @param context Pointer to context_t structure
 *
 * Increments the pinout state callback counter.
 */
void proton_bundle_pinout_state_callback(void *context) {
  context_t *c = (context_t *)context;
  c->cb_counts[CALLBACK_PINOUT_STATE]++;
}

/**
 * @brief Callback for alerts bundle reception
 * @param context Pointer to context_t structure
 *
 * Increments the alerts callback counter.
 */
void proton_bundle_alerts_callback(void *context) {
  context_t *c = (context_t *)context;
  c->cb_counts[CALLBACK_ALERTS]++;
}

/**
 * @brief Callback for MCU heartbeat bundle reception
 * @param context Pointer to context_t structure
 *
 * Updates MCU peer state to active and records heartbeat timestamp.
 */
void proton_bundle_mcu_heartbeat_callback(void *context) {
  context_t *c = (context_t *)context;
  printf("Heartbeat received %u\r\n", c->bundles.mcu_heartbeat_bundle.heartbeat);
  c->node->peers[PROTON__PEER__MCU].state = PROTON_NODE_ACTIVE;
  c->last_mcu_heartbeat = time(NULL);
}

/**
 * @brief Updates and sends command lights bundle
 * @param node Pointer to proton node
 * @param cmd_lights_bundle Pointer to command lights bundle
 *
 * Populates lights array with random values and sends to MCU.
 */
void update_lights(proton_node_t * node, proton_bundle_cmd_lights_t * cmd_lights_bundle) {
  for (uint8_t i = 0; i < PROTON__BUNDLE__CMD_LIGHTS__SIGNAL__LIGHTS__LENGTH;
       i++) {
    for (uint8_t j = 0;
         j < PROTON__BUNDLE__CMD_LIGHTS__SIGNAL__LIGHTS__CAPACITY; j++) {
      cmd_lights_bundle->lights[i][j] = rand_uint8();
    }
  }

  proton_bundle_send(node, PROTON__BUNDLE__CMD_LIGHTS);
}

/**
 * @brief Updates and sends command fans bundle
 * @param node Pointer to proton node
 * @param cmd_fans_bundle Pointer to command fans bundle
 *
 * Populates fans array with random values and sends to MCU.
 */
void update_fans(proton_node_t * node, proton_bundle_cmd_fans_t * cmd_fans_bundle) {
  for (uint8_t i = 0; i < PROTON__BUNDLE__CMD_FANS__SIGNAL__FANS__CAPACITY;
       i++) {
    cmd_fans_bundle->fans[i] = rand_uint8();
  }

  proton_bundle_send(node, PROTON__BUNDLE__CMD_FANS);
}

/**
 * @brief Updates and sends display status bundle
 * @param node Pointer to proton node
 * @param display_status_bundle Pointer to display status bundle
 *
 * Sets test strings for display and sends to MCU.
 */
void update_display_status(proton_node_t * node, proton_bundle_display_status_t * display_status_bundle) {
  strncpy(display_status_bundle->string1, "TEST_STRING",
          PROTON__BUNDLE__DISPLAY_STATUS__SIGNAL__STRING1__CAPACITY);
  strncpy(display_status_bundle->string2, "TEST_STRING2",
          PROTON__BUNDLE__DISPLAY_STATUS__SIGNAL__STRING2__CAPACITY);

  proton_bundle_send(node, PROTON__BUNDLE__DISPLAY_STATUS);
}

/**
 * @brief Updates and sends battery bundle
 * @param node Pointer to proton node
 * @param battery_bundle Pointer to battery bundle
 *
 * Sets random battery percentage and sends to MCU.
 */
void update_battery(proton_node_t * node, proton_bundle_battery_t * battery_bundle) {
  battery_bundle->percentage = rand_float();
  proton_bundle_send(node, PROTON__BUNDLE__BATTERY);
}

/**
 * @brief Updates and sends pinout command bundle
 * @param node Pointer to proton node
 * @param pinout_command_bundle Pointer to pinout command bundle
 *
 * Populates rails and outputs arrays with random values and sends to MCU.
 */
void update_pinout_command(proton_node_t * node, proton_bundle_pinout_command_t * pinout_command_bundle) {
  for (uint8_t i = 0; i < PROTON__BUNDLE__PINOUT_COMMAND__SIGNAL__RAILS__LENGTH;
       i++) {
    pinout_command_bundle->rails[i] = rand_bool();
  }
  for (uint8_t i = 0;
       i < PROTON__BUNDLE__PINOUT_COMMAND__SIGNAL__OUTPUTS__LENGTH; i++) {
    pinout_command_bundle->outputs[i] = rand_uint32();
  }

  proton_bundle_send(node, PROTON__BUNDLE__PINOUT_COMMAND);
}

/**
 * @brief Establishes UDP transport connection with MCU
 * @return PROTON_OK if connection successful, error code otherwise.
 *
 * Initializes UDP sockets for bidirectional communication with MCU node.
 */
proton_status_e proton_node_mcu_transport_connect() {
  sock_recv = socket_init(PROTON__NODE__PC__ENDPOINT__0__IPHL,
                          PROTON__NODE__PC__ENDPOINT__0__PORT, true);
  sock_send = socket_init(PROTON__NODE__MCU__ENDPOINT__0__IPHL,
                          PROTON__NODE__MCU__ENDPOINT__0__PORT, false);

  return (sock_recv >= 0 && sock_send >= 0) ? PROTON_OK : PROTON_CONNECT_ERROR;
}

/**
 * @brief Disconnects MCU transport
 * @return PROTON_OK if disconnection successful, error code otherwise.
 */
proton_status_e proton_node_mcu_transport_disconnect() {
  return PROTON_OK;
}

/**
 * @brief Reads data from MCU transport
 * @param buf Buffer to store received data
 * @param len Maximum bytes to read
 * @return Number of bytes read, 0 on error
 *
 * Receives data from MCU socket and updates rx statistics.
 */
proton_status_e proton_node_mcu_transport_read(uint8_t *buf, size_t len, size_t * bytes_read) {
  int ret = recv(sock_recv, buf, len, 0);

  if (ret < 0) {
    return PROTON_READ_ERROR;
  }

  rx += ret;
  *bytes_read = ret;

  return PROTON_OK;
}

/**
 * @brief Writes data to MCU transport
 * @param buf Buffer containing data to send
 * @param len Number of bytes to write
 * @param bytes_written Pointer to store number of bytes actually written
 * @return PROTON_OK if write successful, error code otherwise.
 *
 * Sends data to MCU socket and updates tx statistics.
 */
proton_status_e proton_node_mcu_transport_write(const uint8_t *buf, size_t len, size_t * bytes_written) {
  int ret = send(sock_send, buf, len, 0);

  if (ret < 0) {
    return PROTON_WRITE_ERROR;
  }

  tx += ret;
  *bytes_written = ret;

  return PROTON_OK;
}

/**
 * @brief Locks the MCU mutex.
 * @param context Pointer to context_t structure.
 * @return True if lock successful, false otherwise.
 */
proton_status_e proton_node_mcu_lock(void * context) {
  if (context == NULL)
  {
    return PROTON_NULL_PTR_ERROR;
  }

  context_t * c = (context_t *)context;
  return pthread_mutex_lock(&c->mcu_lock) == 0 ? PROTON_OK : PROTON_MUTEX_ERROR;
}

/**
 * @brief Unlocks the MCU mutex.
 * @param context Pointer to context_t structure.
 * @return True if unlock successful, false otherwise.
 */
proton_status_e proton_node_mcu_unlock(void * context) {
  if (context == NULL)
  {
    return PROTON_NULL_PTR_ERROR;
  }

  context_t * c = (context_t *)context;
  return pthread_mutex_unlock(&c->mcu_lock) == 0 ? PROTON_OK : PROTON_MUTEX_ERROR;
}

/**
 * @brief Locks the PC mutex.
 * @param context Pointer to context_t structure.
 * @return True if lock successful, false otherwise.
 */
proton_status_e proton_node_pc_lock(void * context) {
  if (context == NULL)
  {
    return PROTON_NULL_PTR_ERROR;
  }

  context_t * c = (context_t *)context;
  return pthread_mutex_lock(&c->pc_lock) == 0 ? PROTON_OK : PROTON_MUTEX_ERROR;
}

/**
 * @brief Unlocks the PC mutex.
 * @param context Pointer to context_t structure.
 * @return True if unlock successful, false otherwise.
 */
proton_status_e proton_node_pc_unlock(void * context) {
  if (context == NULL)
  {
    return PROTON_NULL_PTR_ERROR;
  }

  context_t * c = (context_t *)context;
  return pthread_mutex_unlock(&c->pc_lock) == 0 ? PROTON_OK : PROTON_MUTEX_ERROR;
}

/**
 * @brief Thread function running at 1 Hz
 * @param arg Pointer to context_t structure
 * @return NULL (runs indefinitely)
 *
 * Sends periodic updates for fans, display, battery, pinout, and heartbeat.
 * Monitors MCU heartbeat timeout and updates peer state.
 */
void *timer_1hz(void *arg) {
  context_t * context = (context_t *)arg;

  uint32_t i = 0;
  while (1) {
    update_fans(context->node, &context->bundles.cmd_fans_bundle);
    update_display_status(context->node, &context->bundles.display_status_bundle);
    update_battery(context->node, &context->bundles.battery_bundle);
    update_pinout_command(context->node, &context->bundles.pinout_command_bundle);

    context->bundles.pc_heartbeat_bundle.heartbeat++;
    proton_bundle_send(context->node, PROTON_HEARTBEAT_ID);
    msleep(1000);

    if (time(NULL) - context->last_mcu_heartbeat >
        PROTON__NODE__MCU__HEARTBEAT__PERIOD / 1000) {
      context->node->peers[PROTON__PEER__MCU].state = PROTON_NODE_INACTIVE;
    }
  }

  return NULL;
}

/**
 * @brief Thread function running at 20 Hz
 * @param arg Pointer to context_t structure
 * @return NULL (runs indefinitely)
 *
 * Sends periodic updates for lights command.
 */
void *timer_20hz(void *arg) {
  context_t * context = (context_t *)arg;
  uint32_t i = 0;
  while (1) {
    update_lights(context->node, &context->bundles.cmd_lights_bundle);
    msleep(50);
  }

  return NULL;
}

/**
 * @brief Thread function displaying statistics and logs
 * @param arg Pointer to context_t structure
 * @return NULL (runs indefinitely)
 *
 * Prints node status, peer info, network stats, bundle reception rates, and logs.
 * Resets counters after each display cycle.
 */
void *stats(void *arg) {
  context_t * context = (context_t *)arg;
  uint32_t i = 0;

  while (1) {
    printf("\033[2J\033[1;1H");
    printf("--------- A300 PC C ---------\r\n");
    printf("Node: %u\r\n", context->node->state);
    printf("Peer: %s\r\n", PROTON__NODE__MCU__NAME);
    printf("  State: %u, Transport: %u\r\n",
           context->node->peers[PROTON__PEER__MCU].state,
           context->node->peers[PROTON__PEER__MCU].transport.state);
    printf("Rx: %.3lf KB/s Tx: %.3lf KB/s\r\n", rx / 1000, tx / 1000);
    printf("--- Received Bundles (hz) ---\r\n");
    printf("log: %d\r\n", context->cb_counts[CALLBACK_LOG]);
    printf("status: %d\r\n", context->cb_counts[CALLBACK_STATUS]);
    printf("power: %d\r\n", context->cb_counts[CALLBACK_POWER]);
    printf("emergency_stop: %d\r\n", context->cb_counts[CALLBACK_ESTOP]);
    printf("temperature: %d\r\n", context->cb_counts[CALLBACK_TEMPERATURE]);
    printf("stop_status: %d\r\n", context->cb_counts[CALLBACK_STOP_STATUS]);
    printf("pinout_state: %d\r\n", context->cb_counts[CALLBACK_PINOUT_STATE]);
    printf("alerts: %d\r\n", context->cb_counts[CALLBACK_ALERTS]);
    printf("----------- Logs ------------\r\n");

    for (uint8_t i = 0; i < context->log_index; i++) {
      printf("%s\r\n", context->logs[i]);
    }

    printf("-----------------------------\r\n");

    memset(context->logs, '\0', sizeof(context->logs));
    context->log_index = 0;

    rx = 0.0;
    tx = 0.0;

    for (uint8_t i = 0; i < CALLBACK_COUNT; i++) {
      context->cb_counts[i] = 0;
    }

    msleep(1000);
  }

  return NULL;
}

/**
 * @brief Main entry point for A300 PC application
 * @return 0 on successful execution
 *
 * Initializes proton node, peers, buffers, and context.
 * Creates worker threads for 1Hz, 20Hz timers and statistics display.
 * Spins main thread processing MCU messages.
 */
int main() {
  pthread_t thread_20hz, thread_1hz, thread_stats;

  // Node
  proton_node_t pc_node = PROTON__NODE__PC__DEFAULT_VALUE;

  // Peers
  proton_peer_t pc_peers[PROTON__PEER__COUNT] = {PROTON__NODE__MCU__PEER__DEFAULT_VALUE};

  // Buffers
  uint8_t mcu_buffer[PROTON_MAX_MESSAGE_SIZE];
  uint8_t pc_buffer[PROTON_MAX_MESSAGE_SIZE];
  proton_buffer_t proton_mcu_buffer = {mcu_buffer, sizeof(mcu_buffer)};
  proton_buffer_t proton_pc_buffer = {pc_buffer, sizeof(pc_buffer)};

  // Context
  context_t context = {.node = &pc_node,
                       .bundles = PROTON__BUNDLES__PC__DEFAULT_VALUE,
                       .last_mcu_heartbeat = 0};

  // Init
  proton_bundles_pc_init(&context.bundles);

  pthread_mutex_init(&context.mcu_lock, NULL);
  pthread_mutex_init(&context.pc_lock, NULL);

  proton_peer_mcu_init(&pc_peers[PROTON__PEER__MCU], proton_mcu_buffer);
  proton_node_pc_init(&pc_node, pc_peers, proton_pc_buffer, &context);

  // Start threads
  pthread_create(&thread_20hz, NULL, &timer_20hz, &context);
  pthread_create(&thread_1hz, NULL, &timer_1hz, &context);
  pthread_create(&thread_stats, NULL, &stats, &context);

  // Spin
  proton_spin(&pc_node, PROTON__PEER__MCU);

  pthread_join(thread_20hz, NULL);
  pthread_join(thread_1hz, NULL);
  pthread_join(thread_stats, NULL);

  return 0;
}
