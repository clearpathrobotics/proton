#include "protoncpp/proton.hpp"
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <chrono>

std::unique_ptr<proton::Node> node;

std::vector<std::string> logs;

void update_lights()
{
  auto& cmd_lights_signal = node->getBundle("cmd_lights").getSignal("lights");
  for (auto i = 0; i < cmd_lights_signal.getLength(); i++)
  {
    cmd_lights_signal.setValue<proton::bytes>(i, {rand() % 255, rand() % 255, rand() % 255});
  }

  node->sendBundle("cmd_lights");
}

void update_fans()
{
  node->getBundle("cmd_fans").getSignal("fans").setValue<proton::bytes>({rand() % 255, rand() % 255, rand() % 255, rand() % 255, rand() % 255, rand() % 255, rand() % 255, rand() % 255});
  node->sendBundle("cmd_fans");
}

void update_display_status()
{
  node->getBundle("display_status").getSignal("string1").setValue<std::string>("TEST_STRING_1");
  node->getBundle("display_status").getSignal("string2").setValue<std::string>("TEST_STRING_1");
  node->sendBundle("display_status");
}

void update_battery()
{
  node->getBundle("battery").getSignal("percentage").setValue<float>(static_cast<float>(rand()));
  node->sendBundle("battery");
}

void update_pinout_command()
{
  node->getBundle("pinout_command").getSignal("rails").setValue<proton::list_bool>({true});
  node->getBundle("pinout_command").getSignal("outputs").setValue<proton::list_uint32>({true});
  node->sendBundle("pinout_command");
}

void run_1hz_thread()
{
  while(1)
  {
    update_fans();
    update_display_status();
    update_battery();
    update_pinout_command();
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

void run_20hz_thread()
{
  while(1)
  {
    update_lights();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
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

int main()
{
  node = std::make_unique<proton::Node>(CONFIG_FILE, "pc");

  node->registerCallback("log", logger_callback);

  std::thread stats_thread(run_stats_thread);
  std::thread send_1hz_thread(run_1hz_thread);
  std::thread send_20hz_thread(run_20hz_thread);

  node->startStatsThread();
  node->spin();

  stats_thread.join();
  send_1hz_thread.join();
  send_20hz_thread.join();

  return 0;
}

