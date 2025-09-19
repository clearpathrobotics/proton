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

#ifndef INC_PROTONCPP_BUNDLE_HPP_
#define INC_PROTONCPP_BUNDLE_HPP_

#include "protoncpp/signal.hpp"
#include "protoncpp/bundle.pb.h"
#include "protoncpp/config.hpp"
#include <map>
#include <memory>

namespace proton {

class BundleHandle {
public:
  using BundleCallback = std::function<void(BundleHandle &)>;

  BundleHandle();
  BundleHandle(BundleConfig config);

  void addSignal(SignalConfig config);
  SignalHandle &getSignal(const std::string &signal_name);
  const SignalHandle getConstSignal(const std::string &signal_name) const;
  bool hasSignal(const std::string &signal_name) const;

  void printBundle();
  void printBundleVerbose();

  void registerCallback(BundleCallback callback);

  std::string getName() { return name_; }
  uint32_t getId() { return id_; }
  std::string getProducer() { return producer_; }
  std::string getConsumer() { return consumer_; }
  std::shared_ptr<Bundle> getBundlePtr() { return bundle_; }
  Bundle& getBundle() { return *bundle_; }
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
  std::shared_ptr<Bundle> bundle_;
  std::map<std::string, SignalHandle> signals_;
  BundleCallback callback_;
  uint32_t rx_count_, rxps_;
  uint32_t tx_count_, txps_;
};

} // namespace proton

#endif // INC_PROTONCPP_BUNDLE_HPP_
