#include "protoncpp/proton.hpp"
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <thread>
#include <chrono>

proton::Node node;

void send_log(char *file, const char* func, int line, uint8_t level, char *msg, ...);

#define LOG_DEBUG(message, ...)                                                \
  send_log(__FILE_NAME__, __func__, __LINE__, 10U, message, ##__VA_ARGS__)
#define LOG_INFO(message, ...)                                                 \
  send_log(__FILE_NAME__, __func__, __LINE__, 20U, message, ##__VA_ARGS__)
#define LOG_WARNING(message, ...)                                              \
  send_log(__FILE_NAME__, __func__, __LINE__, 30U, message, ##__VA_ARGS__)
#define LOG_ERROR(message, ...)                                                \
  send_log(__FILE_NAME__, __func__, __LINE__, 40U, message, ##__VA_ARGS__)
#define LOG_FATAL(message, ...)                                                \
  send_log(__FILE_NAME__, __func__, __LINE__, 50U, message, ##__VA_ARGS__)

void send_log(char *file, const char* func, int line, uint8_t level, char *msg, ...) {
  auto& log_bundle = node.getBundle("log");
  log_bundle.getSignal("name").setValue<std::string>("J100_mcu_cpp");
  log_bundle.getSignal("file").setValue<std::string>(file);
  log_bundle.getSignal("line").setValue<uint32_t>(line);
  log_bundle.getSignal("level").setValue<uint32_t>(level);
  log_bundle.getSignal("function").setValue<std::string>(func);

  va_list args;
  va_start(args, msg);
  // Get string size
  int size = std::vsnprintf(nullptr, 0, msg, args);
  va_end(args);

  std::string message(size, '\0');

  va_start(args, msg);
  std::vsnprintf(message.data(), size + 1, msg, args);
  va_end(args);

  log_bundle.getSignal("msg").setValue<std::string>(message);

  node.sendBundle("log");
}

void update_status()
{
  auto& status_bundle = node.getBundle("status");
  status_bundle.getSignal("hardware_id").setValue<std::string>("J100_MCU");
  status_bundle.getSignal("firmware_version").setValue<std::string>("3.0.0");
  status_bundle.getSignal("mcu_uptime_sec").setValue<int32_t>(rand());
  status_bundle.getSignal("mcu_uptime_nanosec").setValue<uint32_t>(rand());
  status_bundle.getSignal("connection_uptime_sec").setValue<int32_t>(rand());
  status_bundle.getSignal("connection_uptime_nanosec").setValue<uint32_t>(rand());

  node.sendBundle(status_bundle);
}

void update_power()
{
  auto& power_bundle = node.getBundle("power");

  auto& measured_voltages = power_bundle.getSignal("measured_voltages");
  proton::list_float voltages(measured_voltages.getLength());

  for (auto i = 0; i < measured_voltages.getLength(); i++)
  {
    measured_voltages.setValue<float>(i, static_cast<float>(rand()));
  }

  auto& measured_currents = power_bundle.getSignal("measured_currents");
  proton::list_float currents(measured_currents.getLength());

  for (auto i = 0; i < measured_currents.getLength(); i++)
  {
    measured_currents.setValue<float>(i, static_cast<float>(rand()));
  }

  node.sendBundle(power_bundle);
}

void update_temperature()
{
  auto& temperature_bundle = node.getBundle("temperature");

  auto& temperatures_signal = temperature_bundle.getSignal("temperatures");
  proton::list_float temperatures(temperatures_signal.getLength());

  for (auto i = 0; i < temperatures_signal.getLength(); i++)
  {
    temperatures_signal.setValue<float>(i, static_cast<float>(rand()));
  }

  node.sendBundle(temperature_bundle);
}

void update_emergency_stop()
{
  node.getBundle("emergency_stop").getSignal("data").setValue<bool>(true);
  node.sendBundle("emergency_stop");
}

void update_stop_status()
{
  node.getBundle("stop_status").getSignal("external_stop_present").setValue<bool>(rand() % 2);
  node.sendBundle("stop_status");
}

void update_imu()
{
  auto& imu_bundle = node.getBundle("imu");

  imu_bundle.getSignal("linear_acceleration_x").setValue<double>(static_cast<double>(rand()));
  imu_bundle.getSignal("linear_acceleration_y").setValue<double>(static_cast<double>(rand()));
  imu_bundle.getSignal("linear_acceleration_z").setValue<double>(static_cast<double>(rand()));

  imu_bundle.getSignal("angular_velocity_x").setValue<double>(static_cast<double>(rand()));
  imu_bundle.getSignal("angular_velocity_y").setValue<double>(static_cast<double>(rand()));
  imu_bundle.getSignal("angular_velocity_z").setValue<double>(static_cast<double>(rand()));

  node.sendBundle(imu_bundle);
}

void update_magnetometer()
{
  auto& mag_bundle = node.getBundle("magnetometer");

  mag_bundle.getSignal("magnetic_field_x").setValue<double>(static_cast<double>(rand()));
  mag_bundle.getSignal("magnetic_field_y").setValue<double>(static_cast<double>(rand()));
  mag_bundle.getSignal("magnetic_field_z").setValue<double>(static_cast<double>(rand()));

  node.sendBundle(mag_bundle);
}

std::string gen_random_string(const int len) {
  static const char alphanum[] =
      "0123456789"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz";
  std::string tmp_s;
  tmp_s.reserve(len);

  for (int i = 0; i < len; ++i) {
      tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
  }

  return tmp_s;
}

void update_nmea()
{
  auto& nmea_bundle = node.getBundle("nmea");

  nmea_bundle.getSignal("sentence").setValue<std::string>(gen_random_string(rand() % nmea_bundle.getSignal("sentence").getCapacity()));

  node.sendBundle(nmea_bundle);
}

void update_motor_feedback()
{
  auto& feedback_bundle = node.getBundle("motor_feedback");

  feedback_bundle.getSignal("drivers_current").setValue<proton::list_float>({static_cast<float>(rand()), static_cast<float>(rand())});
  feedback_bundle.getSignal("drivers_bridge_temperature").setValue<proton::list_float>({static_cast<float>(rand()), static_cast<float>(rand())});
  feedback_bundle.getSignal("drivers_motor_temperature").setValue<proton::list_float>({static_cast<float>(rand()), static_cast<float>(rand())});
  feedback_bundle.getSignal("drivers_driver_fault").setValue<proton::list_bool>({static_cast<bool>(rand() % 2), static_cast<bool>(rand() % 2)});
  feedback_bundle.getSignal("drivers_duty_cycle").setValue<proton::list_float>({static_cast<float>(rand()), static_cast<float>(rand())});
  feedback_bundle.getSignal("drivers_measured_velocity").setValue<proton::list_float>({static_cast<float>(rand()), static_cast<float>(rand())});
  feedback_bundle.getSignal("drivers_measured_travel").setValue<proton::list_float>({static_cast<float>(rand()), static_cast<float>(rand())});

  node.sendBundle(feedback_bundle);
}

void run_1hz_thread()
{
  uint32_t i = 0;
  while(1)
  {
    LOG_INFO("Test Log %d", i++);
    update_status();
    update_emergency_stop();
    update_stop_status();
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
}

void run_10hz_thread()
{
  uint32_t i = 0;
  while(1)
  {
    update_power();
    update_temperature();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

void run_50hz_thread()
{
  uint32_t i = 0;
  while(1)
  {
    update_imu();
    update_magnetometer();
    update_nmea();
    update_motor_feedback();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
  }
}

void run_stats_thread()
{
  while(1)
  {
    node.printStats();
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

int main()
{
  node = proton::Node(CONFIG_FILE, "mcu");

  std::thread stats_thread(run_stats_thread);
  std::thread send_1hz_thread(run_1hz_thread);
  std::thread send_10hz_thread(run_10hz_thread);
  std::thread send_50hz_thread(run_50hz_thread);

  node.startStatsThread();
  node.spin();

  stats_thread.join();
  send_1hz_thread.join();
  send_10hz_thread.join();
  send_50hz_thread.join();

  return 0;
}

