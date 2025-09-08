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

#ifndef INC_PROTONCPP_MANAGER_HPP_
#define INC_PROTONCPP_MANAGER_HPP_

#include "protoncpp/bundle.pb.h"
#include "protoncpp/config.hpp"
#include <map>
#include <memory>

namespace proton {

using bytes = std::vector<uint8_t>;
using list_double = std::vector<double>;
using list_float = std::vector<float>;
using list_int32 = std::vector<int32_t>;
using list_int64 = std::vector<int64_t>;
using list_uint32 = std::vector<uint32_t>;
using list_uint64 = std::vector<uint64_t>;
using list_bool = std::vector<bool>;
using list_string = std::vector<std::string>;

class SignalHandle {
public:
  SignalHandle(proton::SignalConfig config, std::shared_ptr<proton::Signal> ptr,
               int idx);

  bool valid() const { return index_ >= 0; }

  // Primary template (not defined -> unsupported types will fail to compile)
  template <typename T> T getValue();
  template <typename T> void setValue(T value);

  std::string getName() { return name_; }
  std::string getBundleName() { return bundle_name_; }
  proton::Signal::SignalCase getType() { return type_; }
  uint32_t getLength() { return length_; }
  uint32_t getCapacity() { return capacity_; }
  int getIndex() { return index_; }
  std::shared_ptr<proton::Signal> getSignalPtr() { return signal_; }

private:
  std::string name_;
  std::string bundle_name_;
  proton::Signal::SignalCase type_;
  uint32_t length_;
  uint32_t capacity_;
  int index_; // position in bundle.signals()
  std::shared_ptr<proton::Signal> signal_;
};

class BundleHandle {
public:
  using BundleCallback = std::function<void(BundleHandle &)>;

  BundleHandle(proton::BundleConfig config);

  void addSignal(proton::SignalConfig config);
  SignalHandle &getSignal(const std::string &signal_name);

  void printBundle();
  void printBundleVerbose();

  void registerCallback(BundleCallback callback);

  std::string getName() { return name_; }
  uint32_t getId() { return id_; }
  std::string getProducer() { return producer_; }
  std::string getConsumer() { return consumer_; }
  std::shared_ptr<proton::Bundle> getBundlePtr() { return bundle_; }
  std::map<std::string, SignalHandle> getSignalMap() { return signals_; }
  BundleCallback getCallback() { return callback_; }

  void incrementRxCount() { rx_count_++; };
  void resetRxCount() { rx_count_ = 0; };
  uint32_t getRxCount() { return rx_count_; };

  void setRxps(uint32_t rxps) { rxps_ = rxps; }
  uint32_t getRxps() { return rxps_; }

  void incrementTxCount() { tx_count_++; };
  void resetTxCount() { tx_count_ = 0; };
  uint32_t getTxCount() { return tx_count_; };

  void setTxps(uint32_t txps) { txps_ = txps; }
  uint32_t getTxps() { return txps_; }

private:
  std::string name_;
  uint32_t id_;
  std::string producer_;
  std::string consumer_;
  std::shared_ptr<proton::Bundle> bundle_;
  std::map<std::string, SignalHandle> signals_;
  BundleCallback callback_;
  uint32_t rx_count_, rxps_;
  uint32_t tx_count_, txps_;
};

class BundleManager {
public:
  void addBundle(proton::BundleConfig config);
  BundleHandle &getBundle(const std::string &bundle_name);
  BundleHandle &receiveBundle(const uint8_t *buffer, const uint32_t len);

  BundleHandle &setBundle(const proton::Bundle &bundle);

  void printAllBundles();
  void printAllBundlesVerbose();

protected:
  std::map<std::string, BundleHandle> bundles_;
};

} // namespace proton

#endif // INC_PROTONCPP_MANAGER_HPP_
