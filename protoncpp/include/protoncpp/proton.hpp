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

#include <stdint.h>
#include <vector>

#include "protoncpp/bundle.pb.h"
#include "protoncpp/bundle_manager.hpp"
#include "protoncpp/config.hpp"
#include "protoncpp/transport/udp4.hpp"

namespace proton {

class Node : public BundleManager, public TransportManager {
public:
  Node();
  Node(const std::string config_file, const std::string target);

  Config getConfig() { return config_; }

  void spinOnce();
  void spin();

private:
  std::string target_;
  Config config_;
};

} // namespace proton

#endif // INC_PROTONCPP_PROTON_HPP_
