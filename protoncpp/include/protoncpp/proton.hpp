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
#include <thread>

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

  void sendBundle(const std::string &bundle_name);

  uint64_t getRx() { return rx_; }
  uint64_t getTx() { return tx_; }

  void resetRx() { rx_ = 0; }
  void resetTx() { tx_ = 0; }

  bool registerCallback(const std::string &bundle_name, BundleHandle::BundleCallback callback);

private:
  Config config_;
  std::string target_;
  uint64_t rx_, tx_;
};

} // namespace proton

#endif // INC_PROTONCPP_PROTON_HPP_
