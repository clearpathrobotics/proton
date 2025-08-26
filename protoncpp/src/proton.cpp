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

using namespace proton::node;

Signal::Signal(proton::config::SignalConfig config, proton::Signal *message)
    : config_(config), message_(message) {
  initSignal();
}

std::string Signal::getName() { return config_.getName(); }

proton::Signal::SignalCase Signal::getType() { return message_->signal_case(); }

uint32_t Signal::getLength() { return config_.getLength(); }

uint32_t Signal::getCapacity() { return config_.getCapacity(); }

void Signal::initSignal() {
  proton::Signal::SignalCase type;

  if (getLength() == 0)
  {
    type = config_.SignalMap.at(config_.getTypeString());
  }
  else
  {
    type = config_.ListSignalMap.at(config_.getTypeString());
  }

  switch (type) {
  case proton::Signal::SignalCase::kDoubleValue: {
    message_->set_double_value(0.0);
    break;
  }

  case proton::Signal::SignalCase::kFloatValue: {
    message_->set_float_value(0.0f);
    break;
  }

  case proton::Signal::SignalCase::kInt32Value: {
    message_->set_int32_value(0);
    break;
  }

  case proton::Signal::SignalCase::kInt64Value: {
    message_->set_int64_value(0);
    break;
  }

  case proton::Signal::SignalCase::kUint32Value: {
    message_->set_uint32_value(0);
    break;
  }

  case proton::Signal::SignalCase::kUint64Value: {
    message_->set_uint64_value(0);
    break;
  }

  case proton::Signal::SignalCase::kBoolValue: {
    message_->set_bool_value(false);
    break;
  }

  case proton::Signal::SignalCase::kStringValue: {
    message_->set_string_value("");
    break;
  }

  case proton::Signal::SignalCase::kBytesValue: {
    message_->set_bytes_value("");
    break;
  }

  case proton::Signal::SignalCase::kListDoubleValue: {
    auto *list = message_->mutable_list_double_value();
    for (uint32_t i = 0; i < getLength(); i++) {
      list->add_doubles(0.0);
    }
    break;
  }

  case proton::Signal::SignalCase::kListFloatValue: {
    auto *list = message_->mutable_list_float_value();
    for (uint32_t i = 0; i < getLength(); i++) {
      list->add_floats(0.0f);
    }
    break;
  }

  case proton::Signal::SignalCase::kListInt32Value: {
    auto *list = message_->mutable_list_int32_value();
    for (uint32_t i = 0; i < getLength(); i++) {
      list->add_int32s(0);
    }
    break;
  }

  case proton::Signal::SignalCase::kListInt64Value: {
    auto *list = message_->mutable_list_int64_value();
    for (uint32_t i = 0; i < getLength(); i++) {
      list->add_int64s(0);
    }
    break;
  }

  case proton::Signal::SignalCase::kListUint32Value: {
    auto *list = message_->mutable_list_uint32_value();
    for (uint32_t i = 0; i < getLength(); i++) {
      list->add_uint32s(0);
    }
    break;
  }

  case proton::Signal::SignalCase::kListUint64Value: {
    auto *list = message_->mutable_list_uint64_value();
    for (uint32_t i = 0; i < getLength(); i++) {
      list->add_uint64s(0);
    }
    break;
  }

  case proton::Signal::SignalCase::kListBoolValue: {
    auto *list = message_->mutable_list_bool_value();
    for (uint32_t i = 0; i < getLength(); i++) {
      list->add_bools(false);
    }
    break;
  }

  case proton::Signal::SignalCase::kListStringValue: {
    auto *list = message_->mutable_list_string_value();
    for (uint32_t i = 0; i < getLength(); i++) {
      list->add_strings("");
    }
    break;
  }

  default:
    break;
  }
}

Bundle::Bundle(proton::config::BundleConfig config) : config_(config) {
  message_.set_id(config_.getID());

  for (auto s : config_.getSignals()) {
    proton::Signal *signal = message_.add_signals();
    signals_.insert(std::pair(s.getName(), proton::node::Signal(s, signal)));
  }
}

std::string Bundle::getName() { return config_.getName(); }

uint32_t Bundle::getID() { return message_.id(); }

std::string Bundle::getProducer() { return config_.getProducer(); }

std::string Bundle::getConsumer() { return config_.getConsumer(); }

proton::node::Signal Bundle::getSignal(std::string name) {
  return signals_.at(name);
}

proton::Bundle Bundle::getMessage() { return message_; }

Node::Node(const std::string config_file) : config_(config_file) {
  for (auto b : config_.getBundles()) {
    proton::node::Bundle bundle(b);
    // bundle.getMessage().PrintDebugString();
    bundles_.insert(std::pair(b.getName(), bundle));
  }
}

proton::node::Bundle &Node::getBundle(std::string name) {
  return bundles_.at(name);
}
