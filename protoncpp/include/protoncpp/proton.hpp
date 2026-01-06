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

#include "proton/common.h"
#include "protoncpp/bundle.pb.h"
#include "protoncpp/bundle_manager.hpp"
#include "protoncpp/config.hpp"
#include "protoncpp/safe_queue.hpp"
#include "protoncpp/transport/serial.hpp"
#include "protoncpp/transport/udp4.hpp"

namespace proton {

  inline std::ostream& operator<<(std::ostream& os, proton_transport_state_e state) {
  switch(state)
  {
    case PROTON_TRANSPORT_DISCONNECTED: return os << "Disconnected";
    case PROTON_TRANSPORT_CONNECTED: return os << "Connected";
    case PROTON_TRANSPORT_ERROR: return os << "Error";
    default: return os << "UNKNOWN";
  }
}

inline std::ostream& operator<<(std::ostream& os, proton_node_state_e state) {
  switch(state)
  {
    case PROTON_NODE_UNCONFIGURED: return os << "Unconfigured";
    case PROTON_NODE_INACTIVE: return os << "Inactive";
    case PROTON_NODE_ACTIVE: return os << "Active";
    default: return os << "UNKNOWN";
  }
}

inline std::ostream& operator<<(std::ostream& os, proton_status_e state) {
  switch(state)
  {
    case PROTON_OK: return os << "OK";
    case PROTON_ERROR: return os << "Error";
    case PROTON_NULL_PTR_ERROR: return os << "Null Pointer";
    case PROTON_INVALID_STATE_ERROR: return os << "Invalid State";
    case PROTON_INVALID_STATE_TRANSITION_ERROR: return os << "Invalid State Transition";
    case PROTON_SERIALIZATION_ERROR: return os << "Serialization Error";
    case PROTON_CONNECT_ERROR: return os << "Connect Error";
    case PROTON_DISCONNECT_ERROR: return os << "Disconnect Error";
    case PROTON_READ_ERROR: return os << "Read Error";
    case PROTON_WRITE_ERROR: return os << "Write Error";
    case PROTON_INVALID_HEADER_ERROR: return os << "Invalid Header Error";
    case PROTON_CRC16_ERROR: return os << "CRC16 Error";
    case PROTON_INSUFFICIENT_BUFFER_ERROR: return os << "Insufficient Buffer";
    default: return os << "UNKNOWN";
  }
}

class Connection : public TransportManager {
public:
  using ReadCompleteCallback = std::function<proton_status_e(Bundle&, const std::string& producer)>;

  Connection();
  Connection(const NodeConfig& node_config,
             const NodeConfig& peer_config,
             const ConnectionConfig& connection_config,
             ReadCompleteCallback callback);

  void run();
  void heartbeat();
  NodeConfig getConfig() { return config_; }
  proton_node_state_e getNodeState() { return state_; }
  std::string getTransportType() { return transport_type_; }

  double getRxKbps() { return rx_kbps_; }
  double getTxKbps() { return tx_kbps_; }

private:
  void spin();
  void checkHeartbeat();
  proton_status_e pollForBundle();

  NodeConfig config_;
  std::string transport_type_;
  ReadCompleteCallback callback_;
  std::thread run_thread_, heartbeat_thread_;
  proton_node_state_e state_;
  int64_t last_heartbeat_ms_;
  double rx_kbps_, tx_kbps_;
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

  proton_status_e configure();
  proton_status_e activate();
  proton_status_e spinOnce();
  proton_status_e spin();

  proton_status_e waitForBundle();
  proton_status_e readCompleteCallback(Bundle& bundle, const std::string& producer);

  proton_status_e sendBundle(const std::string &bundle_name);
  proton_status_e sendBundle(BundleHandle &bundle_handle);
  proton_status_e sendHeartbeat();

  proton_status_e registerCallback(const std::string &bundle_name,
                          BundleHandle::BundleCallback callback);
  proton_status_e registerHeartbeatCallback(const std::string &producer,
                                         BundleHandle::BundleCallback callback);

  std::string getName() const { return name_; }
  NodeConfig getNodeConfig() const { return node_config_; }
  Config& getConfig() { return config_; }
  std::map<std::string, Connection>& getConnections() { return connections_; }

  void startStatsThread();
  void printStats();

private:
  void runStatsThread();
  void runHeartbeatThread();

  Config config_;
  NodeConfig node_config_;
  std::string name_;
  proton_node_state_e state_;
  std::map<std::string, Connection> connections_;
  std::vector<std::string> peers_;
  SafeQueue<ReceivedBundle> read_queue_;

  // Stats
  std::thread stats_thread_, heartbeat_thread_;
};

} // namespace proton

#endif // INC_PROTONCPP_PROTON_HPP_
