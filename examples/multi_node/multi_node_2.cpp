#include "protoncpp/proton.hpp"
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <chrono>

std::unique_ptr<proton::Node> node;

std::vector<std::string> logs;


void run_1hz_thread()
{
  while(1)
  {
    node->getBundle("node_name").getSignal("name").setValue<std::string>(node->getName());
    node->sendBundle("node_name");
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

void run_50hz_thread()
{
  auto& time_bundle = node->getBundle("time");
  while(1)
  {
    time_bundle.getSignal("seconds").setValue<int32_t>(std::time(NULL));
    node->sendBundle(time_bundle);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
  }
}

void run_stats_thread()
{
  while(1)
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

void logger_callback(proton::BundleHandle& bundle)
{
  logs.push_back(bundle.getSignal("msg").getValue<std::string>());
}

void print_callback(proton::BundleHandle& bundle)
{
  bundle.printBundleVerbose();
}

int main()
{
  node = std::make_unique<proton::Node>(CONFIG_FILE, "node2");

  node->registerCallback("log", logger_callback);

  std::thread stats_thread(run_stats_thread);
  std::thread send_1hz_thread(run_1hz_thread);
  std::thread send_50hz_thread(run_50hz_thread);

  node->startStatsThread();

  node->spin();

  stats_thread.join();
  send_1hz_thread.join();
  send_50hz_thread.join();

  return 0;
}

