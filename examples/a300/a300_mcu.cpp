#include "protoncpp/proton.hpp"
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <thread>
#include <chrono>

proton::Node node;

void send_log(const char *file, const char* func, int line, uint8_t level, std::string msg, ...);

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

void send_log(const char *file, const char* func, int line, uint8_t level, std::string msg, ...) {
  auto& log_bundle = node.getBundle("log");
  log_bundle.getSignal("name").setValue<std::string>("a300_mcu_cpp");
  log_bundle.getSignal("file").setValue<std::string>(file);
  log_bundle.getSignal("line").setValue<uint32_t>(line);
  log_bundle.getSignal("level").setValue<uint32_t>(level);
  log_bundle.getSignal("function").setValue<std::string>(func);

  va_list args;
  va_start(args, msg);
  // Get string size
  int size = std::vsnprintf(nullptr, 0, msg.c_str(), args);
  va_end(args);

  std::string message(size, '\0');

  va_start(args, msg);
  std::vsnprintf(message.data(), size + 1, msg.c_str(), args);
  va_end(args);

  log_bundle.getSignal("msg").setValue<std::string>(message);

  node.sendBundle("log");
}

void update_status()
{
  auto& status_bundle = node.getBundle("status");
  status_bundle.getSignal("hardware_id").setValue<std::string>("A300_MCU");
  status_bundle.getSignal("firmware_version").setValue<std::string>("3.0.0");
  status_bundle.getSignal("mcu_uptime_s").setValue<int32_t>(rand());
  status_bundle.getSignal("mcu_uptime_ns").setValue<uint32_t>(rand());
  status_bundle.getSignal("connection_uptime_s").setValue<int32_t>(rand());
  status_bundle.getSignal("connection_uptime_ns").setValue<uint32_t>(rand());

  node.sendBundle(status_bundle);
}

void update_power()
{
  auto& power_bundle = node.getBundle("power");

  auto& measured_voltages = power_bundle.getSignal("measured_voltages");
  proton::list_float voltages(measured_voltages.getLength());

  for (auto i = 0; i < measured_voltages.getLength(); i++)
  {
    voltages.at(i) = static_cast<float>(rand());
  }

  auto& measured_currents = power_bundle.getSignal("measured_currents");
  proton::list_float currents(measured_currents.getLength());

  for (auto i = 0; i < measured_currents.getLength(); i++)
  {
    currents.at(i) = static_cast<float>(rand());
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
    temperatures.at(i) = static_cast<float>(rand());
  }

  node.sendBundle(temperature_bundle);
}

void update_emergency_stop()
{
  node.getBundle("emergency_stop").getSignal("data").setValue<bool>(!node.getBundle("emergency_stop").getSignal("data").getValue<bool>());
  node.sendBundle("emergency_stop");
}

bool needs_reset = true;

void update_stop_status()
{
  node.getBundle("stop_status").getSignal("needs_reset").setValue<bool>(needs_reset);
  node.sendBundle("stop_status");
}

void update_alerts()
{
  node.getBundle("alerts").getSignal("data").setValue<std::string>("E810");
  node.sendBundle("alerts");
}

void update_pinout_state()
{
  auto& pinout_state_bundle = node.getBundle("pinout_state");

  pinout_state_bundle.getSignal("rails").setValue<proton::list_bool>({rand() % 2});
  pinout_state_bundle.getSignal("inputs").setValue<proton::list_bool>({rand() % 2, rand() % 2, rand() % 2, rand() % 2, rand() % 2, rand() % 2, rand() % 2});
  pinout_state_bundle.getSignal("outputs").setValue<proton::list_bool>({rand() % 2, rand() % 2, rand() % 2, rand() % 2, rand() % 2, rand() % 2, rand() % 2});
  pinout_state_bundle.getSignal("output_periods").setValue<proton::list_uint32>({rand(), rand(), rand(), rand(), rand(), rand(), rand()});

  node.sendBundle(pinout_state_bundle);
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
    update_alerts();
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

void run_10hz_thread()
{
  uint32_t i = 0;
  while(1)
  {
    LOG_INFO("Test Log %d", i++);
    update_power();
    update_temperature();
    update_pinout_state();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
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

void clear_needs_reset_callback(proton::BundleHandle& bundle)
{
  needs_reset = false;
}

void cmd_lights_callback(proton::BundleHandle& bundle)
{
  bundle.printBundleVerbose();
}

int main()
{
  node = proton::Node(CONFIG_FILE, "mcu");

  node.registerCallback("clear_needs_reset", clear_needs_reset_callback);
  node.registerCallback("cmd_lights", cmd_lights_callback);

  std::thread stats_thread(run_stats_thread);
  std::thread send_1hz_thread(run_1hz_thread);
  std::thread send_10hz_thread(run_10hz_thread);

  node.startStatsThread();
  node.spin();

  stats_thread.join();
  send_1hz_thread.join();
  send_10hz_thread.join();

  return 0;
}

