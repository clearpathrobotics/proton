#include <pthread.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <stdarg.h>

#include "proton__j100_mcu.h"

#define PROTON_MAX_MESSAGE_SIZE 1024

uint8_t write_buf_[PROTON_MAX_MESSAGE_SIZE];
uint8_t read_buf_[PROTON_MAX_MESSAGE_SIZE];

double rx, tx;

proton_buffer_t proton_mcu_read_buffer = {read_buf_, PROTON_MAX_MESSAGE_SIZE};
proton_buffer_t proton_mcu_write_buffer = {write_buf_, PROTON_MAX_MESSAGE_SIZE};

int serial_port;

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
  CALLBACK_WIFI_CONNECTED,
  CALLBACK_HMI,
  CALLBACK_MOTOR_COMMAND,
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


int serialInit()
{
  serial_port = open(PROTON_NODE__MCU__DEVICE, O_RDWR | O_NOCTTY | O_SYNC);

  if (serial_port == -1) {
    printf("Error opening serial device\r\n");
    return -1;
  }

  struct termios tty;

  if (tcgetattr(serial_port, &tty) != 0) {
    return -1;
  }

  cfsetospeed(&tty, B921600);
  cfsetispeed(&tty, B921600);
  tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8; // 8-bit
  tty.c_cflag |= (CLOCAL | CREAD);            // enable receiver
  tcsetattr(serial_port, TCSANOW, &tty);

  return 0;
}

void PROTON_BUNDLE_WifiConnectedCallback() {
  cb_counts[CALLBACK_WIFI_CONNECTED]++;
}

void PROTON_BUNDLE_HmiCallback() {
  cb_counts[CALLBACK_HMI]++;
}

void PROTON_BUNDLE_MotorCommandCallback() {
  cb_counts[CALLBACK_MOTOR_COMMAND]++;
  motor_feedback_bundle.actual_mode = motor_command_bundle.mode;
  motor_feedback_bundle.commanded_mode = motor_command_bundle.mode;
  motor_feedback_bundle.measured_velocity[0] = motor_command_bundle.command[0];
  motor_feedback_bundle.measured_velocity[1] = motor_command_bundle.command[1];
}

void send_log(char *file, int line, uint8_t level, char *msg, ...) {
  strcpy(log_bundle.name, "J100_proton");
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

  PROTON_BUNDLE_Send(PROTON_BUNDLE__POWER);
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

void update_stop_status() {
  static bool external_stop = false;

  external_stop = !external_stop;
  stop_status_bundle.external_stop_present = external_stop;

  PROTON_BUNDLE_Send(PROTON_BUNDLE__STOP_STATUS);
}

void update_imu() {
  for (uint8_t i = 0; i < PROTON_SIGNALS__IMU__LINEAR_ACCELERATION__LENGTH; i++)
  {
    imu_bundle.linear_acceleration[i] = (double)rand();
    imu_bundle.angular_velocity[i] = (double)rand();
  }

  PROTON_BUNDLE_Send(PROTON_BUNDLE__IMU);
}

void update_mag() {
  for (uint8_t i = 0; i < PROTON_SIGNALS__MAGNETOMETER__MAGNETIC_FIELD__LENGTH; i++)
  {
    magnetometer_bundle.magnetic_field[i] = (double)rand();
  }

  PROTON_BUNDLE_Send(PROTON_BUNDLE__MAGNETOMETER);
}

void update_nmea() {
  static const char alphanum[] =
      "0123456789"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz";

  char tmp[PROTON_SIGNALS__NMEA__SENTENCE__CAPACITY];

  uint16_t i;

  for (i = 0; i < (rand() % (PROTON_SIGNALS__NMEA__SENTENCE__CAPACITY - 1)); i++)
  {
    tmp[i] = alphanum[rand()% (sizeof(alphanum) - 1)];
  }

  tmp[i+1] = '\0';

  PROTON_BUNDLE_Send(PROTON_BUNDLE__NMEA);
}

void update_motor_feedback() {

  for (uint8_t i = 0; i < 2; i++)
  {
    motor_feedback_bundle.current[i] = (float)rand();
    motor_feedback_bundle.bridge_temperature[i] = (float)rand();
    motor_feedback_bundle.motor_temperature[i] = (float)rand();
    motor_feedback_bundle.driver_fault[i] = (bool)(rand() % 2);
    motor_feedback_bundle.duty_cycle[i] = (float)rand();
    motor_feedback_bundle.measured_travel[i] = (float)rand();
  }

  PROTON_BUNDLE_Send(PROTON_BUNDLE__MOTOR_FEEDBACK);
}

bool PROTON_TRANSPORT__McuConnect() {
  return serialInit() == 0;
}

bool PROTON_TRANSPORT__McuDisconnect() { return true; }

size_t PROTON_TRANSPORT__McuRead(uint8_t *buf, size_t len) {
  // Read header first
  int ret = read(serial_port, buf, PROTON_FRAME_HEADER_OVERHEAD);

  if (ret < 0) {
    return 0;
  }

  // Get payload length from header
  uint16_t payload_len = PROTON_GetFramedPayloadLength(buf);

  // Invalid header
  if (payload_len == 0)
  {
    return 0;
  }

  // Read payload
  ret = read(serial_port, buf, payload_len);

  if (ret != payload_len)
  {
    return 0;
  }

  uint8_t crc[2];

  ret = read(serial_port, crc, PROTON_FRAME_CRC_OVERHEAD);

  // Check for valid CRC16
  if (ret != PROTON_FRAME_CRC_OVERHEAD || !PROTON_CheckFramedPayload(buf, payload_len, (uint16_t)(crc[0] | (crc[1] << 8))))
  {
    return 0;
  }

  rx += payload_len + PROTON_FRAME_OVERHEAD;

  return payload_len;
}

size_t PROTON_TRANSPORT__McuWrite(const uint8_t *buf, size_t len) {
  uint8_t header[4];
  uint8_t crc[2];

  if (!PROTON_FillFrameHeader(header, len))
  {
    return 0;
  }

  if (!PROTON_FillCRC16(buf, len, crc))
  {
    return 0;
  }

  // Write header
  int ret = write(serial_port, header, PROTON_FRAME_HEADER_OVERHEAD);

  if (ret < 0) {
    return 0;
  }

  // Write payload
  ret = write(serial_port, buf, len);

  if (ret < 0) {
    return 0;
  }

  // Write CRC16
  ret = write(serial_port, crc, PROTON_FRAME_CRC_OVERHEAD);

  if (ret < 0) {
    return 0;
  }

  tx += len + PROTON_FRAME_OVERHEAD;

  return len;
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
    msleep(1000);
  }
}

void *timer_10hz(void *arg) {
  uint32_t i = 0;
  while (1) {
    LOG_INFO("10hz timer %ld", i++);
    update_power();
    update_temperature();
    msleep(100);
  }
}

void *timer_50hz(void *arg) {
  uint32_t i = 0;
  while (1) {
    update_imu();
    update_mag();
    update_nmea();
    update_motor_feedback();
    msleep(20);
  }
}

void *stats(void *arg) {
  uint32_t i = 0;
  while (1) {
    printf("\033[2J\033[1;1H");
    printf("--------- J100 MCU C --------\r\n");
    printf("Rx: %.3lf KB/s Tx: %.3lf KB/s\r\n", rx / 1000, tx / 1000);
    printf("--- Received Bundles (hz) ---\r\n");
    printf("wifi_connected: %d\r\n", cb_counts[CALLBACK_WIFI_CONNECTED]);
    printf("hmi: %d\r\n", cb_counts[CALLBACK_HMI]);
    printf("motor_command: %d\r\n", cb_counts[CALLBACK_MOTOR_COMMAND]);
    printf("-----------------------------\r\n");

    rx = 0.0;
    tx = 0.0;

    for (uint8_t i = 0; i < CALLBACK_COUNT; i++) {
      cb_counts[i] = 0;
    }

    msleep(1000);
  }
}


int main() {
  printf("~~~~~~~ J100 node ~~~~~~~\r\n");

  pthread_mutex_init(&lock, NULL);

  PROTON_Init();

  serialInit();

  printf("INIT\r\n");

  strcpy(status_bundle.firmware_version, "3.0.0");
  strcpy(status_bundle.hardware_id, "J100");

  strcpy(imu_bundle.frame_id, "imu_0_link");
  strcpy(magnetometer_bundle.frame_id, "imu_0_link");
  strcpy(nmea_bundle.frame_id, "gps_0_link");

  pthread_t thread_50hz, thread_10hz, thread_1hz, thread_stats;

  pthread_create(&thread_50hz, NULL, &timer_50hz, NULL);
  pthread_create(&thread_10hz, NULL, &timer_10hz, NULL);
  pthread_create(&thread_1hz, NULL, &timer_1hz, NULL);
  pthread_create(&thread_stats, NULL, &stats, NULL);

  PROTON_Spin(&mcu_node);

  pthread_join(thread_50hz, NULL);
  pthread_join(thread_10hz, NULL);
  pthread_join(thread_1hz, NULL);
  pthread_join(thread_stats, NULL);

  return 0;
}
