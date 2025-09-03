#include "protoncpp/proton.hpp"
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <chrono>

proton::Node node;


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
  //node.printAllBundles();
  node.sendBundle("cmd_fans");
}

void run_send_thread()
{
  uint32_t i = 0;
  while(1)
  {
    if (i % 1000 == 0)
    {
      update_fans();
    }

    if (i % 100 == 0)
    {

    }

    if (i % 50 == 0)
    {
      update_lights();
    }

    if (i % 20 == 0)
    {

    }

    i++;
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}

void run_stats_thread()
{
  while(1)
  {
    std::cout << "Rx: " << static_cast<double>(node.getRx()) / 1000 << " KB/s " << "Tx: " << static_cast<double>(node.getTx()) / 1000 << " KB/s" << std::endl;
    node.resetRx();
    node.resetTx();
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}


int main()
{
  printf("~~~~~~~ A300 node ~~~~~~~\r\n");

  node = proton::Node("/home/rkreinin/proto_ws/src/proton/protoncpp/tests/a300/config/a300.yaml", "pc");

  std::thread stats_thread(run_stats_thread);
  std::thread send_thread(run_send_thread);

  node.spin();

  return 0;
}

