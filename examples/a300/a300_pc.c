#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>

#include "proton__a300_pc.h"

#define PROTON_MAX_MESSAGE_SIZE 1024

uint8_t write_buf_[PROTON_MAX_MESSAGE_SIZE];
uint8_t read_buf_[PROTON_MAX_MESSAGE_SIZE];

double rx, tx;

proton_buffer_t proton_pc_read_buffer = {read_buf_, PROTON_MAX_MESSAGE_SIZE};
proton_buffer_t proton_pc_write_buffer = {write_buf_, PROTON_MAX_MESSAGE_SIZE};

int sock_send, sock_recv;

#define MAX_LOGS 100

char * logs[MAX_LOGS][PROTON_SIGNALS__LOG__MSG__CAPACITY];
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
  servaddr.sin_addr.s_addr = htonl((in_addr_t)PROTON_NODE__MCU__IP);
  servaddr.sin_port = htons(PROTON_NODE__MCU__PORT);

  if (connect(sock_send, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) {
    printf("connect error\r\n");
    return 1;
  }

  printf("Send Socket connected\r\n");

  struct sockaddr_in servaddr2;
  sock_recv = socket(AF_INET, SOCK_DGRAM, 0);

  memset(&servaddr2, 0, sizeof(servaddr2));
  servaddr2.sin_family = AF_INET;
  servaddr2.sin_addr.s_addr = htonl((in_addr_t)PROTON_NODE__PC__IP);
  servaddr2.sin_port = htons(PROTON_NODE__PC__PORT);

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

void PROTON_BUNDLE_LogCallback()
{
  cb_counts[CALLBACK_LOG]++;
  strncpy(logs[log_index++], log_bundle.msg, PROTON_SIGNALS__LOG__MSG__CAPACITY);
  memset(log_bundle.msg, '\0', PROTON_SIGNALS__LOG__MSG__CAPACITY);
}

void PROTON_BUNDLE_StatusCallback()
{
  cb_counts[CALLBACK_STATUS]++;
}

void PROTON_BUNDLE_PowerCallback()
{
  cb_counts[CALLBACK_POWER]++;
}

void PROTON_BUNDLE_EmergencyStopCallback()
{
  cb_counts[CALLBACK_ESTOP]++;
}

void PROTON_BUNDLE_TemperatureCallback()
{
  cb_counts[CALLBACK_TEMPERATURE]++;
}

void PROTON_BUNDLE_StopStatusCallback()
{
  cb_counts[CALLBACK_STOP_STATUS]++;
}

void PROTON_BUNDLE_PinoutStateCallback()
{
  cb_counts[CALLBACK_PINOUT_STATE]++;
}

void PROTON_BUNDLE_AlertsCallback()
{
  cb_counts[CALLBACK_ALERTS]++;
}

void update_lights()
{
  for (uint8_t i = 0; i < PROTON_SIGNALS__CMD_LIGHTS__FRONT_LEFT_LIGHT__CAPACITY; i++)
  {
    cmd_lights_bundle.front_left_light[i] = rand() % 255;
    cmd_lights_bundle.front_right_light[i] = rand() % 255;
    cmd_lights_bundle.rear_left_light[i] = rand() % 255;
    cmd_lights_bundle.rear_right_light[i] = rand() % 255;
  }

  PROTON_BUNDLE_Send(PROTON_BUNDLE__CMD_LIGHTS);
}

void update_fans()
{
  for (uint8_t i = 0; i < PROTON_SIGNALS__CMD_FANS__FAN_SPEEDS__CAPACITY; i++)
  {
    cmd_fans_bundle.fan_speeds[i] = rand() % 255;
  }

  PROTON_BUNDLE_Send(PROTON_BUNDLE__CMD_FANS);
}

void update_display_status()
{
  strncpy(display_status_bundle.string_1, "TEST_STRING", PROTON_SIGNALS__DISPLAY_STATUS__STRING_1__CAPACITY);
  strncpy(display_status_bundle.string_2, "TEST_STRING2", PROTON_SIGNALS__DISPLAY_STATUS__STRING_2__CAPACITY);

  PROTON_BUNDLE_Send(PROTON_BUNDLE__DISPLAY_STATUS);
}

void update_battery()
{
  battery_bundle.percentage = (float)rand();
  PROTON_BUNDLE_Send(PROTON_BUNDLE__BATTERY);
}

void update_pinout_command()
{
  for (uint8_t i = 0; i < PROTON_SIGNALS__PINOUT_COMMAND__RAILS__LENGTH; i++)
  {
    pinout_command_bundle.rails[i] = rand() % 2;
  }
  for (uint8_t i = 0; i < PROTON_SIGNALS__PINOUT_COMMAND__RAILS__LENGTH; i++)
  {
    pinout_command_bundle.rails[i] = rand() % 2;
  }

  PROTON_BUNDLE_Send(PROTON_BUNDLE__PINOUT_COMMAND);
}

bool PROTON_TRANSPORT__PcConnect() { return socket_init() == 0; }

bool PROTON_TRANSPORT__PcDisconnect() { return true; }

size_t PROTON_TRANSPORT__PcRead(uint8_t *buf, size_t len) {
  int ret = recv(sock_recv, buf, len, 0);

  if (ret < 0) {
    return 0;
  }

  rx += ret;

  return ret;
}

size_t PROTON_TRANSPORT__PcWrite(const uint8_t *buf, size_t len) {
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
    update_fans();
    update_display_status();
    update_battery();
    update_pinout_command();
    msleep(1000);
  }
}

void *timer_20hz(void *arg) {
  uint32_t i = 0;
  while (1) {
    update_lights();
    msleep(50);
  }
}

void * stats(void *arg) {
  uint32_t i = 0;
  while (1) {
    printf("\033[2J\033[1;1H");
    printf("--------- A300 PC C ---------\r\n");
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

    for (uint8_t i = 0; i < log_index; i++)
    {
      printf("%s\r\n", logs[i]);
    }

    printf("-----------------------------\r\n");

    memset(logs, '\0', sizeof(logs));
    log_index = 0;

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

  pthread_t thread_20hz, thread_1hz, thread_stats;

  pthread_create(&thread_20hz, NULL, &timer_20hz, NULL);
  pthread_create(&thread_1hz, NULL, &timer_1hz, NULL);
  pthread_create(&thread_stats, NULL, &stats, NULL);

  PROTON_Spin(&pc_node);

  pthread_join(thread_20hz, NULL);
  pthread_join(thread_1hz, NULL);
  pthread_join(thread_stats, NULL);

  return 0;
}
