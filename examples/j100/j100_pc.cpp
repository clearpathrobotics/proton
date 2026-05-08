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

#include <stdlib.h>
#include <string.h>
#include <chrono>
#include <iostream>
#include <thread>
#include "protoncpp/proton.hpp"

std::unique_ptr<proton::Node> node;

std::vector<std::string> logs;

void update_wifi_connected()
{
  node->getBundle("wifi_connected").getSignal("data").setValue<bool>(static_cast<bool>(rand() % 2));
  node->sendBundle("wifi_connected");
}

void update_hmi()
{
  node->getBundle("hmi").getSignal("data").setValue<uint32_t>(4);
  node->sendBundle("hmi");
}

void update_motor_command()
{
  node->getBundle("motor_command").getSignal("mode").setValue<int32_t>(-1);
  node->getBundle("motor_command")
    .getSignal("left_driver")
    .setValue<float>(static_cast<float>(rand()));
  node->getBundle("motor_command")
    .getSignal("right_driver")
    .setValue<float>(static_cast<float>(rand()));
  node->sendBundle("motor_command");
}

/**
 * @brief Updates the PC heartbeat signal.
 */
void update_pc_heartbeat(uint32_t heartbeat_count)
{
  node->getBundle("pc_heartbeat").getSignal("heartbeat").setValue<uint32_t>(heartbeat_count);
  node->sendBundle("pc_heartbeat");
}

void run_1hz_thread()
{
  while (1)
  {
    update_wifi_connected();
    update_hmi();
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

/**
 * @brief Runs the 10 Hz update thread for heartbeats
 */
void run_10hz_thread()
{
  uint32_t heartbeat_count = 0;
  while (1)
  {
    update_pc_heartbeat(heartbeat_count);
    heartbeat_count++;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

void run_50hz_thread()
{
  while (1)
  {
    update_motor_command();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
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

void logger_callback(proton::BundleHandle & bundle)
{
  logs.push_back(bundle.getSignal("msg").getValue<std::string>());
}

/**
 * @brief Callback function to handle heartbeat bundle from mcu peer
 *
 * @param bundle the bundle containing the heartbeat signal
 */
void mcu_heartbeat_callback(proton::BundleHandle & bundle)
{
  auto last_heartbeat_time = std::chrono::steady_clock::now();
  std::stringstream ss;
  ss << "Received MCU Heartbeat at "
     << std::chrono::duration_cast<std::chrono::seconds>(last_heartbeat_time.time_since_epoch())
          .count()
     << " seconds";
  ss << " count: " << bundle.getSignal("heartbeat").getValue<uint32_t>();
  logs.push_back(ss.str());
}

void print_callback(proton::BundleHandle & bundle) { bundle.printBundleVerbose(); }

int main()
{
  node = std::make_unique<proton::Node>(CONFIG_FILE, "pc");

  node->registerCallback("log", logger_callback);
  node->registerCallback("mcu_heartbeat", mcu_heartbeat_callback);

  std::thread stats_thread(run_stats_thread);
  std::thread send_1hz_thread(run_1hz_thread);
  std::thread send_10hz_thread(run_10hz_thread);
  std::thread send_50hz_thread(run_50hz_thread);

  node->startStatsThread();

  node->spin();

  stats_thread.join();
  send_1hz_thread.join();
  send_10hz_thread.join();
  send_50hz_thread.join();

  return 0;
}
