#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdarg.h>

#include "proton__a300_mcu.h"

#define PROTON_MAX_MESSAGE_SIZE 1024

uint8_t write_buf_[PROTON_MAX_MESSAGE_SIZE];
uint8_t read_buf_[PROTON_MAX_MESSAGE_SIZE];

int sock_send, sock_recv;

void send_log(char* file, int line, uint8_t level, char* msg, ...);

#define LOG_DEBUG(message, ...) send_log(__FILE_NAME__, __LINE__, 10U, message, ##__VA_ARGS__)
#define LOG_INFO(message, ...) send_log(__FILE_NAME__, __LINE__, 20U, message, ##__VA_ARGS__)
#define LOG_WARNING(message, ...) send_log(__FILE_NAME__, __LINE__, 30U, message, ##__VA_ARGS__)
#define LOG_ERROR(message, ...) send_log(__FILE_NAME__, __LINE__, 40U, message, ##__VA_ARGS__)
#define LOG_FATAL(message, ...) send_log(__FILE_NAME__, __LINE__, 50U, message, ##__VA_ARGS__)

int msleep(long msec)
{
  struct timespec ts;
  int res;

  if (msec < 0)
  {
      return -1;
  }

  ts.tv_sec = msec / 1000;
  ts.tv_nsec = (msec % 1000) * 1000000;

  do {
      res = nanosleep(&ts, &ts);
  } while (res);

  return res;
}

int socket_init()
{
  struct sockaddr_in servaddr;
  sock_send = socket(AF_INET, SOCK_DGRAM, 0);

  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl((in_addr_t)PROTON_NODE__PC__IP);
  servaddr.sin_port = htons(PROTON_NODE__PC__PORT);

  if (connect(sock_send, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0)
  {
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
  if(fcntl(sock_recv, F_SETFL, fcntl(sock_recv, F_GETFL) | O_NONBLOCK) < 0) {
    printf("Set non-blocking error\r\n");
    return 2;
  }

  if (bind(sock_recv, (struct sockaddr *)&servaddr2, sizeof(servaddr2)) != 0)
  {
    printf("bind error\r\n");
    return 3;
  }

  printf("Receive Socket bound\r\n");

  return 0;
}

int send_bundle(proton_bundle_t * bundle)
{
  int e = PROTON_Encode(bundle, write_buf_, sizeof(write_buf_));

  if (e > 0)
  {
    send(sock_send, write_buf_, e, 0);
  }

  return e;
}

void PROTON_BUNDLE_CmdFansCallback()
{
  printf("Received CmdFans\r\n");
  print_bundle(cmd_fans_bundle.bundle);
}

void PROTON_BUNDLE_DisplayStatusCallback()
{
  printf("Received DisplayStatus\r\n");
  print_bundle(display_status_bundle.bundle);
}

void PROTON_BUNDLE_CmdLightsCallback()
{
  printf("Received CmdLights\r\n");
  print_bundle(cmd_lights_bundle.bundle);
}

void PROTON_BUNDLE_BatteryCallback()
{
  printf("Received Battery %f\r\n", battery_struct.percentage);
  print_bundle(battery_bundle.bundle);
}

void PROTON_BUNDLE_PinoutCommandCallback()
{
  printf("Received Pinout\r\n");
  print_bundle(pinout_command_bundle.bundle);
}

void PROTON_BUNDLE_CmdShutdownCallback()
{
  printf("~~~SHUTTING DOWN~~~\r\n");
}

void PROTON_BUNDLE_ClearNeedsResetCallback()
{
  printf("~~~Needs reset cleared~~~\r\n");
  stop_status_struct.needs_reset = false;
}

int get_bundle()
{
  proton_bundle_t * bundle;
  int s = recv(sock_recv, read_buf_, sizeof(read_buf_), 0);

  if (s > 0)
  {
    PROTON_BUNDLE_Decode(read_buf_, s);
  }

  return s;
}

void send_log(char* file, int line, uint8_t level, char* msg, ...)
{
  strcpy(logger_struct.name, "A300_proton");
  strcpy(logger_struct.file, file);
  logger_struct.line = line;
  logger_struct.level = level;

  va_list args;
  va_start(args, msg);
  vsprintf(logger_struct.msg, msg, args);
  va_end(args);

  send_bundle(&logger_bundle);
}

void update_power()
{
  for (uint8_t i = 0; i < POWER__MEASURED_VOLTAGES__LENGTH; i++)
  {
    power_struct.measured_currents[i] = (float)rand();
    power_struct.measured_voltages[i] = (float)rand();
  }
  send_bundle(&power_bundle);
}

void update_temperature()
{
  for (uint8_t i = 0; i < TEMPERATURE__TEMPERATURES__LENGTH; i++)
  {
    temperature_struct.temperatures[i] = (float)rand();
  }
  send_bundle(&temperature_bundle);
}

void update_status(uint32_t ms)
{
  status_struct.connection_uptime_s = ms / 1000;
  status_struct.connection_uptime_ns = rand();

  status_struct.mcu_uptime_s = ms / 1000;
  status_struct.mcu_uptime_ns = rand();

  send_bundle(&status_bundle);
}

void update_emergency_stop()
{
  static bool stopped = false;

  stopped = !stopped;
  emergency_stop_struct.stopped = stopped;

  send_bundle(&emergency_stop_bundle);
}

void update_stop_status()
{
  send_bundle(&stop_status_bundle);
}

void update_alerts()
{
  strcpy(alerts_struct.alert_string, "E124,E100");

  send_bundle(&alerts_bundle);
}

void update_pinout_state()
{
  pinout_state_struct.rails[0] = true;

  for (uint8_t i = 0; i < PINOUT_STATE__OUTPUTS__LENGTH; i++)
  {
    pinout_state_struct.outputs[i] = i % 2;
  }

  for (uint8_t i = 0; i < PINOUT_STATE__OUTPUT_PERIODS__LENGTH; i++)
  {
    pinout_state_struct.output_periods[i] = rand();
  }

  send_bundle(&pinout_state_bundle);
}

int main()
{
  printf("~~~~~~~ A300 node ~~~~~~~\r\n");

  if (socket_init() != 0)
  {
    return -1;
  }

  PROTON_BUNDLE_Init();

  printf("INIT\r\n");

  strcpy(status_struct.firmware_version, "3.0.0");
  strcpy(status_struct.hardware_id, "A300");
  stop_status_struct.needs_reset = true;

  uint32_t i = 0;

  while (1)
  {
    // 1 hz
    if (i % 1000 == 0)
    {
      update_status(i);
      update_emergency_stop();
      update_stop_status();
      update_alerts();
    }

    // 10hz
    if (i % 100 == 0)
    {
      update_power();
      update_temperature();
      update_pinout_state();
    }

    // 50hz
    if (i % 20 == 0)
    {

    }

    get_bundle();
    msleep(1);
    i++;
  }

  return 0;
}

