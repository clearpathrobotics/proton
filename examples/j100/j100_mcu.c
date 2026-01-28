/*
 * Copyright 2026 Rockwell Automation Technologies, Inc., All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @author Roni Kreinin (roni.kreinin@rockwellautomation.com)
 */

/**
 * @file j100_mcu.c
 * @brief J100 MCU application implementing Proton node communication.
 *
 * This module simulates an MCU node for the J100 that communicates with a PC
 * via the Proton protocol. It handles multiple bundle callbacks for commands and status
 * updates, manages threading for periodic tasks (1Hz, 10Hz, and 50Hz timers), and maintains
 * transport communication with PC peer.
 *
 * Key features:
 * - Callback handlers for various bundle types (WiFi connected, HMI, motor command, PC heartbeat)
 * - Periodic update functions for power, temperature, status, emergency stop, IMU, magnetometer, NMEA, and motor feedback data
 * - Serial transport layer for PC communication
 * - Thread-safe operations using mutexes for MCU and PC resources
 * - Real-time monitoring and statistics collection
 * - PC heartbeat monitoring with automatic peer state management
 *
 * The application spawns four worker threads:
 * - 1Hz timer: Updates status, emergency stop, and stop status at 1Hz frequency
 * - 10Hz timer: Updates power, temperature, and MCU heartbeat at 10Hz frequency
 * - 50Hz timer: Updates IMU, magnetometer, NMEA, and motor feedback at 50Hz frequency
 * - Stats thread: Displays real-time statistics including throughput and callback counts
 *
 */

#include "utils.h"
#include "proton__j100_mcu.h"
#include "stdarg.h"

/**
 * @brief Enumeration of callback types for bundle reception.
 *
 */
typedef enum {
  CALLBACK_WIFI_CONNECTED,
  CALLBACK_HMI,
  CALLBACK_MOTOR_COMMAND,
  CALLBACK_COUNT
} callback_e;

/**
 * @brief Context structure for MCU node operations.
 *
 */
typedef struct {
  proton_node_t * node;
  proton_bundles_mcu_t bundles;
  pthread_mutex_t mcu_lock;
  pthread_mutex_t pc_lock;
  uint32_t last_pc_heartbeat;
  uint32_t cb_counts[CALLBACK_COUNT];
  int serial_port;
  double rx;
  double tx;
} context_t;

/**
 * @brief Callback for WiFi connected bundle reception.
 * @param context Pointer to context_t structure.
 */
void proton_bundle_wifi_connected_callback(void * context) {
  context_t * c = (context_t *)context;
  c->cb_counts[CALLBACK_WIFI_CONNECTED]++;
}

/**
 * @brief Callback for HMI bundle reception.
 * @param context Pointer to context_t structure.
 */
void proton_bundle_hmi_callback(void * context) {
  context_t * c = (context_t *)context;
  c->cb_counts[CALLBACK_HMI]++;
}

/**
 * @brief Callback for motor command bundle reception.
 * @param context Pointer to context_t structure.
 */
void proton_bundle_motor_command_callback(void * context) {
  context_t * c = (context_t *)context;
  c->cb_counts[CALLBACK_MOTOR_COMMAND]++;
  c->bundles.motor_feedback_bundle.actual_mode = c->bundles.motor_command_bundle.mode;
  c->bundles.motor_feedback_bundle.commanded_mode = c->bundles.motor_command_bundle.mode;
  c->bundles.motor_feedback_bundle.drivers_measured_velocity[0] = c->bundles.motor_command_bundle.drivers[0];
  c->bundles.motor_feedback_bundle.drivers_measured_velocity[1] = c->bundles.motor_command_bundle.drivers[1];
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
 * @brief Sends a log message bundle.
 * @param context Pointer to context_t structure.
 * @param file Source file name.
 * @param func Function name.
 * @param line Line number.
 * @param level Log level.
 * @param msg Log message format string.
 * @param ... Additional arguments for format string.
 */
void send_log(void * context, const char *file, const char* func, int line, uint8_t level, char *msg, ...) {
  context_t * c = (context_t *)context;
  strcpy(c->bundles.log_bundle.name, "J100_proton");
  strcpy(c->bundles.log_bundle.file, file);
  strcpy(c->bundles.log_bundle.function, func);
  c->bundles.log_bundle.line = line;
  c->bundles.log_bundle.level = level;
  va_list args;
  va_start(args, msg);
  vsnprintf(c->bundles.log_bundle.msg, PROTON__BUNDLE__LOG__SIGNAL__MSG__CAPACITY, msg, args);
  va_end(args);

  proton_bundle_send(c->node, PROTON__BUNDLE__LOG);
}

/**
 * @brief Updates and sends power bundle data.
 * @param context Pointer to context_t structure.
 */
void update_power(void * context) {
  context_t * c = (context_t *)context;
  for (uint8_t i = 0; i < PROTON__BUNDLE__POWER__SIGNAL__MEASURED_VOLTAGES__LENGTH;
       i++) {
    c->bundles.power_bundle.measured_currents[i] = rand_float();
    c->bundles.power_bundle.measured_voltages[i] = rand_float();
  }

  proton_bundle_send(c->node, PROTON__BUNDLE__POWER);
}

/**
 * @brief Updates and sends temperature bundle data.
 * @param context Pointer to context_t structure.
 */
void update_temperature(void * context) {
  context_t * c = (context_t *)context;
  for (uint8_t i = 0; i < PROTON__BUNDLE__TEMPERATURE__SIGNAL__TEMPERATURES__LENGTH;
       i++) {
    c->bundles.temperature_bundle.temperatures[i] = rand_float();
  }

  proton_bundle_send(c->node, PROTON__BUNDLE__TEMPERATURE);
}

/**
 * @brief Updates and sends status bundle data.
 * @param context Pointer to context_t structure.
 * @param s Uptime in seconds.
 */
void update_status(void * context, uint32_t s) {
  context_t * c = (context_t *)context;

  c->bundles.status_bundle.connection_uptime_sec = s;
  c->bundles.status_bundle.connection_uptime_nanosec = rand_uint32();
  c->bundles.status_bundle.mcu_uptime_sec = s;
  c->bundles.status_bundle.mcu_uptime_nanosec = rand_uint32();

  proton_bundle_send(c->node, PROTON__BUNDLE__STATUS);
}

/**
 * @brief Updates and sends emergency stop bundle data.
 * @param context Pointer to context_t structure.
 */
void update_emergency_stop(void * context) {
  context_t * c = (context_t *)context;
  c->bundles.emergency_stop_bundle.data = !c->bundles.emergency_stop_bundle.data;

  proton_bundle_send(c->node, PROTON__BUNDLE__EMERGENCY_STOP);
}

/**
 * @brief Updates and sends stop status bundle data.
 * @param context Pointer to context_t structure.
 */
void update_stop_status(void * context) {
  context_t * c = (context_t *)context;
  c->bundles.stop_status_bundle.external_stop_present = !c->bundles.stop_status_bundle.external_stop_present;

  proton_bundle_send(c->node, PROTON__BUNDLE__STOP_STATUS);
}

/**
 * @brief Updates and sends IMU bundle data.
 * @param context Pointer to context_t structure.
 */
void update_imu(void * context) {
  context_t * c = (context_t *)context;

  c->bundles.imu_bundle.linear_acceleration_x = rand_double();
  c->bundles.imu_bundle.linear_acceleration_y = rand_double();
  c->bundles.imu_bundle.linear_acceleration_z = rand_double();
  c->bundles.imu_bundle.angular_velocity_x = rand_double();
  c->bundles.imu_bundle.angular_velocity_y = rand_double();
  c->bundles.imu_bundle.angular_velocity_z = rand_double();

  proton_bundle_send(c->node, PROTON__BUNDLE__IMU);
}

/**
 * @brief Updates and sends magnetometer bundle data.
 * @param context Pointer to context_t structure.
 */
void update_mag(void * context) {
  context_t * c = (context_t *)context;

  c->bundles.magnetometer_bundle.magnetic_field_x = rand_double();
  c->bundles.magnetometer_bundle.magnetic_field_y = rand_double();
  c->bundles.magnetometer_bundle.magnetic_field_z = rand_double();

  proton_bundle_send(c->node, PROTON__BUNDLE__MAGNETOMETER);
}

/**
 * @brief Updates and sends NMEA bundle data.
 * @param context Pointer to context_t structure.
 */
void update_nmea(void * context) {
  context_t * c = (context_t *)context;

  char tmp[PROTON__BUNDLE__NMEA__SIGNAL__SENTENCE__CAPACITY] = {0};

  uint16_t i;

  for (i = 0; i < (rand() % (PROTON__BUNDLE__NMEA__SIGNAL__SENTENCE__CAPACITY - 1)); i++)
  {
    tmp[i] = rand_char();
  }

  tmp[i+1] = '\0';
  strncpy(c->bundles.nmea_bundle.sentence, tmp, PROTON__BUNDLE__NMEA__SIGNAL__SENTENCE__CAPACITY);

  proton_bundle_send(c->node, PROTON__BUNDLE__NMEA);
}

/**
 * @brief Updates and sends motor feedback bundle data.
 * @param context Pointer to context_t structure.
 */
void update_motor_feedback(void * context) {
  context_t * c = (context_t *)context;

  for (uint8_t i = 0; i < 2; i++)
  {
    c->bundles.motor_feedback_bundle.drivers_current[i] = rand_float();
    c->bundles.motor_feedback_bundle.drivers_bridge_temperature[i] = rand_float();
    c->bundles.motor_feedback_bundle.drivers_motor_temperature[i] = rand_float();
    c->bundles.motor_feedback_bundle.drivers_driver_fault[i] = rand_bool();
    c->bundles.motor_feedback_bundle.drivers_duty_cycle[i] = rand_float();
    c->bundles.motor_feedback_bundle.drivers_measured_travel[i] = rand_float();
  }

  proton_bundle_send(c->node, PROTON__BUNDLE__MOTOR_FEEDBACK);
}

/**
 * @brief Connects PC transport
 * @return PROTON_OK if connection successful, error code otherwise.
 */
proton_status_e proton_node_pc_transport_connect(void * context) {
  context_t * c = (context_t *)context;
  c->serial_port = serial_init(PROTON__NODE__PC__ENDPOINT__0__DEVICE);
  return c->serial_port >= 0 ? PROTON_OK : PROTON_CONNECT_ERROR;
}

/**
 * @brief Disconnects PC transport
 * @return PROTON_OK if disconnection successful, error code otherwise.
 */
proton_status_e proton_node_pc_transport_disconnect(void * context) {
  (void)context;
  return PROTON_OK;
}

/**
 * @brief Reads data from PC transport.
 * @param context Pointer to context_t structure.
 * @param buf Buffer to store read data.
 * @param len Number of bytes to read.
 * @param bytes_read Pointer to store number of bytes actually read.
 * @return PROTON_OK if read successful, error code otherwise.
 */
proton_status_e proton_node_pc_transport_read(void * context, uint8_t *buf, size_t len, size_t * bytes_read) {
  context_t * c = (context_t *)context;
  size_t bytes = serial_read(c->serial_port, buf, len);

  if (bytes_read > 0)
  {
    c->rx += bytes + PROTON_FRAME_OVERHEAD;
  }
  else
  {
    return PROTON_READ_ERROR;
  }

  *bytes_read = bytes;

  return PROTON_OK;
}

/**
 * @brief Writes data to PC transport.
 * @param context Pointer to context_t structure.
 * @param buf Buffer containing data to write.
 * @param len Number of bytes to write.
 * @param bytes_written Pointer to store number of bytes actually written.
 * @return PROTON_OK if write successful, error code otherwise.
 */
proton_status_e proton_node_pc_transport_write(void * context, const uint8_t *buf, size_t len, size_t * bytes_written) {
  context_t * c = (context_t *)context;
  size_t bytes = serial_write(c->serial_port, buf, len);

  if (bytes > 0)
  {
    c->tx += bytes + PROTON_FRAME_OVERHEAD;
  }
  else
  {
    return PROTON_WRITE_ERROR;
  }

  *bytes_written = bytes;

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
 * @brief 1 Hz timer thread routine.
 * @param arg Thread argument (pointer to context_t structure).
 * @return NULL.
 */
void *timer_1hz(void *arg) {
  if (arg == NULL)
  {
    return NULL;
  }

  context_t * context = (context_t *)arg;
  uint32_t i = 0;
  while (1) {
    LOG_INFO(context, "1hz timer %ld", i++);
    update_status(context, i);
    update_emergency_stop(context);
    update_stop_status(context);
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

  context_t * context = (context_t *)arg;
  uint32_t i = 0;

  while (1) {
    LOG_INFO(context, "10hz timer %ld", i++);
    update_power(context);
    update_temperature(context);

    context->bundles.mcu_heartbeat_bundle.heartbeat++;
    proton_bundle_send(context->node, PROTON_HEARTBEAT_ID);
    msleep(100);
  }

  return NULL;
}

/**
 * @brief 50 Hz timer thread routine.
 * @param arg Thread argument (pointer to context_t structure).
 * @return NULL.
 */
void *timer_50hz(void *arg) {
  if (arg == NULL)
  {
    return NULL;
  }

  context_t * context = (context_t *)arg;
  uint32_t i = 0;
  while (1) {
    update_imu(context);
    update_mag(context);
    update_nmea(context);
    update_motor_feedback(context);
    msleep(20);
  }

  return NULL;
}

/**
 * @brief Statistics thread routine.
 * @param arg Thread argument (pointer to context_t structure).
 * @return NULL.
 */
void *stats(void *arg) {
  if (arg == NULL)
  {
    return NULL;
  }

  context_t * context = (context_t *)arg;
  uint32_t i = 0;

  while (1) {
    printf("\033[2J\033[1;1H");
    printf("--------- J100 MCU C --------\r\n");
    printf("Node: %u\r\n", context->node->state);
    printf("Peer: %s\r\n", PROTON__NODE__PC__NAME);
    printf("  State: %u, Transport: %u\r\n", context->node->peers[PROTON__PEER__PC].state, context->node->peers[PROTON__PEER__PC].transport.state);
    printf("Rx: %.3lf KB/s Tx: %.3lf KB/s\r\n", context->rx / 1000, context->tx / 1000);
    printf("--- Received Bundles (hz) ---\r\n");
    printf("wifi_connected: %d\r\n", context->cb_counts[CALLBACK_WIFI_CONNECTED]);
    printf("hmi: %d\r\n", context->cb_counts[CALLBACK_HMI]);
    printf("motor_command: %d\r\n", context->cb_counts[CALLBACK_MOTOR_COMMAND]);
    printf("-----------------------------\r\n");

    context->rx = 0.0;
    context->tx = 0.0;

    for (uint8_t i = 0; i < CALLBACK_COUNT; i++) {
      context->cb_counts[i] = 0;
    }

    msleep(1000);
  }
}

/**
 * @brief Main function for J100 MCU application.
 * @return Exit status.
 */
int main() {
  pthread_t thread_50hz, thread_10hz, thread_1hz, thread_stats;

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


  strncpy(context.bundles.status_bundle.firmware_version, "3.0.0", PROTON__BUNDLE__STATUS__SIGNAL__FIRMWARE_VERSION__CAPACITY);
  strncpy(context.bundles.status_bundle.hardware_id, "J100", PROTON__BUNDLE__STATUS__SIGNAL__HARDWARE_ID__CAPACITY);

  // Start threads
  pthread_create(&thread_50hz, NULL, &timer_50hz, &context);
  pthread_create(&thread_10hz, NULL, &timer_10hz, &context);
  pthread_create(&thread_1hz, NULL, &timer_1hz, &context);
  pthread_create(&thread_stats, NULL, &stats, &context);

  // Spin
  proton_spin(&mcu_node, PROTON__PEER__PC);

  pthread_join(thread_50hz, NULL);
  pthread_join(thread_10hz, NULL);
  pthread_join(thread_1hz, NULL);
  pthread_join(thread_stats, NULL);

  return 0;
}
