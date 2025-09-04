#include "protoncpp/proton.hpp"
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <chrono>

proton::Node node;

typedef enum {
  CALLBACK_LOGGER,
  CALLBACK_STATUS,
  CALLBACK_POWER,
  CALLBACK_ESTOP,
  CALLBACK_TEMPERATURE,
  CALLBACK_STOP_STATUS,
  CALLBACK_PINOUT_STATE,
  CALLBACK_ALERTS,
  CALLBACK_COUNT
} callback_e;

uint32_t cb_counts[CALLBACK_COUNT];
std::vector<std::string> logs;

void update_lights()
{
  auto& cmd_lights_bundle = node.getBundle("cmd_lights");
  cmd_lights_bundle.getSignal("front_left_light").setValue<proton::bytes>({rand() % 255, rand() % 255, rand() % 255});
  cmd_lights_bundle.getSignal("front_right_light").setValue<proton::bytes>({rand() % 255, rand() % 255, rand() % 255});
  cmd_lights_bundle.getSignal("rear_left_light").setValue<proton::bytes>({rand() % 255, rand() % 255, rand() % 255});
  cmd_lights_bundle.getSignal("rear_right_light").setValue<proton::bytes>({rand() % 255, rand() % 255, rand() % 255});

  node.sendBundle("cmd_lights");
}

void update_fans()
{
  node.getBundle("cmd_fans").getSignal("fan_speeds").setValue<proton::bytes>({rand() % 255, rand() % 255, rand() % 255, rand() % 255, rand() % 255, rand() % 255, rand() % 255, rand() % 255});
  node.sendBundle("cmd_fans");
}

void update_display_status()
{
  node.getBundle("display_status").getSignal("string_1").setValue<std::string>("TEST_STRING_1");
  node.getBundle("display_status").getSignal("string_2").setValue<std::string>("TEST_STRING_1");
  node.sendBundle("display_status");
}

void update_battery()
{
  node.getBundle("battery").getSignal("percentage").setValue<float>(static_cast<float>(rand()));
  node.sendBundle("battery");
}

void update_pinout_command()
{
  node.getBundle("pinout_command").getSignal("rails").setValue<proton::list_bool>({true});
  node.getBundle("pinout_command").getSignal("outputs").setValue<proton::list_uint32>({true});
  node.sendBundle("pinout_command");
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
    std::cout<< u8"\033[2J\033[1;1H";
    std::cout << "-------- A300 PC CPP --------" << std::endl;
    std::cout << "Rx: " << static_cast<double>(node.getRx()) / 1000 << " KB/s " << "Tx: " << static_cast<double>(node.getTx()) / 1000 << " KB/s" << std::endl;
    std::cout << "--- Received Bundles (hz) ---" << std::endl;
    std::cout << "log: " << cb_counts[CALLBACK_LOGGER] << std::endl;
    std::cout << "status: " << cb_counts[CALLBACK_STATUS] << std::endl;
    std::cout << "power: " << cb_counts[CALLBACK_POWER] << std::endl;
    std::cout << "emergency_stop: " << cb_counts[CALLBACK_ESTOP] << std::endl;
    std::cout << "temperature: " << cb_counts[CALLBACK_TEMPERATURE] << std::endl;
    std::cout << "stop_status: " << cb_counts[CALLBACK_STOP_STATUS] << std::endl;
    std::cout << "pinout_state: " << cb_counts[CALLBACK_PINOUT_STATE] << std::endl;
    std::cout << "alerts: " << cb_counts[CALLBACK_ALERTS] << std::endl;
    std::cout << "----------- Logs ------------" << std::endl;

    for (auto & l : logs)
    {
      std::cout << l << std::endl;
    }

    std::cout << "-----------------------------" << std::endl;

    logs.clear();

    node.resetRx();
    node.resetTx();
    for (int i = 0; i < CALLBACK_COUNT; i++)
    {
      cb_counts[i] = 0;
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

void logger_callback(proton::BundleHandle& bundle)
{
  cb_counts[CALLBACK_LOGGER]++;

  logs.push_back(bundle.getSignal("msg").getValue<std::string>());
}

void status_callback(proton::BundleHandle& bundle)
{
  cb_counts[CALLBACK_STATUS]++;
}

void power_callback(proton::BundleHandle& bundle)
{
  cb_counts[CALLBACK_POWER]++;
}

void estop_callback(proton::BundleHandle& bundle)
{
  cb_counts[CALLBACK_ESTOP]++;
}

void temperature_callback(proton::BundleHandle& bundle)
{
  cb_counts[CALLBACK_TEMPERATURE]++;
}

void stop_status_callback(proton::BundleHandle& bundle)
{
  cb_counts[CALLBACK_STOP_STATUS]++;
}

void pinout_state_callback(proton::BundleHandle& bundle)
{
  cb_counts[CALLBACK_PINOUT_STATE]++;
}

void alerts_callback(proton::BundleHandle& bundle)
{
  cb_counts[CALLBACK_ALERTS]++;
}

int main()
{
  node = proton::Node(CONFIG_FILE, "pc");

  node.registerCallback("log", logger_callback);
  node.registerCallback("status", status_callback);
  node.registerCallback("power", power_callback);
  node.registerCallback("emergency_stop", estop_callback);
  node.registerCallback("temperature", temperature_callback);
  node.registerCallback("stop_status", stop_status_callback);
  node.registerCallback("pinout_state", pinout_state_callback);
  node.registerCallback("alerts", alerts_callback);

  std::thread stats_thread(run_stats_thread);
  std::thread send_1hz_thread(run_1hz_thread);
  std::thread send_20hz_thread(run_20hz_thread);

  node.spin();

  stats_thread.join();
  send_1hz_thread.join();
  send_20hz_thread.join();

  return 0;
}

