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

#include "protoncpp/bundle_manager.hpp"

using namespace proton;

SignalHandle::SignalHandle(proton::SignalConfig config,
                           std::shared_ptr<proton::Signal> ptr, int idx) {
  name_ = config.name;
  bundle_name_ = config.bundle_name;
  length_ = config.length;
  capacity_ = config.capacity;

  try {
    if (length_ == 0) {
      type_ = signal_map::ScalarSignalMap.at(config.type_string);
    } else {
      type_ = signal_map::ListSignalMap.at(config.type_string);
    }
  } catch (std::out_of_range &e) {
    throw std::runtime_error("Invalid signal_ type_ " + config.type_string +
                             " for signal_ " + name_);
  }

  signal_ = ptr;
  index_ = idx;

  switch (type_) {
  case proton::Signal::SignalCase::kDoubleValue: {
    signal_->set_double_value(0.0);
    break;
  }

  case proton::Signal::SignalCase::kFloatValue: {
    signal_->set_float_value(0.0f);
    break;
  }

  case proton::Signal::SignalCase::kInt32Value: {
    signal_->set_int32_value(0);
    break;
  }

  case proton::Signal::SignalCase::kInt64Value: {
    signal_->set_int64_value(0);
    break;
  }

  case proton::Signal::SignalCase::kUint32Value: {
    signal_->set_uint32_value(0);
    break;
  }

  case proton::Signal::SignalCase::kUint64Value: {
    signal_->set_uint64_value(0);
    break;
  }

  case proton::Signal::SignalCase::kBoolValue: {
    signal_->set_bool_value(false);
    break;
  }

  case proton::Signal::SignalCase::kStringValue: {
    auto str = signal_->mutable_string_value();
    str->resize(capacity_);
    break;
  }

  case proton::Signal::SignalCase::kBytesValue: {
    auto bytes = signal_->mutable_bytes_value();
    bytes->resize(capacity_);
    break;
  }

  case proton::Signal::SignalCase::kListDoubleValue: {
    auto *list = signal_->mutable_list_double_value();
    list->mutable_doubles()->Resize(length_, 0.0);
    break;
  }

  case proton::Signal::SignalCase::kListFloatValue: {
    auto *list = signal_->mutable_list_float_value();
    list->mutable_floats()->Resize(length_, 0.0f);
    break;
  }

  case proton::Signal::SignalCase::kListInt32Value: {
    auto *list = signal_->mutable_list_int32_value();
    list->mutable_int32s()->Resize(length_, 0);
    break;
  }

  case proton::Signal::SignalCase::kListInt64Value: {
    auto *list = signal_->mutable_list_int64_value();
    list->mutable_int64s()->Resize(length_, 0);
    break;
  }

  case proton::Signal::SignalCase::kListUint32Value: {
    auto *list = signal_->mutable_list_uint32_value();
    list->mutable_uint32s()->Resize(length_, 0);
    break;
  }

  case proton::Signal::SignalCase::kListUint64Value: {
    auto *list = signal_->mutable_list_uint64_value();
    list->mutable_uint64s()->Resize(length_, 0);
    break;
  }

  case proton::Signal::SignalCase::kListBoolValue: {
    auto *list = signal_->mutable_list_bool_value();
    list->mutable_bools()->Resize(length_, false);
    break;
  }

  case proton::Signal::SignalCase::kListStringValue: {
    auto *list = signal_->mutable_list_string_value();
    for (uint32_t i = 0; i < length_; i++) {
      list->add_strings("");
    }
    break;
  }

  default:
    break;
  }
}

BundleHandle::BundleHandle(BundleConfig config) {
  name_ = config.name;
  id_ = config.id;
  producer_ = config.producer;
  consumer_ = config.consumer;
  bundle_ = std::make_shared<Bundle>();
  bundle_->set_id(id_);
  callback_ = nullptr;

  // Add each signal_ for this bundle
  for (auto s : config.signals) {
    addSignal(s);
  }
}

void BundleHandle::registerCallback(BundleCallback callback)
{
  if (callback)
  {
    callback_ = callback;
  }
}

void BundleManager::addBundle(BundleConfig config) {
  bundles_.emplace(config.name, BundleHandle(config));
}

BundleHandle &BundleManager::getBundle(const std::string &bundle_name) {
  try {
    return bundles_.at(bundle_name);
  } catch (std::out_of_range &e) {
    throw std::runtime_error("Invalid bundle name_ " + bundle_name);
  }
}

BundleHandle &BundleManager::receiveBundle(const uint8_t *buffer,
                                           const uint32_t len) {
  auto bundle = Bundle();
  bundle.ParseFromArray(buffer, len);
  return setBundle(bundle);
}

BundleHandle &BundleManager::setBundle(const Bundle &bundle) {
  for (auto &[name_, handle] : bundles_) {
    if (handle.getId() == bundle.id()) {
      *handle.getBundlePtr().get() = bundle;
      return handle;
    }
  }

  throw std::runtime_error("Invalid bundle received with ID " + bundle.id());
}

void BundleManager::printAllBundles() {
  for (auto &[name_, handle] : bundles_) {
    handle.printBundle();
  }
}

void BundleManager::printAllBundlesVerbose() {
  for (auto &[name_, handle] : bundles_) {
    handle.printBundleVerbose();
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

void BundleHandle::addSignal(proton::SignalConfig config) {
  auto sig = std::shared_ptr<proton::Signal>(bundle_->add_signals());
  int idx = bundle_->signals_size() - 1;

  signals_.emplace(config.name, SignalHandle(config, sig, idx));
}

SignalHandle &BundleHandle::getSignal(const std::string &signal_name) {
  try {
    return signals_.at(signal_name);
  } catch (std::out_of_range &e) {
    throw std::runtime_error("Invalid signal_ name_ " + signal_name +
                             " in bundle " + name_);
  }
}

template <> double SignalHandle::getValue<double>() {
  if (type_ != proton::Signal::SignalCase::kDoubleValue) {
    throw std::runtime_error("Signal " + name_ + " is not of double type_");
  }
  return signal_->double_value();
}

template <> float SignalHandle::getValue<float>() {
  if (type_ != proton::Signal::SignalCase::kFloatValue) {
    throw std::runtime_error("Signal " + name_ + " is not of float type_");
  }
  return signal_->float_value();
}

template <> int32_t SignalHandle::getValue<int32_t>() {
  if (type_ != proton::Signal::SignalCase::kInt32Value) {
    throw std::runtime_error("Signal " + name_ + " is not of int32_t type_");
  }
  return signal_->int32_value();
}

template <> int64_t SignalHandle::getValue<int64_t>() {
  if (type_ != proton::Signal::SignalCase::kInt64Value) {
    throw std::runtime_error("Signal " + name_ + " is not of int64_t type_");
  }
  return signal_->int64_value();
}

template <> uint32_t SignalHandle::getValue<uint32_t>() {
  if (type_ != proton::Signal::SignalCase::kUint32Value) {
    throw std::runtime_error("Signal " + name_ + " is not of uint32_t type_");
  }
  return signal_->uint32_value();
}

template <> uint64_t SignalHandle::getValue<uint64_t>() {
  if (type_ != proton::Signal::SignalCase::kUint64Value) {
    throw std::runtime_error("Signal " + name_ + " is not of uint64_t type_");
  }
  return signal_->uint64_value();
}

template <> bool SignalHandle::getValue<bool>() {
  if (type_ != proton::Signal::SignalCase::kBoolValue) {
    throw std::runtime_error("Signal " + name_ + " is not of bool type_");
  }
  return signal_->bool_value();
}

template <> std::string SignalHandle::getValue<std::string>() {
  if (type_ != proton::Signal::SignalCase::kStringValue) {
    throw std::runtime_error("Signal " + name_ + " is not of std::string type_");
  }
  return signal_->string_value();
}

template <> proton::bytes SignalHandle::getValue<proton::bytes>() {
  if (type_ != proton::Signal::SignalCase::kBytesValue) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::bytes type_");
  }
  const std::string &bytes = signal_->bytes_value();
  return {bytes.begin(), bytes.end()};
}

template <> proton::list_double SignalHandle::getValue<proton::list_double>() {
  if (type_ != proton::Signal::SignalCase::kListDoubleValue) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::list_double type_");
  }
  return {signal_->list_double_value().doubles().begin(),
          signal_->list_double_value().doubles().end()};
}

template <> proton::list_float SignalHandle::getValue<proton::list_float>() {
  if (type_ != proton::Signal::SignalCase::kListFloatValue) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::list_float type_");
  }
  return {signal_->list_float_value().floats().begin(),
          signal_->list_float_value().floats().end()};
}

template <> proton::list_int32 SignalHandle::getValue<proton::list_int32>() {
  if (type_ != proton::Signal::SignalCase::kListInt32Value) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::list_int32 type_");
  }
  return {signal_->list_int32_value().int32s().begin(),
          signal_->list_int32_value().int32s().end()};
}

template <> proton::list_int64 SignalHandle::getValue<proton::list_int64>() {
  if (type_ != proton::Signal::SignalCase::kListInt64Value) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::list_int64 type_");
  }
  return {signal_->list_int64_value().int64s().begin(),
          signal_->list_int64_value().int64s().end()};
}

template <> proton::list_uint32 SignalHandle::getValue<proton::list_uint32>() {
  if (type_ != proton::Signal::SignalCase::kListUint32Value) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::list_uint32 type_");
  }
  return {signal_->list_uint32_value().uint32s().begin(),
          signal_->list_uint32_value().uint32s().end()};
}

template <> proton::list_uint64 SignalHandle::getValue<proton::list_uint64>() {
  if (type_ != proton::Signal::SignalCase::kListUint64Value) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::list_uint64 type_");
  }
  return {signal_->list_uint64_value().uint64s().begin(),
          signal_->list_uint64_value().uint64s().end()};
}

template <> proton::list_bool SignalHandle::getValue<proton::list_bool>() {
  if (type_ != proton::Signal::SignalCase::kListBoolValue) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::list_bool type_");
  }
  return {signal_->list_bool_value().bools().begin(),
          signal_->list_bool_value().bools().end()};
}

template <> proton::list_string SignalHandle::getValue<proton::list_string>() {
  if (type_ != proton::Signal::SignalCase::kListStringValue) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::list_string type_");
  }
  return {signal_->list_string_value().strings().begin(),
          signal_->list_string_value().strings().end()};
}

template <> void SignalHandle::setValue<double>(double value) {
  if (type_ != proton::Signal::SignalCase::kDoubleValue) {
    throw std::runtime_error("Signal " + name_ + " is not of double type_");
  }
  signal_->set_double_value(value);
}

template <> void SignalHandle::setValue<float>(float value) {
  if (type_ != proton::Signal::SignalCase::kFloatValue) {
    throw std::runtime_error("Signal " + name_ + " is not of float type_");
  }
  signal_->set_float_value(value);
}

template <> void SignalHandle::setValue<int32_t>(int32_t value) {
  if (type_ != proton::Signal::SignalCase::kInt32Value) {
    throw std::runtime_error("Signal " + name_ + " is not of int32_t type_");
  }
  signal_->set_int32_value(value);
}

template <> void SignalHandle::setValue<int64_t>(int64_t value) {
  if (type_ != proton::Signal::SignalCase::kInt64Value) {
    throw std::runtime_error("Signal " + name_ + " is not of int64_t type_");
  }
  signal_->set_int64_value(value);
}

template <> void SignalHandle::setValue<uint32_t>(uint32_t value) {
  if (type_ != proton::Signal::SignalCase::kUint32Value) {
    throw std::runtime_error("Signal " + name_ + " is not of uint32_t type_");
  }
  signal_->set_uint32_value(value);
}

template <> void SignalHandle::setValue<uint64_t>(uint64_t value) {
  if (type_ != proton::Signal::SignalCase::kUint64Value) {
    throw std::runtime_error("Signal " + name_ + " is not of uint64_t type_");
  }
  signal_->set_uint64_value(value);
}

template <> void SignalHandle::setValue<bool>(bool value) {
  if (type_ != proton::Signal::SignalCase::kBoolValue) {
    throw std::runtime_error("Signal " + name_ + " is not of bool type_");
  }
  signal_->set_bool_value(value);
}

template <> void SignalHandle::setValue<std::string>(std::string value) {
  if (type_ != proton::Signal::SignalCase::kStringValue) {
    throw std::runtime_error("Signal " + name_ + " is not of std::string type_");
  }
  signal_->set_string_value(value);
}

template <> void SignalHandle::setValue<proton::bytes>(proton::bytes value) {
  if (type_ != proton::Signal::SignalCase::kBytesValue) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::bytes type_");
  }
  signal_->mutable_bytes_value()->assign(value.begin(), value.end());
}

template <>
void SignalHandle::setValue<proton::list_double>(proton::list_double value) {
  if (type_ != proton::Signal::SignalCase::kListDoubleValue) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::list_double type_");
  }
  signal_->mutable_list_double_value()->mutable_doubles()->Assign(value.begin(),
                                                                 value.end());
}

template <>
void SignalHandle::setValue<proton::list_float>(proton::list_float value) {
  if (type_ != proton::Signal::SignalCase::kListFloatValue) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::list_float type_");
  }
  signal_->mutable_list_float_value()->mutable_floats()->Assign(value.begin(),
                                                               value.end());
}

template <>
void SignalHandle::setValue<proton::list_int32>(proton::list_int32 value) {
  if (type_ != proton::Signal::SignalCase::kListInt32Value) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::list_int32 type_");
  }
  signal_->mutable_list_int32_value()->mutable_int32s()->Assign(value.begin(),
                                                               value.end());
}

template <>
void SignalHandle::setValue<proton::list_int64>(proton::list_int64 value) {
  if (type_ != proton::Signal::SignalCase::kListInt64Value) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::list_int64 type_");
  }
  signal_->mutable_list_int64_value()->mutable_int64s()->Assign(value.begin(),
                                                               value.end());
}

template <>
void SignalHandle::setValue<proton::list_uint32>(proton::list_uint32 value) {
  if (type_ != proton::Signal::SignalCase::kListUint32Value) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::list_uint32 type_");
  }
  signal_->mutable_list_uint32_value()->mutable_uint32s()->Assign(value.begin(),
                                                                 value.end());
}

template <>
void SignalHandle::setValue<proton::list_uint64>(proton::list_uint64 value) {
  if (type_ != proton::Signal::SignalCase::kListUint64Value) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::list_uint64 type_");
  }
  signal_->mutable_list_uint64_value()->mutable_uint64s()->Assign(value.begin(),
                                                                 value.end());
}

template <>
void SignalHandle::setValue<proton::list_bool>(proton::list_bool value) {
  if (type_ != proton::Signal::SignalCase::kListBoolValue) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::list_bool type_");
  }
  signal_->mutable_list_bool_value()->mutable_bools()->Assign(value.begin(),
                                                             value.end());
}

template <>
void SignalHandle::setValue<proton::list_string>(proton::list_string value) {
  if (type_ != proton::Signal::SignalCase::kListStringValue) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::list_string type_");
  }
  signal_->mutable_list_string_value()->mutable_strings()->Assign(value.begin(),
                                                                 value.end());
}
