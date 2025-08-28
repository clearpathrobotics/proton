#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>

#include "proton__a300_mcu.h"

#define PROTON_MAX_MESSAGE_SIZE 1024

uint8_t write_buf_[PROTON_MAX_MESSAGE_SIZE];
uint8_t read_buf_[PROTON_MAX_MESSAGE_SIZE];

proton_buffer_t proton_mcu_read_buffer = {read_buf_, PROTON_MAX_MESSAGE_SIZE};
proton_buffer_t proton_mcu_write_buffer = {write_buf_, PROTON_MAX_MESSAGE_SIZE};

int sock_send, sock_recv;

void send_log(char *file, int line, uint8_t level, char *msg, ...);

#define LOG_DEBUG(message, ...)                                                \
  send_log(__FILE_NAME__, __LINE__, 10U, message, ##__VA_ARGS__)
#define LOG_INFO(message, ...)                                                 \
  send_log(__FILE_NAME__, __LINE__, 20U, message, ##__VA_ARGS__)
#define LOG_WARNING(message, ...)                                              \
  send_log(__FILE_NAME__, __LINE__, 30U, message, ##__VA_ARGS__)
#define LOG_ERROR(message, ...)                                                \
  send_log(__FILE_NAME__, __LINE__, 40U, message, ##__VA_ARGS__)
#define LOG_FATAL(message, ...)                                                \
  send_log(__FILE_NAME__, __LINE__, 50U, message, ##__VA_ARGS__)

int msleep(long msec) {
  struct timespec ts;
  int res;

  if (msec < 0) {
    return -1;
  }

  ts.tv_sec = msec / 1000;
  ts.tv_nsec = (msec % 1000) * 1000000;

  do {
    res = nanosleep(&ts, &ts);
  } while (res);

  return res;
}

int socket_init() {
  struct sockaddr_in servaddr;
  sock_send = socket(AF_INET, SOCK_DGRAM, 0);

  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl((in_addr_t)PROTON_NODE__PC__IP);
  servaddr.sin_port = htons(PROTON_NODE__PC__PORT);

  if (connect(sock_send, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) {
    printf("connect error\r\n");
    return 1;
  }

  printf("Send Socket connected\r\n");

  struct sockaddr_in servaddr2;
  sock_recv = socket(AF_INET, SOCK_DGRAM, 0);

  memset(&servaddr2, 0, sizeof(servaddr2));
  servaddr2.sin_family = AF_INET;
  servaddr2.sin_addr.s_addr = htonl((in_addr_t)PROTON_NODE__MCU__IP);
  servaddr2.sin_port = htons(PROTON_NODE__MCU__PORT);

  // Put the socket in non-blocking mode:
  if (fcntl(sock_recv, F_SETFL, fcntl(sock_recv, F_GETFL) | O_NONBLOCK) < 0) {
    printf("Set non-blocking error\r\n");
    return 2;
  }

  if (bind(sock_recv, (struct sockaddr *)&servaddr2, sizeof(servaddr2)) != 0) {
    printf("bind error\r\n");
    return 3;
  }

  printf("Receive Socket bound\r\n");

  return 0;
}

void PROTON_BUNDLE_CmdFansCallback() {
  printf("Received CmdFans\r\n");
  printf("[");
  for (int i = 0; i < PROTON_SIGNALS__CMD_FANS__FAN_SPEEDS__CAPACITY; i++) {
    printf("%d", cmd_fans_bundle.fan_speeds[i]);
    if (i != PROTON_SIGNALS__CMD_FANS__FAN_SPEEDS__CAPACITY - 1) {
      printf(", ");
    }
  }
  printf("]\r\n");
}

void PROTON_BUNDLE_DisplayStatusCallback() {
  printf("Received DisplayStatus\r\n");
  printf("string_1: %s\r\n", display_status_bundle.string_1);
  printf("string_1: %s\r\n", display_status_bundle.string_2);
}

void PROTON_BUNDLE_CmdLightsCallback() {
  printf("Received CmdLights\r\n");
  printf("front_left_light [%d, %d, %d]\r\n",
         cmd_lights_bundle.front_left_light[0],
         cmd_lights_bundle.front_left_light[1],
         cmd_lights_bundle.front_left_light[2]);
  printf("front_right_light [%d, %d, %d]\r\n",
         cmd_lights_bundle.front_right_light[0],
         cmd_lights_bundle.front_right_light[1],
         cmd_lights_bundle.front_right_light[2]);
  printf("rear_left_light [%d, %d, %d]\r\n",
         cmd_lights_bundle.rear_left_light[0],
         cmd_lights_bundle.rear_left_light[1],
         cmd_lights_bundle.rear_left_light[2]);
  printf("rear_right_light [%d, %d, %d]\r\n",
         cmd_lights_bundle.rear_right_light[0],
         cmd_lights_bundle.rear_right_light[1],
         cmd_lights_bundle.rear_right_light[2]);
  // print_bundle(cmd_lights_bundle.bundle);
}

void PROTON_BUNDLE_BatteryCallback() {
  printf("Received Battery %f\r\n", battery_bundle.percentage);
  // print_bundle(battery_bundle.bundle);
}

void PROTON_BUNDLE_PinoutCommandCallback() {
  printf("Received Pinout\r\n");
  // print_bundle(pinout_command_bundle.bundle);
}

void PROTON_BUNDLE_CmdShutdownCallback() { printf("~~~SHUTTING DOWN~~~\r\n"); }

void PROTON_BUNDLE_ClearNeedsResetCallback() {
  printf("~~~Needs reset cleared~~~\r\n");
  stop_status_bundle.needs_reset = false;
}

void send_log(char *file, int line, uint8_t level, char *msg, ...) {
  strcpy(logger_bundle.name, "A300_proton");
  strcpy(logger_bundle.file, file);
  logger_bundle.line = line;
  logger_bundle.level = level;

  va_list args;
  va_start(args, msg);
  vsprintf(logger_bundle.msg, msg, args);
  va_end(args);

  PROTON_BUNDLE_Send(PROTON_BUNDLE__LOGGER);
}

void update_power() {
  for (uint8_t i = 0; i < PROTON_SIGNALS__POWER__MEASURED_VOLTAGES__LENGTH;
       i++) {
    power_bundle.measured_currents[i] = (float)rand();
    power_bundle.measured_voltages[i] = (float)rand();
  }

  if (!PROTON_BUNDLE_Send(PROTON_BUNDLE__POWER)) {
    // printf("Failed to send power\r\n");
  }
}

void update_temperature() {
  for (uint8_t i = 0; i < PROTON_SIGNALS__TEMPERATURE__TEMPERATURES__LENGTH;
       i++) {
    temperature_bundle.temperatures[i] = (float)rand();
  }

  PROTON_BUNDLE_Send(PROTON_BUNDLE__TEMPERATURE);
}

void update_status(uint32_t ms) {
  status_bundle.connection_uptime_s = ms / 1000;
  status_bundle.connection_uptime_ns = rand();

  status_bundle.mcu_uptime_s = ms / 1000;
  status_bundle.mcu_uptime_ns = rand();

  PROTON_BUNDLE_Send(PROTON_BUNDLE__STATUS);
}

void update_emergency_stop() {
  static bool stopped = false;

  stopped = !stopped;
  emergency_stop_bundle.stopped = stopped;

  PROTON_BUNDLE_Send(PROTON_BUNDLE__EMERGENCY_STOP);
}

void update_stop_status() { PROTON_BUNDLE_Send(PROTON_BUNDLE__STOP_STATUS); }

void update_alerts() {
  strcpy(alerts_bundle.alert_string, "E124,E100");

  PROTON_BUNDLE_Send(PROTON_BUNDLE__ALERTS);
}

void update_pinout_state() {
  pinout_state_bundle.rails[0] = true;

  for (uint8_t i = 0; i < PROTON_SIGNALS__PINOUT_STATE__OUTPUTS__LENGTH; i++) {
    pinout_state_bundle.outputs[i] = i % 2;
  }

  for (uint8_t i = 0; i < PROTON_SIGNALS__PINOUT_STATE__OUTPUT_PERIODS__LENGTH;
       i++) {
    pinout_state_bundle.output_periods[i] = rand();
  }

  PROTON_BUNDLE_Send(PROTON_BUNDLE__PINOUT_STATE);
}

bool PROTON_TRANSPORT__McuConnect() { return socket_init() == 0; }

bool PROTON_TRANSPORT__McuDisconnect() { return true; }

size_t PROTON_TRANSPORT__McuRead(uint8_t *buf, size_t len) {
  int ret = recv(sock_recv, buf, len, 0);

  if (ret < 0) {
    return 0;
  }

  return ret;
}

size_t PROTON_TRANSPORT__McuWrite(const uint8_t *buf, size_t len) {
  int ret = send(sock_send, buf, len, 0);

  if (ret < 0) {
    return 0;
  }

  return ret;
}

pthread_mutex_t lock;
pthread_t thread;

bool PROTON_MUTEX__McuLock() { return pthread_mutex_lock(&lock) == 0; }

bool PROTON_MUTEX__McuUnlock() { return pthread_mutex_unlock(&lock) == 0; }

void *spam_log(void *arg) {
  uint32_t i = 0;
  while (1) {
    LOG_INFO("Test %ld", i++);
    msleep(10);
  }
}

int main() {
  printf("~~~~~~~ A300 node ~~~~~~~\r\n");

  pthread_mutex_init(&lock, NULL);

  PROTON_Init();

  printf("INIT\r\n");

  strcpy(status_bundle.firmware_version, "3.0.0");
  strcpy(status_bundle.hardware_id, "A300");
  stop_status_bundle.needs_reset = true;

  uint32_t i = 0;

  pthread_create(&thread, NULL, &spam_log, NULL);

  while (1) {
    // 1 hz
    if (i % 1000 == 0) {
      update_status(i);
      update_emergency_stop();
      update_stop_status();
      update_alerts();
    }

    // 10hz
    if (i % 100 == 0) {
      update_power();
      update_temperature();
      update_pinout_state();
    }

    // 50hz
    if (i % 20 == 0) {
    }

    PROTON_SpinOnce(&mcu_node);
    msleep(1);
    i++;
  }

  return 0;
}
