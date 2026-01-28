/*
 * Copyright 2026 Rockwell Automation Technologies, Inc., All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @author Roni Kreinin (roni.kreinin@rockwellautomation.com)
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
  std::vector<std::string> getProducers() { return producers_; }
  std::vector<std::string> getConsumers() { return consumers_; }
  std::shared_ptr<Bundle> getBundlePtr() { return bundle_; }
  std::map<std::string, SignalHandle> getSignalMap() { return signals_; }
  BundleCallback getCallback() { return callback_; }

  void updateBundle(const Bundle &bundle) { *bundle_ = bundle; }

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
  std::vector<std::string> producers_;
  std::vector<std::string> consumers_;
  std::shared_ptr<Bundle> bundle_;
  std::map<std::string, SignalHandle> signals_;
  BundleCallback callback_;
  uint32_t rx_count_, rxps_;
  uint32_t tx_count_, txps_;
};

} // namespace proton

#endif // INC_PROTONCPP_BUNDLE_HPP_
