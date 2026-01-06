#include "protoncpp/proton.hpp"
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <chrono>

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
  node->getBundle("motor_command").getSignal("drivers").setValue<proton::list_float>({static_cast<float>(rand()), static_cast<float>(rand())});
  node->sendBundle("motor_command");
}

void run_1hz_thread()
{
  while(1)
  {
    update_wifi_connected();
    update_hmi();
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

void run_50hz_thread()
{
  while(1)
  {
    update_motor_command();
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
  node = std::make_unique<proton::Node>(CONFIG_FILE, "pc");

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

