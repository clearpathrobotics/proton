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

#include "protoncpp/bundle.hpp"

using namespace proton;

BundleHandle::BundleHandle(BundleConfig config) {
  name_ = config.name;
  id_ = config.id;
  producer_ = config.producer;
  consumer_ = config.consumer;
  bundle_ = std::make_shared<Bundle>();
  bundle_->set_id(id_);
  callback_ = nullptr;
  rx_count_ = rxps_ = 0;
  tx_count_ = txps_ = 0;

  // Add each signal for this bundle
  for (auto s : config.signals) {
    addSignal(s);
  }
}

BundleHandle::BundleHandle()
{
  name_ = "UNKNOWN";
  id_ = 0;
  producer_ = "UNKNOWN";
  consumer_ = "UNKNOWN";
  bundle_ = std::make_shared<Bundle>();
  bundle_->set_id(id_);
  callback_ = nullptr;
  rx_count_ = rxps_ = 0;
  tx_count_ = txps_ = 0;
}

void BundleHandle::registerCallback(BundleCallback callback)
{
  if (callback)
  {
    callback_ = callback;
  }
}

void BundleHandle::printBundle() {
  std::cout << name_ << " bundle: {" << std::endl;
  std::cout << "  id: 0x" << std::hex << id_ << std::dec << std::endl;
  std::cout << "}" << std::endl;
}

void BundleHandle::printBundleVerbose() {
  std::cout << name_ << " bundle: {" << std::endl;
  std::cout << "  id: 0x" << std::hex << id_ << std::dec << std::endl;
  std::cout << "  signals: {" << std::endl;
  for (auto&[name, handle] : signals_)
  {
    std::cout << "    " << name << " {" << std::endl;
    std::string debug = handle.getSignalPtr()->DebugString();
    std::string spaces = "  "; //tab
    std::string tab = "\t"; //four spaces
    std::string close_bracket = "      }\r\n";

    auto it = debug.find(spaces);
    while (it != std::string::npos)
    {
      debug.replace(it, spaces.size(), tab);
      it = debug.find(spaces, it);
    }

    it = debug.find('}');
    if (it != std::string::npos)
    {
      debug.replace(it, close_bracket.size(), close_bracket);
    }

    std::cout << "      " << debug;
    std::cout << "    }" << std::endl;
  }
  std::cout << "  }" << std::endl;
  std::cout << "}" << std::endl;
}

void BundleHandle::addSignal(SignalConfig config) {
  Signal* sig;
  // Non-constant signals are added to the bundle
  if (!config.is_const)
  {
    sig = bundle_->add_signals();
  }
  // Constant signals exist just to store their value
  else
  {
    sig = new Signal();
  }

  signals_.emplace(config.name, SignalHandle(config, name_, sig));
}

SignalHandle &BundleHandle::getSignal(const std::string &signal_name) {
  try {
    return signals_.at(signal_name);
  } catch (std::out_of_range &e) {
    throw std::runtime_error("Invalid signal name " + signal_name +
                             " in bundle " + name_);
  }
}

const SignalHandle BundleHandle::getConstSignal(const std::string &signal_name) const {
  try {
    return signals_.at(signal_name);
  } catch (std::out_of_range &e) {
    throw std::runtime_error("Invalid signal name " + signal_name +
                             " in bundle " + name_);
  }
}

bool BundleHandle::hasSignal(const std::string &signal_name) const {
  return signals_.find(signal_name) != signals_.end();
}