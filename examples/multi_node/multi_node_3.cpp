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
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include "protoncpp/proton.hpp"

std::unique_ptr<proton::Node> node;

std::vector<std::string> logs;

void send_heartbeat(uint32_t count, const std::string & node_name)
{
  auto & heartbeat_signal = node->getBundle(node_name).getSignal("heartbeat");
  heartbeat_signal.setValue<uint32_t>(count);
  node->sendBundle(node_name);
}

void run_1hz_thread()
{
  while (1)
  {
    node->getBundle("node_name").getSignal("name").setValue<std::string>(node->getName());
    node->sendBundle("node_name");
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

void run_10hz_thread()
{
  uint32_t heartbeat = 0;
  while (1)
  {
    send_heartbeat(heartbeat++, "node3_heartbeat");
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

void logger_callback(proton::BundleHandle & bundle)
{
  logs.push_back(bundle.getSignal("msg").getValue<std::string>());
}

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
  node = std::make_unique<proton::Node>(CONFIG_FILE, "node3");

  std::cout << "Init" << std::endl;

  node->registerCallback("log", logger_callback);
  node->registerCallback("node1_heartbeat", heartbeat_callback);
  node->registerCallback("node2_heartbeat", heartbeat_callback);

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
