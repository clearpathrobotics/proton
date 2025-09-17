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

double rx, tx;

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
  printf("Received Bundle {\r\n");
  printf("  id: 0x%x\r\n", PROTON_BUNDLE__BATTERY);
  printf("  percentage: %f\r\n", battery_bundle.percentage);
  printf("}\r\n");
}

void PROTON_BUNDLE_PinoutCommandCallback() {
  cb_counts[CALLBACK_PINOUT_COMMAND]++;
}

void PROTON_BUNDLE_CmdShutdownCallback() {
  cb_counts[CALLBACK_CMD_SHUTDOWN]++;
}

void PROTON_BUNDLE_ClearNeedsResetCallback() {
  cb_counts[CALLBACK_CLEAR_NEEDS_RESET]++;
  stop_status_bundle.needs_reset = false;
}

void send_log(char *file, int line, uint8_t level, char *msg, ...) {
  strcpy(log_bundle.name, "A300_proton");
  strcpy(log_bundle.file, file);
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
  emergency_stop_bundle.data = !emergency_stop_bundle.data;

  PROTON_BUNDLE_Send(PROTON_BUNDLE__EMERGENCY_STOP);
}

void update_stop_status() { PROTON_BUNDLE_Send(PROTON_BUNDLE__STOP_STATUS); }

void update_alerts() {
  strcpy(alerts_bundle.data, "E124,E100");

  PROTON_BUNDLE_Send(PROTON_BUNDLE__ALERTS);
}

// void print_const_bundle() {
//   printf("%lf\r\n", test_const_bundle.const_double);
//   printf("%f\r\n", test_const_bundle.const_float);
//   printf("%d\r\n", test_const_bundle.const_int32);
//   printf("%ld\r\n", test_const_bundle.const_int64);
//   printf("%u\r\n", test_const_bundle.const_uint32);
//   printf("%lu\r\n", test_const_bundle.const_uint64);
//   printf("%u\r\n", test_const_bundle.const_bool);
//   printf("%s\r\n", test_const_bundle.const_string);
//   printf("[");
//   for (uint8_t i = 0; i < PROTON_SIGNALS__TEST_CONST__CONST_BYTES__CAPACITY; i++)
//   {
//     if (i == PROTON_SIGNALS__TEST_CONST__CONST_BYTES__CAPACITY - 1)
//     {
//       printf("%u]\r\n", test_const_bundle.const_bytes[i]);
//     }
//     else
//     {
//       printf("%u, ", test_const_bundle.const_bytes[i]);
//     }
//   }

//   printf("[");
//   for (uint8_t i = 0; i < PROTON_SIGNALS__TEST_CONST__CONST_LIST_DOUBLES__LENGTH; i++)
//   {
//     if (i == PROTON_SIGNALS__TEST_CONST__CONST_LIST_DOUBLES__LENGTH - 1)
//     {
//       printf("%lf]\r\n", test_const_bundle.const_list_doubles[i]);
//     }
//     else
//     {
//       printf("%lf, ", test_const_bundle.const_list_doubles[i]);
//     }
//   }

//   printf("[");
//   for (uint8_t i = 0; i < PROTON_SIGNALS__TEST_CONST__CONST_LIST_FLOATS__LENGTH; i++)
//   {
//     if (i == PROTON_SIGNALS__TEST_CONST__CONST_LIST_FLOATS__LENGTH - 1)
//     {
//       printf("%f]\r\n", test_const_bundle.const_list_floats[i]);
//     }
//     else
//     {
//       printf("%f, ", test_const_bundle.const_list_floats[i]);
//     }
//   }

//   printf("[");
//   for (uint8_t i = 0; i < PROTON_SIGNALS__TEST_CONST__CONST_LIST_INT32S__LENGTH; i++)
//   {
//     if (i == PROTON_SIGNALS__TEST_CONST__CONST_LIST_INT32S__LENGTH - 1)
//     {
//       printf("%d]\r\n", test_const_bundle.const_list_int32s[i]);
//     }
//     else
//     {
//       printf("%d, ", test_const_bundle.const_list_int32s[i]);
//     }
//   }

//   printf("[");
//   for (uint8_t i = 0; i < PROTON_SIGNALS__TEST_CONST__CONST_LIST_INT64S__LENGTH; i++)
//   {
//     if (i == PROTON_SIGNALS__TEST_CONST__CONST_LIST_INT64S__LENGTH - 1)
//     {
//       printf("%ld]\r\n", test_const_bundle.const_list_int64s[i]);
//     }
//     else
//     {
//       printf("%ld, ", test_const_bundle.const_list_int64s[i]);
//     }
//   }

//   printf("[");
//   for (uint8_t i = 0; i < PROTON_SIGNALS__TEST_CONST__CONST_LIST_UINT32S__LENGTH; i++)
//   {
//     if (i == PROTON_SIGNALS__TEST_CONST__CONST_LIST_UINT32S__LENGTH - 1)
//     {
//       printf("%u]\r\n", test_const_bundle.const_list_uint32s[i]);
//     }
//     else
//     {
//       printf("%u, ", test_const_bundle.const_list_uint32s[i]);
//     }
//   }

//   printf("[");
//   for (uint8_t i = 0; i < PROTON_SIGNALS__TEST_CONST__CONST_LIST_UINT64S__LENGTH; i++)
//   {
//     if (i == PROTON_SIGNALS__TEST_CONST__CONST_LIST_UINT64S__LENGTH - 1)
//     {
//       printf("%u]\r\n", test_const_bundle.const_list_uint64s[i]);
//     }
//     else
//     {
//       printf("%u, ", test_const_bundle.const_list_uint64s[i]);
//     }
//   }

//   printf("[");
//   for (uint8_t i = 0; i < PROTON_SIGNALS__TEST_CONST__CONST_LIST_BOOLS__LENGTH; i++)
//   {
//     if (i == PROTON_SIGNALS__TEST_CONST__CONST_LIST_BOOLS__LENGTH - 1)
//     {
//       printf("%u]\r\n", test_const_bundle.const_list_bools[i]);
//     }
//     else
//     {
//       printf("%u, ", test_const_bundle.const_list_bools[i]);
//     }
//   }

//   printf("[");
//   for (uint8_t i = 0; i < PROTON_SIGNALS__TEST_CONST__CONST_LIST_STRING__LENGTH; i++)
//   {
//     if (i == PROTON_SIGNALS__TEST_CONST__CONST_LIST_STRING__LENGTH - 1)
//     {
//       printf("%s]\r\n", test_const_bundle.const_list_string[i]);
//     }
//     else
//     {
//       printf("%s, ", test_const_bundle.const_list_string[i]);
//     }
//   }
// }

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
