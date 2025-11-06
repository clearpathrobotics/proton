#include "utils.h"
#include "proton__j100_mcu.h"
#include "stdarg.h"

#define PROTON_MAX_MESSAGE_SIZE 1024

uint8_t write_buf_[PROTON_MAX_MESSAGE_SIZE];
uint8_t read_buf_[PROTON_MAX_MESSAGE_SIZE];

double rx, tx;

proton_buffer_t proton_mcu_read_buffer = {read_buf_, PROTON_MAX_MESSAGE_SIZE};
proton_buffer_t proton_mcu_write_buffer = {write_buf_, PROTON_MAX_MESSAGE_SIZE};

int serial_port;

typedef enum {
  CALLBACK_WIFI_CONNECTED,
  CALLBACK_HMI,
  CALLBACK_MOTOR_COMMAND,
  CALLBACK_COUNT
} callback_e;

uint32_t cb_counts[CALLBACK_COUNT];

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
  motor_feedback_bundle.drivers_measured_velocity[0] = motor_command_bundle.drivers[0];
  motor_feedback_bundle.drivers_measured_velocity[1] = motor_command_bundle.drivers[1];
}

void PROTON_BUNDLE_PcHeartbeatCallback() {
  printf("Received heartbeat %u\r\n", pc_heartbeat_bundle.heartbeat);
}

void send_log(const char *file, const char* func, int line, uint8_t level, char *msg, ...) {
  strcpy(log_bundle.name, "J100_proton");
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

void update_stop_status() {
  stop_status_bundle.external_stop_present = !stop_status_bundle.external_stop_present;

  PROTON_BUNDLE_Send(PROTON_BUNDLE__STOP_STATUS);
}

void update_imu() {

  imu_bundle.linear_acceleration_x = rand_double();
  imu_bundle.linear_acceleration_y = rand_double();
  imu_bundle.linear_acceleration_z = rand_double();
  imu_bundle.angular_velocity_x = rand_double();
  imu_bundle.angular_velocity_y = rand_double();
  imu_bundle.angular_velocity_z = rand_double();

  PROTON_BUNDLE_Send(PROTON_BUNDLE__IMU);
}

void update_mag() {
  magnetometer_bundle.magnetic_field_x = rand_double();
  magnetometer_bundle.magnetic_field_y = rand_double();
  magnetometer_bundle.magnetic_field_z = rand_double();

  PROTON_BUNDLE_Send(PROTON_BUNDLE__MAGNETOMETER);
}

void update_nmea() {
  char tmp[PROTON_SIGNALS__NMEA__SENTENCE__CAPACITY] = {0};

  uint16_t i;

  for (i = 0; i < (rand() % (PROTON_SIGNALS__NMEA__SENTENCE__CAPACITY - 1)); i++)
  {
    tmp[i] = rand_char();
  }

  tmp[i+1] = '\0';
  strcpy(nmea_bundle.sentence, tmp);

  PROTON_BUNDLE_Send(PROTON_BUNDLE__NMEA);
}

void update_motor_feedback() {

  for (uint8_t i = 0; i < 2; i++)
  {
    motor_feedback_bundle.drivers_current[i] = rand_float();
    motor_feedback_bundle.drivers_bridge_temperature[i] = rand_float();
    motor_feedback_bundle.drivers_motor_temperature[i] = rand_float();
    motor_feedback_bundle.drivers_driver_fault[i] = rand_bool();
    motor_feedback_bundle.drivers_duty_cycle[i] = rand_float();
    motor_feedback_bundle.drivers_measured_travel[i] = rand_float();
  }

  PROTON_BUNDLE_Send(PROTON_BUNDLE__MOTOR_FEEDBACK);
}

bool PROTON_TRANSPORT__McuConnect() {
  serial_port = serial_init(PROTON_NODE__MCU__DEVICE);
  return serial_port >= 0;
}

bool PROTON_TRANSPORT__McuDisconnect() { return true; }

size_t PROTON_TRANSPORT__McuRead(uint8_t *buf, size_t len) {
  size_t bytes_read = serial_read(serial_port, buf, len);

  if (bytes_read > 0)
  {
    rx += bytes_read + PROTON_FRAME_OVERHEAD;
  }

  return bytes_read;
}

size_t PROTON_TRANSPORT__McuWrite(const uint8_t *buf, size_t len) {
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
    PROTON_BUNDLE_SendHeartbeat();
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

  printf("INIT\r\n");

  strcpy(status_bundle.firmware_version, "3.0.0");
  strcpy(status_bundle.hardware_id, "J100");

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
