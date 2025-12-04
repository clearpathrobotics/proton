#include "utils.h"
#include "proton__j100_pc.h"



uint8_t write_buf_[PROTON_MAX_MESSAGE_SIZE];
uint8_t read_buf_[PROTON_MAX_MESSAGE_SIZE];

double rx, tx;

uint32_t last_mcu_heartbeat = 0;

proton_buffer_t proton_pc_buffer = {write_buf_, PROTON_MAX_MESSAGE_SIZE};
proton_buffer_t proton_mcu_buffer = {read_buf_, PROTON_MAX_MESSAGE_SIZE};

int serial_port;

#define MAX_LOGS 100

char logs[MAX_LOGS][PROTON__BUNDLE__LOG__SIGNAL__MSG__CAPACITY];
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

void proton_bundle_log_callback()
{
  cb_counts[CALLBACK_LOG]++;
  strncpy(logs[log_index++], log_bundle.msg, PROTON__BUNDLE__LOG__SIGNAL__MSG__CAPACITY);
  memset(log_bundle.msg, '\0', PROTON__BUNDLE__LOG__SIGNAL__MSG__CAPACITY);
}

void proton_bundle_status_callback()
{
  cb_counts[CALLBACK_STATUS]++;
}

void proton_bundle_power_callback()
{
  cb_counts[CALLBACK_POWER]++;
}

void proton_bundle_emergency_stop_callback()
{
  cb_counts[CALLBACK_ESTOP]++;
}

void proton_bundle_temperature_callback()
{
  cb_counts[CALLBACK_TEMPERATURE]++;
}

void proton_bundle_stop_status_callback()
{
  cb_counts[CALLBACK_STOP_STATUS]++;
}

void proton_bundle_imu_callback()
{
  cb_counts[CALLBACK_IMU]++;
}

void proton_bundle_magnetometer_callback()
{
  cb_counts[CALLBACK_MAG]++;
}

void proton_bundle_nmea_callback()
{
  cb_counts[CALLBACK_NMEA]++;
}

void proton_bundle_motor_feedback_callback()
{
  cb_counts[CALLBACK_MOTOR_FEEDBACK]++;
}

void proton_bundle_mcu_heartbeat_callback()
{
  printf("Heartbeat received %u\r\n", mcu_heartbeat_bundle.heartbeat);
  pc_node.peers[PROTON__PEER__MCU].state = PROTON_NODE_ACTIVE;
  last_mcu_heartbeat = time(NULL);
}

bool proton_node_mcu_transport_connect() {
  serial_port = serial_init(PROTON__NODE__MCU__ENDPOINT__0__DEVICE);
  return serial_port >= 0;
}

bool proton_node_mcu_transport_disconnect() { return true; }

size_t proton_node_mcu_transport_read(uint8_t *buf, size_t len) {
  size_t bytes_read = serial_read(serial_port, buf, len);

  if (bytes_read > 0)
  {
    rx += bytes_read + PROTON_FRAME_OVERHEAD;
  }

  return bytes_read;
}

size_t proton_node_mcu_transport_write(const uint8_t *buf, size_t len) {
  size_t bytes_written = serial_write(serial_port, buf, len);

  if (bytes_written > 0)
  {
    tx += bytes_written + PROTON_FRAME_OVERHEAD;
  }

  return bytes_written;
}

pthread_mutex_t pc_lock, mcu_lock;

bool PROTON_MUTEX__McuLock() { return pthread_mutex_lock(&mcu_lock) == 0; }

bool PROTON_MUTEX__McuUnlock() { return pthread_mutex_unlock(&mcu_lock) == 0; }

bool PROTON_MUTEX__PcLock() { return pthread_mutex_lock(&pc_lock) == 0; }

bool PROTON_MUTEX__PcUnlock() { return pthread_mutex_unlock(&pc_lock) == 0; }

void update_wifi_connected()
{
  wifi_connected_bundle.data = !wifi_connected_bundle.data;
  proton_bundle_send(PROTON__BUNDLE__WIFI_CONNECTED);
}

void update_hmi()
{
  hmi_bundle.data = rand() % 8;
  proton_bundle_send(PROTON__BUNDLE__HMI);
}

void update_motor_command()
{
  motor_command_bundle.mode = -1;
  motor_command_bundle.drivers[0] = 1.0f;
  motor_command_bundle.drivers[1] = -1.0f;

  proton_bundle_send(PROTON__BUNDLE__MOTOR_COMMAND);
}

void *timer_1hz(void *arg) {
  uint32_t i = 0;
  while (1) {
    update_wifi_connected();
    update_hmi();
    proton_bundle_send_heartbeat();
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
    printf("Node: %u\r\n", pc_node.state);
    printf("Peer: %s\r\n", PROTON__NODE__MCU__NAME);
    printf("  State: %u, Transport: %u\r\n", pc_node.peers[PROTON__PEER__MCU].state, pc_node.peers[PROTON__PEER__MCU].transport.state);
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

  pthread_mutex_init(&pc_lock, NULL);
  pthread_mutex_init(&mcu_lock, NULL);

  proton_init();

  pthread_t thread_50hz, thread_1hz, thread_stats;

  pthread_create(&thread_50hz, NULL, &timer_50hz, NULL);
  pthread_create(&thread_1hz, NULL, &timer_1hz, NULL);
  pthread_create(&thread_stats, NULL, &stats, NULL);

  proton_spin(&pc_node, PROTON__PEER__MCU);

  pthread_join(thread_50hz, NULL);
  pthread_join(thread_1hz, NULL);
  pthread_join(thread_stats, NULL);

  return 0;
}
