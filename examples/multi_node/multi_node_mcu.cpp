#include "protoncpp/proton.hpp"
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <thread>
#include <chrono>

std::unique_ptr<proton::Node> node;

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
  auto& log_bundle = node->getBundle("log");
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

  node->sendBundle("log");
}

void run_1hz_thread()
{
  uint32_t i = 0;
  while(1)
  {
    LOG_INFO("Test Log %d", i++);
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

void run_10hz_thread()
{
  uint32_t i = 0;
  while(1)
  {
    LOG_INFO("Test Log %d", i++);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

void run_stats_thread()
{
  while(1)
  {
    node->printStats();

    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

void empty_callback(proton::BundleHandle& bundle)
{
  bundle.printBundleVerbose();
}

int main()
{
  node = std::make_unique<proton::Node>(CONFIG_FILE, "mcu");

  std::thread stats_thread(run_stats_thread);
  std::thread send_1hz_thread(run_1hz_thread);
  std::thread send_10hz_thread(run_10hz_thread);

  node->startStatsThread();
  node->spin();

  stats_thread.join();
  send_1hz_thread.join();
  send_10hz_thread.join();

  return 0;
}

