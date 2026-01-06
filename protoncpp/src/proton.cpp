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

#include "protoncpp/proton.hpp"
#include "protoncpp/config.hpp"
#include <iostream>
#include <chrono>


using namespace proton;

Node::Node() {}

Node::Node(const std::string config_file, const std::string name, bool auto_configure, bool auto_activate)
    : config_(config_file), name_(name), state_(PROTON_NODE_UNCONFIGURED)
{
  proton_status_e status;

  if (!auto_configure && auto_activate)
  {
    throw std::runtime_error("Auto activate cannot be enabled without auto configure");
  }

  if (auto_configure)
  {
    status = configure();

    if (status != PROTON_OK)
    {
      throw std::runtime_error("Configuration error: " + std::to_string(status));
    }

    if (auto_activate)
    {
      status = activate();

      if (status != PROTON_OK)
      {
        throw std::runtime_error("Activation error: " + std::to_string(status));
      }
    }
  }
}

proton_status_e Node::configure()
{
  if (state_ != PROTON_NODE_UNCONFIGURED)
  {
    return PROTON_INVALID_STATE_TRANSITION_ERROR;
  }

  for (auto b : config_.getBundles()) {
    addBundle(b);
  }

  // Get config for this node
  node_config_ = config_.getNodes().at(name_);

  // Find all peers
  for (auto& connection_config: config_.getConnections()) {
    std::string peer;
    // Only look at connections involving this node
    if (connection_config.connection.first.node == name_ )
    {
      peer = connection_config.connection.second.node;
    }
    else if (connection_config.connection.second.node == name_ )
    {
      peer = connection_config.connection.first.node;
    }
    else
    {
      continue;
    }

    peers_.push_back(peer);

    connections_.emplace(
      peer,
      Connection(
        node_config_,
        config_.getNodes().at(peer),
        connection_config,
        std::bind(&Node::readCompleteCallback, this, std::placeholders::_1, std::placeholders::_2)
        )
    );
  }

  if (node_config_.heartbeat.enabled)
  {
    // Add a heartbeat bundle to send to each peer
    addHeartbeat(name_, peers_);
  }

  for (auto& [peer_name, peer] : connections_)
  {
    // Add a heartbeat bundle to receive from each peer
    if (peer.getConfig().heartbeat.enabled)
    {
      addHeartbeat(peer_name, {name_});
    }
  }

  state_ = PROTON_NODE_INACTIVE;
  return PROTON_OK;
}

proton_status_e Node::activate()
{
  if (state_ != PROTON_NODE_INACTIVE)
  {
    return PROTON_INVALID_STATE_TRANSITION_ERROR;
  }

  for (auto& [name, peer]: connections_)
  {
    peer.run();
  }

  // Start heartbeat thread
  if (node_config_.heartbeat.enabled)
  {
    heartbeat_thread_ = std::thread(&Node::runHeartbeatThread, this);
  }

  state_ = PROTON_NODE_ACTIVE;
  return PROTON_OK;
}

void Node::startStatsThread()
{
  stats_thread_ = std::thread(&Node::runStatsThread, this);
}

proton_status_e Node::sendBundle(const std::string &bundle_name) {
  return sendBundle(getBundle(bundle_name));
}

proton_status_e Node::sendBundle(BundleHandle &bundle_handle) {
  proton_status_e status = PROTON_OK;
  auto bundle = bundle_handle.getBundlePtr().get();
  auto buf = std::vector<uint8_t>(bundle->ByteSizeLong());

  if (bundle->SerializeToArray(buf.data(), bundle->ByteSizeLong()))
  {
    size_t bytes_written = 0;

    // Send bundle to each consumer
    for (const auto& consumer: bundle_handle.getConsumers())
    {
      auto& connection = connections_.at(consumer);

      // Make sure consumer connection is active
      if (!connection.connected())
      {
        continue;
      }

      status = connection.write(buf.data(), bundle->ByteSizeLong(), bytes_written);
    }

    if (status == PROTON_OK)
    {
      bundle_handle.incrementTxCount();
    }
  }

  return PROTON_SERIALIZATION_ERROR;
}

proton_status_e Node::sendHeartbeat()
{
  if (!node_config_.heartbeat.enabled)
  {
    return PROTON_ERROR;
  }

  auto& heartbeat = getHeartbeat(name_);
  auto& signal = heartbeat.getSignal("heartbeat");
  // Increment heartbeat
  signal.setValue<uint32_t>(signal.getValue<uint32_t>() + 1);
  return sendBundle(heartbeat);
}

proton_status_e Node::registerCallback(const std::string &bundle_name, BundleHandle::BundleCallback callback)
{
  auto& bundle = getBundle(bundle_name);

  if (callback != nullptr)
  {
    for (const auto& consumer: bundle.getConsumers())
    {
      if (consumer == name_)
      {
        bundle.registerCallback(callback);
        return PROTON_OK;
      }
    }
  }

  return PROTON_ERROR;
}

proton_status_e Node::registerHeartbeatCallback(const std::string &producer, BundleHandle::BundleCallback callback)
{
  auto& bundle = getHeartbeat(producer);

  if (callback != nullptr)
  {
    for (const auto& consumer: bundle.getConsumers())
    {
      if (consumer == name_)
      {
        bundle.registerCallback(callback);
        return PROTON_OK;
      }
    }
  }

  return PROTON_ERROR;
}

proton_status_e Node::readCompleteCallback(Bundle& bundle, const std::string& producer)
{
  read_queue_.push({bundle, producer});
  return PROTON_OK;
}

proton_status_e Node::waitForBundle()
{
  auto received_bundle = read_queue_.pop();

  auto ret = updateBundle(received_bundle.bundle, received_bundle.producer);
  if (ret)
  {
    auto& handle = getBundle(ret.value());
    handle.incrementRxCount();

    auto callback = handle.getCallback();

    if (callback)
    {
      callback(handle);
    }

    return PROTON_OK;
  }

  return PROTON_ERROR;
}

proton_status_e Node::spinOnce() {
  if (state_ != PROTON_NODE_ACTIVE) {
    return PROTON_INVALID_STATE_ERROR;
  }

  return waitForBundle();
}

proton_status_e Node::spin() {
  proton_status_e status;
  while (1) {
    status = spinOnce();

    if (status != PROTON_OK)
    {
      return status;
    }
  }
}

void Node::runStatsThread()
{
  while(1)
  {
    if (bundles_.size() > 0)
    {
      for (auto& [name, handle] : bundles_)
      {
        handle.setRxps(handle.getRxCount());
        handle.setTxps(handle.getTxCount());
        handle.resetRxCount();
        handle.resetTxCount();
      }

      for (auto& [name, handle] : heartbeat_bundles_)
      {
        handle.setRxps(handle.getRxCount());
        handle.setTxps(handle.getTxCount());
        handle.resetRxCount();
        handle.resetTxCount();
      }
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

void Node::runHeartbeatThread()
{
  while(1)
  {
    sendHeartbeat();
    std::this_thread::sleep_for(std::chrono::milliseconds(node_config_.heartbeat.period));
  }
}

void Node::printStats()
{
  std::cout<< u8"\033[2J\033[1;1H";
  std::cout << "-------- Proton CPP Node --------" << std::endl;
  std::cout << "Config: " << config_.getName() << std::endl;
  std::cout << "Node: " << name_ << std::endl;
  std::cout << "  State: " << state_ << std::endl;
  std::cout << "Connections: " << std::endl;
  for (auto& [name, peer]: connections_)
  {
    std::cout << "  " << name << ":" << std::endl;
    std::cout << "    Heartbeat: " << peer.getNodeState() << std::endl;
    std::cout << "    Transport (" << peer.getTransportType() << "): " << peer.getTransportState() << std::endl;
    std::cout << "    Rx: " << peer.getRxKbps() << " KB/s " << "Tx: " << peer.getTxKbps() << " KB/s" << std::endl;
  }

  std::cout << "----- Produced Bundles (hz) -----" << std::endl;
  for (auto& [name, handle] : bundles_)
  {
    for (auto& producer: handle.getProducers())
    {
      if (producer == name_)
      {
        std::cout << name << ": " << handle.getTxps() << std::endl;
      }
    }
  }
  std::cout << "----- Consumed Bundles (hz) -----" << std::endl;
  for (auto& [name, handle] : bundles_)
  {
    for (auto& consumer: handle.getConsumers())
    {
      if (consumer == name_)
      {
        std::cout << name << ": " << handle.getRxps() << std::endl;
      }
    }
  }
  std::cout << "----- Heartbeats (hz) -----" << std::endl;
  for (auto& [name, handle] : heartbeat_bundles_)
  {
    for (auto& consumer: handle.getConsumers())
    {
      if (consumer == name_)
      {
        std::cout << name << ": " << handle.getRxps() << std::endl;
      }
    }
  }

  std::cout << "---------------------------------" << std::endl;
}

Connection::Connection(const NodeConfig& node_config, const NodeConfig& peer_config, const ConnectionConfig& connection_config, ReadCompleteCallback callback):
 config_(peer_config), callback_(callback), state_(PROTON_NODE_UNCONFIGURED), rx_kbps_(0.0), tx_kbps_(0.0)
{
  ConnectionEndpointConfig node_connection_endpoint_config, peer_connection_endpoint_config;
  if (connection_config.connection.first.node == node_config.name &&
      connection_config.connection.second.node == peer_config.name)
  {
    node_connection_endpoint_config = connection_config.connection.first;
    peer_connection_endpoint_config = connection_config.connection.second;
  }
  else if (connection_config.connection.second.node == node_config.name &&
           connection_config.connection.first.node == peer_config.name)
  {
    peer_connection_endpoint_config = connection_config.connection.first;
    node_connection_endpoint_config = connection_config.connection.second;
  }
  else
  {
    throw std::runtime_error("Invalid connection");
  }

  EndpointConfig node_endpoint_config = node_config.endpoints.at(node_connection_endpoint_config.id);
  EndpointConfig peer_endpoint_config = peer_config.endpoints.at(peer_connection_endpoint_config.id);

  if (node_endpoint_config.type != peer_endpoint_config.type)
  {
    throw std::runtime_error("Endpoint type mismatch");
  }

  transport_type_ = node_endpoint_config.type;

  if (transport_type_ == transport_types::UDP4) {
    auto node_endpoint =
        proton::socket_endpoint(node_endpoint_config.ip,
                                node_endpoint_config.port);
    auto peer_endpoint =
        proton::socket_endpoint(peer_endpoint_config.ip,
                                peer_endpoint_config.port);
    setTransport(
        std::make_unique<Udp4Transport>(node_endpoint, peer_endpoint));
  }
  else if (transport_type_ == transport_types::SERIAL) {
    auto peer_device = proton::serial_device(peer_endpoint_config.device, 0);
    setTransport(std::make_unique<SerialTransport>(peer_device));
  }

  state_ = PROTON_NODE_INACTIVE;
}

void Connection::run()
{
  run_thread_ = std::thread(&Connection::spin, this);

  if (config_.heartbeat.enabled)
  {
    heartbeat_thread_ = std::thread(&Connection::checkHeartbeat, this);
  }
}

void Connection::checkHeartbeat()
{
  while(1)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(config_.heartbeat.period));

    if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() - last_heartbeat_ms_ > config_.heartbeat.period)
    {
      state_ = PROTON_NODE_INACTIVE;
    }
  }
}

void Connection::heartbeat()
{
  state_ = PROTON_NODE_ACTIVE;
  last_heartbeat_ms_ = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

void Connection::spin()
{
  proton_status_e status;
  time_t start_time = std::time(nullptr);

  while(1)
  {
    if (std::time(nullptr) - start_time >= 1)
    {
      rx_kbps_ = static_cast<double>(getRx()) / 1000;
      tx_kbps_ = static_cast<double>(getTx()) / 1000;
      resetRx();
      resetTx();
      start_time = std::time(nullptr);
    }

    switch(getTransportState())
    {
      case PROTON_TRANSPORT_DISCONNECTED:
      {
        status = connect();
        if (status != PROTON_OK)
        {
          std::cout << "Failed to connect to peer " << config_.name << std::endl;
        }
        break;
      }

      case PROTON_TRANSPORT_CONNECTED:
      {
        status = pollForBundle();
        if (status != PROTON_OK)
        {
          std::cout << "Failed to poll for bundle on peer " << config_.name << ": " << status << std::endl;
        }
        break;
      }

      case PROTON_TRANSPORT_ERROR:
      {
        status = disconnect();
        if (status != PROTON_OK)
        {
          std::cout << "Failed to disconnect from peer " << config_.name << ": " << status << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
      }
    }
  }
}

proton_status_e Connection::pollForBundle()
{
  uint8_t read_buf[PROTON_MAX_MESSAGE_SIZE];
  size_t bytes_read = 0;
  proton_status_e status = read(read_buf, PROTON_MAX_MESSAGE_SIZE, bytes_read);

  if (status == PROTON_OK) {
    if (callback_ == nullptr)
    {
      return PROTON_NULL_PTR_ERROR;
    }

    // Remove frame overhead to get actual bundle length
    if (getTransportType() == proton::transport_types::SERIAL)
    {
      bytes_read -= SerialTransport::FRAME_OVERHEAD;
    }

    // Parse bundle
    auto bundle = Bundle();
    if (!bundle.ParseFromArray(read_buf, bytes_read))
    {
      return PROTON_SERIALIZATION_ERROR;
    }

    if (bundle.id() == 0)
    {
      heartbeat();
    }

    return callback_(bundle, config_.name);
  }

  return status;
}
