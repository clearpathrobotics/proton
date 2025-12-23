

/**
 * @file a300_pc.cpp
 * @brief A300 PC node controller for managing lights, fans, display, battery, and pinout commands.
 *
 * This program implements a Proton node that communicates with an A300 MCU.
 * It manages multiple subsystems through periodic updates at different frequencies:
 * - 20 Hz: Light updates with random RGB values
 * - 1 Hz: Fan speeds, display strings, battery percentage, and pinout rail/output configurations
 *
 * The program runs three concurrent threads:
 * - run_20hz_thread: Updates light commands at 20 Hz
 * - run_1hz_thread: Updates fans, display, battery, and pinout at 1 Hz
 * - run_stats_thread: Periodically prints node statistics and collected log messages
 *
 * All updates are communicated through the Proton messaging system using named bundles.
 * Log messages from the robot are collected via callback and displayed in the stats thread.
 *
 * @see proton::Node
 * @see proton::BundleHandle
 */

#include "protoncpp/proton.hpp"
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <chrono>

std::unique_ptr<proton::Node> node;

std::vector<std::string> logs;

/**
 * @brief Updates the light commands with random RGB values.
 *
 */
void update_lights()
{
  auto& cmd_lights_signal = node->getBundle("cmd_lights").getSignal("lights");
  for (auto i = 0; i < cmd_lights_signal.getLength(); i++)
  {
    cmd_lights_signal.setValue<proton::bytes>(i, {rand() % 255, rand() % 255, rand() % 255});
  }

  node->sendBundle("cmd_lights");
}

/**
 * @brief Updates the fan commands with random speed values.
 *
 */
void update_fans()
{
  node->getBundle("cmd_fans").getSignal("fans").setValue<proton::bytes>({rand() % 255, rand() % 255, rand() % 255, rand() % 255, rand() % 255, rand() % 255, rand() % 255, rand() % 255});
  node->sendBundle("cmd_fans");
}

/**
 * @brief Updates the display status strings.
 *
 */
void update_display_status()
{
  node->getBundle("display_status").getSignal("string1").setValue<std::string>("TEST_STRING_1");
  node->getBundle("display_status").getSignal("string2").setValue<std::string>("TEST_STRING_1");
  node->sendBundle("display_status");
}

/**
 * @brief Updates the battery percentage with a random value.
 *
 */
void update_battery()
{
  node->getBundle("battery").getSignal("percentage").setValue<float>(static_cast<float>(rand()));
  node->sendBundle("battery");
}

/**
 * @brief Updates the pinout command signals.
 *
 */
void update_pinout_command()
{
  node->getBundle("pinout_command").getSignal("rails").setValue<proton::list_bool>({true});
  node->getBundle("pinout_command").getSignal("outputs").setValue<proton::list_uint32>({true});
  node->sendBundle("pinout_command");
}

/**
 * @brief Runs the 1 Hz update thread for fans, display, battery, and pinout commands.
 *
 */
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

/**
 * @brief Runs the 20 Hz update thread for light commands.
 *
 */
void run_20hz_thread()
{
  while(1)
  {
    update_lights();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }
}

/**
 * @brief Runs the stats thread that prints node statistics and collected log messages.
 *
 */
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

/**
 * @brief Callback function to handle log messages from bundles.
 *
 * @param bundle The bundle containing the log message.
 */
void logger_callback(proton::BundleHandle& bundle)
{
  logs.push_back(bundle.getSignal("msg").getValue<std::string>());
}

/**
 * @brief Main function that initializes the node, registers callbacks, starts threads, and runs the node.
 *
 * @return int
 */
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
