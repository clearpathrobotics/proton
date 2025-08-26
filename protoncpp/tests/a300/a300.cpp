#include "protoncpp/proton.hpp"
#include <iostream>

#define PROTON_MAX_MESSAGE_SIZE 1024

uint8_t write_buf_[PROTON_MAX_MESSAGE_SIZE];
uint8_t read_buf_[PROTON_MAX_MESSAGE_SIZE];

int main()
{
  printf("~~~~~~~ A300 node ~~~~~~~\r\n");

  proton::node::Node node = proton::node::Node("/home/rkreinin/proto_ws/src/proton/protoncpp/tests/a300/config/a300.yaml");

  proton::node::Bundle b = node.getBundle("logger");

  std::cout << b.getMessage().DebugString() << std::endl;

  proton::node::Signal s = b.getSignal("name");

  std::cout << s.getCapacity() << std::endl;

  return 0;
}

