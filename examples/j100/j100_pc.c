#include "utils.h"
#include "proton__j100_pc.h"

#define PROTON_MAX_MESSAGE_SIZE 1024

uint8_t write_buf_[PROTON_MAX_MESSAGE_SIZE];
uint8_t read_buf_[PROTON_MAX_MESSAGE_SIZE];

double rx, tx;

proton_buffer_t proton_pc_read_buffer = {read_buf_, PROTON_MAX_MESSAGE_SIZE};
proton_buffer_t proton_pc_write_buffer = {write_buf_, PROTON_MAX_MESSAGE_SIZE};

int serial_port;

#define MAX_LOGS 100

char logs[MAX_LOGS][PROTON_SIGNALS__LOG__MSG__CAPACITY];
uint8_t log_index = 0;

typedef enum {
  CALLBACK_LOG,
  CALLBACK_STATUS,
  CALLBACK_POWER,
  CALLBACK_ESTOP,
  CALLBACK_TEMPERATURE,
  CALLBACK_STOP_STATUS,
  CALLBACK_IMU,
  CALLBACK_MAG,
  CALLBACK_NMEA,
  CALLBACK_MOTOR_FEEDBACK,
  CALLBACK_COUNT
} callback_e;

uint32_t cb_counts[CALLBACK_COUNT];

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

void PROTON_BUNDLE_ImuCallback()
{
  cb_counts[CALLBACK_IMU]++;
}

void PROTON_BUNDLE_MagnetometerCallback()
{
  cb_counts[CALLBACK_MAG]++;
}

void PROTON_BUNDLE_NmeaCallback()
{
  cb_counts[CALLBACK_NMEA]++;
}

void PROTON_BUNDLE_MotorFeedbackCallback()
{
  cb_counts[CALLBACK_MOTOR_FEEDBACK]++;
}

void PROTON_BUNDLE_McuHeartbeatCallback()
{
  printf("Received heartbeat %u\r\n", mcu_heartbeat_bundle.heartbeat);
}

bool PROTON_TRANSPORT__PcConnect() {
  serial_port = serial_init(PROTON_NODE__PC__DEVICE);
  return serial_port >= 0;
}

bool PROTON_TRANSPORT__PcDisconnect() { return true; }

size_t PROTON_TRANSPORT__PcRead(uint8_t *buf, size_t len) {
  size_t bytes_read = serial_read(serial_port, buf, len);

  if (bytes_read > 0)
  {
    rx += bytes_read + PROTON_FRAME_OVERHEAD;
  }

  return bytes_read;
}

size_t PROTON_TRANSPORT__PcWrite(const uint8_t *buf, size_t len) {
  size_t bytes_written = serial_write(serial_port, buf, len);

  if (bytes_written > 0)
  {
    tx += bytes_written + PROTON_FRAME_OVERHEAD;
  }

  return bytes_written;
}

pthread_mutex_t lock;

bool PROTON_MUTEX__McuLock() { return pthread_mutex_lock(&lock) == 0; }

bool PROTON_MUTEX__McuUnlock() { return pthread_mutex_unlock(&lock) == 0; }

void update_wifi_connected()
{
  wifi_connected_bundle.data = !wifi_connected_bundle.data;
  PROTON_BUNDLE_Send(PROTON_BUNDLE__WIFI_CONNECTED);
}

void update_hmi()
{
  hmi_bundle.data = rand() % 8;
  PROTON_BUNDLE_Send(PROTON_BUNDLE__HMI);
}

void update_motor_command()
{
  motor_command_bundle.mode = -1;
  motor_command_bundle.drivers[0] = 1.0f;
  motor_command_bundle.drivers[1] = -1.0f;

  PROTON_BUNDLE_Send(PROTON_BUNDLE__MOTOR_COMMAND);
}

void *timer_1hz(void *arg) {
  uint32_t i = 0;
  while (1) {
    update_wifi_connected();
    update_hmi();
    PROTON_BUNDLE_SendHeartbeat();
    msleep(1000);
  }
}

void *timer_50hz(void *arg) {
  uint32_t i = 0;
  while (1) {
    update_motor_command();
    msleep(20);
  }
}

void * stats(void *arg) {
  uint32_t i = 0;
  while (1) {
    printf("\033[2J\033[1;1H");
    printf("--------- J100 PC C ---------\r\n");
    printf("Node: %u Transport: %u\r\n", pc_node.state, pc_node.transport.state);
    printf("Rx: %.3lf KB/s Tx: %.3lf KB/s\r\n", rx / 1000, tx / 1000);
    printf("--- Received Bundles (hz) ---\r\n");
    printf("log: %d\r\n", cb_counts[CALLBACK_LOG]);
    printf("status: %d\r\n", cb_counts[CALLBACK_STATUS]);
    printf("power: %d\r\n", cb_counts[CALLBACK_POWER]);
    printf("emergency_stop: %d\r\n", cb_counts[CALLBACK_ESTOP]);
    printf("temperature: %d\r\n", cb_counts[CALLBACK_TEMPERATURE]);
    printf("stop_status: %d\r\n", cb_counts[CALLBACK_STOP_STATUS]);
    printf("imu: %d\r\n", cb_counts[CALLBACK_IMU]);
    printf("magnetometer: %d\r\n", cb_counts[CALLBACK_MAG]);
    printf("nmea: %d\r\n", cb_counts[CALLBACK_NMEA]);
    printf("motor_feedback: %d\r\n", cb_counts[CALLBACK_MOTOR_FEEDBACK]);
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
  printf("~~~~~~~ J100 node ~~~~~~~\r\n");

  pthread_mutex_init(&lock, NULL);

  PROTON_Init();

  pthread_t thread_50hz, thread_1hz, thread_stats;

  pthread_create(&thread_50hz, NULL, &timer_50hz, NULL);
  pthread_create(&thread_1hz, NULL, &timer_1hz, NULL);
  pthread_create(&thread_stats, NULL, &stats, NULL);

  PROTON_Spin(&pc_node);

  pthread_join(thread_50hz, NULL);
  pthread_join(thread_1hz, NULL);
  pthread_join(thread_stats, NULL);

  return 0;
}
