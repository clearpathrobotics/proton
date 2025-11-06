#include "utils.h"
#include "proton__a300_mcu.h"
#include <stdlib.h>

#define PROTON_MAX_MESSAGE_SIZE 1024

uint8_t write_buf_[PROTON_MAX_MESSAGE_SIZE];
uint8_t read_buf_[PROTON_MAX_MESSAGE_SIZE];

double rx, tx;

proton_buffer_t proton_mcu_read_buffer = {read_buf_, PROTON_MAX_MESSAGE_SIZE};
proton_buffer_t proton_mcu_write_buffer = {write_buf_, PROTON_MAX_MESSAGE_SIZE};

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

uint32_t cb_counts[CALLBACK_COUNT];


void PROTON_BUNDLE_CmdFansCallback() {
  cb_counts[CALLBACK_CMD_FANS]++;
}

void PROTON_BUNDLE_DisplayStatusCallback() {
  cb_counts[CALLBACK_DISPLAY_STATUS]++;
}

void PROTON_BUNDLE_CmdLightsCallback() {
  cb_counts[CALLBACK_CMD_LIGHTS]++;
}

void PROTON_BUNDLE_BatteryCallback() {
  cb_counts[CALLBACK_BATTERY]++;
}

void PROTON_BUNDLE_PinoutCommandCallback() {
  cb_counts[CALLBACK_PINOUT_COMMAND]++;
}

void PROTON_BUNDLE_PcHeartbeatCallback()
{
  printf("Heartbeat received %u\r\n", pc_heartbeat_bundle.heartbeat);
}

void PROTON_BUNDLE_CmdShutdownCallback() {
  cb_counts[CALLBACK_CMD_SHUTDOWN]++;
  strcpy(cmd_shutdown_response_bundle.message, "SHUTTING DOWN");
  cmd_shutdown_response_bundle.success = true;

  PROTON_BUNDLE_Send(PROTON_BUNDLE__CMD_SHUTDOWN_RESPONSE);

  exit(0);
}

void PROTON_BUNDLE_ClearNeedsResetCallback() {
  cb_counts[CALLBACK_CLEAR_NEEDS_RESET]++;
  stop_status_bundle.needs_reset = false;

  strcpy(clear_needs_reset_response_bundle.message, "Needs Reset Cleared");
  clear_needs_reset_response_bundle.success = true;

  PROTON_BUNDLE_Send(PROTON_BUNDLE__CLEAR_NEEDS_RESET_RESPONSE);
}

void send_log(const char *file, const char* func, int line, uint8_t level, char *msg, ...) {
  strcpy(log_bundle.name, "A300_proton");
  strcpy(log_bundle.file, file);
  strcpy(log_bundle.function, func);
  log_bundle.line = line;
  log_bundle.level = level;

  va_list args;
  va_start(args, msg);
  vsprintf(log_bundle.msg, msg, args);
  va_end(args);

  PROTON_BUNDLE_Send(PROTON_BUNDLE__LOG);
}

void update_power() {
  for (uint8_t i = 0; i < PROTON_SIGNALS__POWER__MEASURED_VOLTAGES__LENGTH;
       i++) {
    power_bundle.measured_currents[i] = rand_float();
    power_bundle.measured_voltages[i] = rand_float();
  }

  PROTON_BUNDLE_Send(PROTON_BUNDLE__POWER);
}

void update_temperature() {
  for (uint8_t i = 0; i < PROTON_SIGNALS__TEMPERATURE__TEMPERATURES__LENGTH;
       i++) {
    temperature_bundle.temperatures[i] = rand_float();
  }

  PROTON_BUNDLE_Send(PROTON_BUNDLE__TEMPERATURE);
}

void update_status(uint32_t s) {
  status_bundle.connection_uptime_sec = s;
  status_bundle.connection_uptime_nanosec = rand_uint32();

  status_bundle.mcu_uptime_sec = s;
  status_bundle.mcu_uptime_nanosec = rand_uint32();

  PROTON_BUNDLE_Send(PROTON_BUNDLE__STATUS);
}

void update_emergency_stop() {
  emergency_stop_bundle.data = !emergency_stop_bundle.data;

  PROTON_BUNDLE_Send(PROTON_BUNDLE__EMERGENCY_STOP);
}

void update_stop_status() { PROTON_BUNDLE_Send(PROTON_BUNDLE__STOP_STATUS); }

void update_alerts() {
  strcpy(alerts_bundle.data, "E124,E100");

  PROTON_BUNDLE_Send(PROTON_BUNDLE__ALERTS);
}

void update_pinout_state() {
  pinout_state_bundle.rails[0] = true;

  for (uint8_t i = 0; i < PROTON_SIGNALS__PINOUT_STATE__OUTPUTS__LENGTH; i++) {
    pinout_state_bundle.outputs[i] = rand_bool();
  }

  for (uint8_t i = 0; i < PROTON_SIGNALS__PINOUT_STATE__OUTPUT_PERIODS__LENGTH;
       i++) {
    pinout_state_bundle.output_periods[i] = rand_uint32();
  }

  PROTON_BUNDLE_Send(PROTON_BUNDLE__PINOUT_STATE);
}

bool PROTON_TRANSPORT__McuConnect() {
  sock_recv = socket_init(PROTON_NODE__MCU__IP, PROTON_NODE__MCU__PORT, true);
  sock_send = socket_init(PROTON_NODE__PC__IP, PROTON_NODE__PC__PORT, false);

  return (sock_recv >= 0 && sock_send >=0);
}

bool PROTON_TRANSPORT__McuDisconnect() { return true; }

size_t PROTON_TRANSPORT__McuRead(uint8_t *buf, size_t len) {
  int ret = recv(sock_recv, buf, len, 0);

  if (ret < 0) {
    return 0;
  }

  rx += ret;

  return ret;
}

size_t PROTON_TRANSPORT__McuWrite(const uint8_t *buf, size_t len) {
  int ret = send(sock_send, buf, len, 0);

  if (ret < 0) {
    return 0;
  }

  tx += ret;

  return ret;
}

pthread_mutex_t lock;

bool PROTON_MUTEX__McuLock() { return pthread_mutex_lock(&lock) == 0; }

bool PROTON_MUTEX__McuUnlock() { return pthread_mutex_unlock(&lock) == 0; }

void *timer_1hz(void *arg) {
  uint32_t i = 0;
  while (1) {
    LOG_INFO("1hz timer %ld", i++);
    update_status(i);
    update_emergency_stop();
    update_stop_status();
    update_alerts();
    msleep(1000);
  }
}

void *timer_10hz(void *arg) {
  uint32_t i = 0;
  while (1) {
    LOG_INFO("10hz timer %ld", i++);
    update_power();
    update_temperature();
    update_pinout_state();
    PROTON_BUNDLE_SendHeartbeat();
    msleep(100);
  }
}

void * stats(void *arg) {
  uint32_t i = 0;
  while (1) {
    printf("\033[2J\033[1;1H");
    printf("--------- A300 MCU C --------\r\n");
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
}

int main() {
  printf("~~~~~~~ A300 node ~~~~~~~\r\n");

  pthread_mutex_init(&lock, NULL);

  PROTON_Init();

  printf("INIT\r\n");

  strcpy(status_bundle.firmware_version, "3.0.0");
  strcpy(status_bundle.hardware_id, "A300");
  stop_status_bundle.needs_reset = true;

  pthread_t thread_10hz, thread_1hz, thread_stats;

  pthread_create(&thread_10hz, NULL, &timer_10hz, NULL);
  pthread_create(&thread_1hz, NULL, &timer_1hz, NULL);
  pthread_create(&thread_stats, NULL, &stats, NULL);

  PROTON_Spin(&mcu_node);

  pthread_join(thread_10hz, NULL);
  pthread_join(thread_1hz, NULL);
  pthread_join(thread_stats, NULL);

  return 0;
}
