/**
 * Software License Agreement (proprietary)
 *
 * @copyright Copyright (c) 2025 Clearpath Robotics, Inc., All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, is not permitted without the express permission of Clearpath
 * Robotics.
 *
 * @author Roni Kreinin (rkreinin@clearpathrobotics.com)
 */

#ifndef INC_PROTONCPP_PROTON_HPP_
#define INC_PROTONCPP_PROTON_HPP_

#include <vector>
#include <stdint.h>

#include "protoncpp/bundle.pb.h"
#include "protoncpp/config.hpp"


namespace proton {

namespace node {

class Signal
{
public:
  Signal(proton::config::SignalConfig config, proton::Signal * message);

  std::string getName();
  proton::Signal::SignalCase getType();
  uint32_t getLength();
  uint32_t getCapacity();

private:
  void initSignal();

  proton::config::SignalConfig config_;
  proton::Signal * message_;
};

class Bundle
{
public:
  Bundle(proton::config::BundleConfig config);

  std::string getName();
  uint32_t getID();
  std::string getProducer();
  std::string getConsumer();
  proton::node::Signal getSignal(std::string name);
  proton::Bundle getMessage();

private:
  proton::config::BundleConfig config_;
  proton::Bundle message_;
  std::map<std::string, proton::node::Signal> signals_;
};

class Node
{
public:
  Node(const std::string config_file);

  config::Config getConfig() {return config_;}
  proton::node::Bundle &getBundle(std::string name);

private:
  config::Config config_;
  std::map<std::string, proton::node::Bundle> bundles_;
};

}

}




#endif // INC_PROTONCPP_PROTON_HPP_
