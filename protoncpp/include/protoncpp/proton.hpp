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
#include "protoncpp/manager.hpp"
#include "protoncpp/transport/udp4.hpp"


namespace proton {

class Node : public BundleManager
{
public:
  Node();
  Node(const std::string config_file);

  Config getConfig() {return config_;}

private:
  Config config_;
  Transport transport_;
};

}

#endif // INC_PROTONCPP_PROTON_HPP_
