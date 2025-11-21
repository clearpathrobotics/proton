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

#include <mutex>
#include <stdint.h>
#include <thread>
#include <vector>

#include "protoncpp/bundle.pb.h"
#include "protoncpp/bundle_manager.hpp"
#include "protoncpp/config.hpp"
#include "protoncpp/common.hpp"
#include "protoncpp/transport/serial.hpp"
#include "protoncpp/transport/udp4.hpp"

namespace proton {

static constexpr size_t PROTON_MAX_MESSAGE_SIZE = UINT16_MAX;


class Peer : public TransportManager {
public:
  using ReadCompleteCallback = std::function<Status(Bundle&, const std::string& producer)>;

  Peer();
  Peer(const NodeConfig& node_config, const NodeConfig& peer_config, ReadCompleteCallback callback);

  void run();
  void heartbeat();
  NodeConfig getConfig() { return config_; }
  NodeState getNodeState() { return state_; }

private:
  void spin();
  void checkHeartbeat();
  Status pollForBundle();

  NodeConfig config_;
  ReadCompleteCallback callback_;
  std::thread run_thread_, heartbeat_thread_;
  NodeState state_;
  int64_t last_heartbeat_ms_;
};

class Node : public BundleManager {
public:
  struct ReceivedBundle{
    Bundle bundle;
    std::string producer;
  };

  Node();
  Node(const std::string config_file, const std::string target,
       bool auto_configure = true, bool auto_activate = true);

  Status configure();
  Status activate();
  Status spinOnce();
  Status spin();

  Status waitForBundle();
  Status readCompleteCallback(Bundle& bundle, const std::string& producer);

  Status sendBundle(const std::string &bundle_name);
  Status sendBundle(BundleHandle &bundle_handle);
  Status sendHeartbeat();

  double getRxKbps() { return rx_kbps_; }
  double getTxKbps() { return tx_kbps_; }

  Status registerCallback(const std::string &bundle_name,
                          BundleHandle::BundleCallback callback);
  Status registerHeartbeatCallback(const std::string &producer,
                                         BundleHandle::BundleCallback callback);

  std::string getName() const { return name_; }
  NodeConfig getNodeConfig() const { return node_config_; }
  Config& getConfig() { return config_; }

  void startStatsThread();
  void printStats();

private:
  void runStatsThread();
  void runHeartbeatThread();

  Config config_;
  NodeConfig node_config_;
  std::string name_;
  NodeState state_;
  std::map<std::string, Peer> peers_;
  SafeQueue<ReceivedBundle> read_queue_;

  // Stats
  uint64_t rx_, tx_;
  double rx_kbps_, tx_kbps_;
  std::thread stats_thread_, heartbeat_thread_;
};

} // namespace proton

#endif // INC_PROTONCPP_PROTON_HPP_
