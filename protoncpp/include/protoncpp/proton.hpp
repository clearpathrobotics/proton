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
#include <thread>
#include <vector>

#include "protoncpp/status.hpp"
#include "protoncpp/bundle.pb.h"
#include "protoncpp/bundle_manager.hpp"
#include "protoncpp/config.hpp"
#include "protoncpp/transport/serial.hpp"
#include "protoncpp/transport/udp4.hpp"

namespace proton {

static constexpr size_t PROTON_MAX_MESSAGE_SIZE = UINT16_MAX;

class Node : public BundleManager, public TransportManager {
public:
  enum State {
    UNCONFIGURED,
    INACTIVE,
    ACTIVE,
    SHUTDOWN
  };

  Node();
  Node(const std::string config_file, const std::string target, bool auto_configure = true, bool auto_activate = true);

  Status configure();
  Status activate();
  Status spinOnce();
  Status spin();

  Config getConfig() { return config_; }

  Status pollForBundle();
  Status sendBundle(const std::string &bundle_name);
  Status sendBundle(BundleHandle &bundle_handle);

  double getRxKbps() { return rx_kbps_; }
  double getTxKbps() { return tx_kbps_; }

  bool registerCallback(const std::string &bundle_name,
                        BundleHandle::BundleCallback callback);

  std::string getTarget() { return target_node_config_.name; }
  NodeConfig getNode() { return target_node_config_; }

  void startStatsThread();
  void printStats();

private:
  void runStatsThread();

  Config config_;
  NodeConfig target_node_config_;
  std::string target_;
  State state_;

  // Stats
  uint64_t rx_, tx_;
  double rx_kbps_, tx_kbps_;
  std::thread stats_thread_;
};

} // namespace proton

#endif // INC_PROTONCPP_PROTON_HPP_
