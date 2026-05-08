/*
 * Copyright 2026 Rockwell Automation Technologies, Inc., All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @author Roni Kreinin (roni.kreinin@rockwellautomation.com)
 */

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include "protoncpp/proton.hpp"

std::unique_ptr<proton::Node> node;

std::vector<std::string> logs;

void send_log(const char * file, const char * func, int line, uint8_t level, std::string msg, ...);

#define LOG_DEBUG(message, ...) \
  send_log(__FILE_NAME__, __func__, __LINE__, 10U, message, ##__VA_ARGS__)
#define LOG_INFO(message, ...) \
  send_log(__FILE_NAME__, __func__, __LINE__, 20U, message, ##__VA_ARGS__)
#define LOG_WARNING(message, ...) \
  send_log(__FILE_NAME__, __func__, __LINE__, 30U, message, ##__VA_ARGS__)
#define LOG_ERROR(message, ...) \
  send_log(__FILE_NAME__, __func__, __LINE__, 40U, message, ##__VA_ARGS__)
#define LOG_FATAL(message, ...) \
  send_log(__FILE_NAME__, __func__, __LINE__, 50U, message, ##__VA_ARGS__)

void send_log(const char * file, const char * func, int line, uint8_t level, std::string msg, ...)
{
  auto & log_bundle = node->getBundle("log");
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

void send_heartbeat(uint32_t count, const std::string & node_name)
{
  auto & heartbeat_signal = node->getBundle(node_name).getSignal("heartbeat");
  heartbeat_signal.setValue<uint32_t>(count);
  node->sendBundle(node_name);
}

void run_1hz_thread()
{
  uint32_t i = 0;
  while (1)
  {
    LOG_INFO("Test Log %d", i++);
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

void run_10hz_thread()
{
  uint32_t i = 0;
  while (1)
  {
    i++;
    LOG_INFO("Test Log %d", i);
    send_heartbeat(i, "node1_heartbeat");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

void run_stats_thread()
{
  while (1)
  {
    node->printStats();
    std::cout << "------------- Logs --------------" << std::endl;

    for (auto & l : logs)
    {
      std::cout << l << std::endl;
    }

    std::cout << "---------------------------------" << std::endl;

    logs.clear();

    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

void print_callback(proton::BundleHandle & bundle) { bundle.printBundleVerbose(); }

/**
 * @brief Callback function to handle heartbeat bundle from peer
 *
 * @param bundle the bundle containing the heartbeat signal
 */
void heartbeat_callback(proton::BundleHandle & bundle)
{
  auto last_heartbeat_time = std::chrono::steady_clock::now();
  std::stringstream ss;
  ss << "Received " << bundle.getName() << " at "
     << std::chrono::duration_cast<std::chrono::seconds>(last_heartbeat_time.time_since_epoch())
          .count()
     << " seconds";
  ss << " count: " << bundle.getSignal("heartbeat").getValue<uint32_t>();
  logs.push_back(ss.str());
}

int main()
{
  node = std::make_unique<proton::Node>(CONFIG_FILE, "node1");

  node->registerCallback("node_name", print_callback);
  node->registerCallback("node2_heartbeat", heartbeat_callback);
  node->registerCallback("node3_heartbeat", heartbeat_callback);

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
