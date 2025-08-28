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

#include "protoncpp/manager.hpp"

using namespace proton;

SignalHandle::SignalHandle(proton::SignalConfig config,
                           std::shared_ptr<proton::Signal> ptr, int idx) {
  name = config.getName();
  bundle_name = config.getBundleName();
  length = config.getLength();
  capacity = config.getCapacity();

  try {
    if (length == 0) {
      type = config.SignalMap.at(config.getTypeString());
    } else {
      type = config.ListSignalMap.at(config.getTypeString());
    }
  } catch (std::out_of_range &e) {
    throw std::runtime_error("Invalid signal type " + config.getTypeString() +
                             " for signal " + name);
  }

  signal = ptr;
  index = idx;

  switch (type) {
  case proton::Signal::SignalCase::kDoubleValue: {
    signal->set_double_value(0.0);
    break;
  }

  case proton::Signal::SignalCase::kFloatValue: {
    signal->set_float_value(0.0f);
    break;
  }

  case proton::Signal::SignalCase::kInt32Value: {
    signal->set_int32_value(0);
    break;
  }

  case proton::Signal::SignalCase::kInt64Value: {
    signal->set_int64_value(0);
    break;
  }

  case proton::Signal::SignalCase::kUint32Value: {
    signal->set_uint32_value(0);
    break;
  }

  case proton::Signal::SignalCase::kUint64Value: {
    signal->set_uint64_value(0);
    break;
  }

  case proton::Signal::SignalCase::kBoolValue: {
    signal->set_bool_value(false);
    break;
  }

  case proton::Signal::SignalCase::kStringValue: {
    auto str = signal->mutable_string_value();
    str->resize(capacity);
    break;
  }

  case proton::Signal::SignalCase::kBytesValue: {
    auto bytes = signal->mutable_bytes_value();
    bytes->resize(capacity);
    break;
  }

  case proton::Signal::SignalCase::kListDoubleValue: {
    auto *list = signal->mutable_list_double_value();
    list->mutable_doubles()->Resize(length, 0.0);
    break;
  }

  case proton::Signal::SignalCase::kListFloatValue: {
    auto *list = signal->mutable_list_float_value();
    list->mutable_floats()->Resize(length, 0.0f);
    break;
  }

  case proton::Signal::SignalCase::kListInt32Value: {
    auto *list = signal->mutable_list_int32_value();
    list->mutable_int32s()->Resize(length, 0);
    break;
  }

  case proton::Signal::SignalCase::kListInt64Value: {
    auto *list = signal->mutable_list_int64_value();
    list->mutable_int64s()->Resize(length, 0);
    break;
  }

  case proton::Signal::SignalCase::kListUint32Value: {
    auto *list = signal->mutable_list_uint32_value();
    list->mutable_uint32s()->Resize(length, 0);
    break;
  }

  case proton::Signal::SignalCase::kListUint64Value: {
    auto *list = signal->mutable_list_uint64_value();
    list->mutable_uint64s()->Resize(length, 0);
    break;
  }

  case proton::Signal::SignalCase::kListBoolValue: {
    auto *list = signal->mutable_list_bool_value();
    list->mutable_bools()->Resize(length, false);
    break;
  }

  case proton::Signal::SignalCase::kListStringValue: {
    auto *list = signal->mutable_list_string_value();
    for (uint32_t i = 0; i < length; i++) {
      list->add_strings("");
    }
    break;
  }

  default:
    break;
  }
}

BundleHandle::BundleHandle(proton::BundleConfig config) {
  name = config.getName();
  id = config.getID();
  producer = config.getProducer();
  consumer = config.getConsumer();
  bundle = std::make_shared<proton::Bundle>();
  bundle->set_id(id);

  // Add each signal for this bundle
  for (auto s : config.getSignals()) {
    addSignal(name, s);
  }
}

void BundleManager::addBundle(proton::BundleConfig config) {
  bundles_.emplace(config.getName(), BundleHandle(config));
}

BundleHandle &BundleManager::getBundle(const std::string &bundle_name) {
  try {
    return bundles_.at(bundle_name);
  } catch (std::out_of_range &e) {
    throw std::runtime_error("Invalid bundle name " + bundle_name);
  }
}

BundleHandle &BundleManager::receiveBundle(const uint8_t *buffer,
                                           const uint32_t len) {
  auto bundle = proton::Bundle();
  bundle.ParseFromArray(buffer, len);
  return setBundle(bundle);
}

BundleHandle &BundleManager::setBundle(const proton::Bundle &bundle) {
  for (auto &[name, handle] : bundles_) {
    if (handle.id == bundle.id()) {
      *handle.bundle.get() = bundle;
      return handle;
    }
  }

  throw std::runtime_error("Invalid bundle received with ID " + bundle.id());
}

void BundleManager::printAllBundles() {
  for (auto &[name, handle] : bundles_) {
    handle.printBundle();
  }
}

void BundleManager::printAllBundlesVerbose() {
  for (auto &[name, handle] : bundles_) {
    handle.printBundleVerbose();
  }
}

void BundleHandle::printBundle() {
  std::cout << name << " bundle: {" << std::endl;
  std::cout << "  id: 0x" << std::hex << id << std::dec << std::endl;
  std::cout << "}" << std::endl;
}

void BundleHandle::printBundleVerbose() {
  std::cout << name << " bundle: {" << std::endl;
  std::cout << "  id: 0x" << std::hex << id << std::dec << std::endl;
  std::cout << "  signals: {" << std::endl;
  for (auto&[name, handle] : signals_)
  {
    std::cout << "    " << name << " {" << std::endl;
    std::string debug = handle.signal->DebugString();
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

void BundleHandle::addSignal(const std::string &bundle_name,
                             proton::SignalConfig config) {
  auto sig = std::shared_ptr<proton::Signal>(bundle->add_signals());
  int idx = bundle->signals_size() - 1;

  signals_.emplace(config.getName(), SignalHandle(config, sig, idx));
}

SignalHandle &BundleHandle::getSignal(const std::string &signal_name) {
  try {
    return signals_.at(signal_name);
  } catch (std::out_of_range &e) {
    throw std::runtime_error("Invalid signal name " + signal_name +
                             " in bundle " + name);
  }
}

template <> double SignalHandle::getValue<double>() {
  if (type != proton::Signal::SignalCase::kDoubleValue) {
    throw std::runtime_error("Signal " + name + " is not of double type");
  }
  return signal->double_value();
}

template <> float SignalHandle::getValue<float>() {
  if (type != proton::Signal::SignalCase::kFloatValue) {
    throw std::runtime_error("Signal " + name + " is not of float type");
  }
  return signal->float_value();
}

template <> int32_t SignalHandle::getValue<int32_t>() {
  if (type != proton::Signal::SignalCase::kInt32Value) {
    throw std::runtime_error("Signal " + name + " is not of int32_t type");
  }
  return signal->int32_value();
}

template <> int64_t SignalHandle::getValue<int64_t>() {
  if (type != proton::Signal::SignalCase::kInt64Value) {
    throw std::runtime_error("Signal " + name + " is not of int64_t type");
  }
  return signal->int64_value();
}

template <> uint32_t SignalHandle::getValue<uint32_t>() {
  if (type != proton::Signal::SignalCase::kUint32Value) {
    throw std::runtime_error("Signal " + name + " is not of uint32_t type");
  }
  return signal->uint32_value();
}

template <> uint64_t SignalHandle::getValue<uint64_t>() {
  if (type != proton::Signal::SignalCase::kUint64Value) {
    throw std::runtime_error("Signal " + name + " is not of uint64_t type");
  }
  return signal->uint64_value();
}

template <> bool SignalHandle::getValue<bool>() {
  if (type != proton::Signal::SignalCase::kBoolValue) {
    throw std::runtime_error("Signal " + name + " is not of bool type");
  }
  return signal->bool_value();
}

template <> std::string SignalHandle::getValue<std::string>() {
  if (type != proton::Signal::SignalCase::kStringValue) {
    throw std::runtime_error("Signal " + name + " is not of std::string type");
  }
  return signal->string_value();
}

template <> proton::bytes SignalHandle::getValue<proton::bytes>() {
  if (type != proton::Signal::SignalCase::kBytesValue) {
    throw std::runtime_error("Signal " + name +
                             " is not of proton::bytes type");
  }
  const std::string &bytes = signal->bytes_value();
  return {bytes.begin(), bytes.end()};
}

template <> proton::list_double SignalHandle::getValue<proton::list_double>() {
  if (type != proton::Signal::SignalCase::kListDoubleValue) {
    throw std::runtime_error("Signal " + name +
                             " is not of proton::list_double type");
  }
  return {signal->list_double_value().doubles().begin(),
          signal->list_double_value().doubles().end()};
}

template <> proton::list_float SignalHandle::getValue<proton::list_float>() {
  if (type != proton::Signal::SignalCase::kListFloatValue) {
    throw std::runtime_error("Signal " + name +
                             " is not of proton::list_float type");
  }
  return {signal->list_float_value().floats().begin(),
          signal->list_float_value().floats().end()};
}

template <> proton::list_int32 SignalHandle::getValue<proton::list_int32>() {
  if (type != proton::Signal::SignalCase::kListInt32Value) {
    throw std::runtime_error("Signal " + name +
                             " is not of proton::list_int32 type");
  }
  return {signal->list_int32_value().int32s().begin(),
          signal->list_int32_value().int32s().end()};
}

template <> proton::list_int64 SignalHandle::getValue<proton::list_int64>() {
  if (type != proton::Signal::SignalCase::kListInt64Value) {
    throw std::runtime_error("Signal " + name +
                             " is not of proton::list_int64 type");
  }
  return {signal->list_int64_value().int64s().begin(),
          signal->list_int64_value().int64s().end()};
}

template <> proton::list_uint32 SignalHandle::getValue<proton::list_uint32>() {
  if (type != proton::Signal::SignalCase::kListUint32Value) {
    throw std::runtime_error("Signal " + name +
                             " is not of proton::list_uint32 type");
  }
  return {signal->list_uint32_value().uint32s().begin(),
          signal->list_uint32_value().uint32s().end()};
}

template <> proton::list_uint64 SignalHandle::getValue<proton::list_uint64>() {
  if (type != proton::Signal::SignalCase::kListUint64Value) {
    throw std::runtime_error("Signal " + name +
                             " is not of proton::list_uint64 type");
  }
  return {signal->list_uint64_value().uint64s().begin(),
          signal->list_uint64_value().uint64s().end()};
}

template <> proton::list_bool SignalHandle::getValue<proton::list_bool>() {
  if (type != proton::Signal::SignalCase::kListBoolValue) {
    throw std::runtime_error("Signal " + name +
                             " is not of proton::list_bool type");
  }
  return {signal->list_bool_value().bools().begin(),
          signal->list_bool_value().bools().end()};
}

template <> proton::list_string SignalHandle::getValue<proton::list_string>() {
  if (type != proton::Signal::SignalCase::kListStringValue) {
    throw std::runtime_error("Signal " + name +
                             " is not of proton::list_string type");
  }
  return {signal->list_string_value().strings().begin(),
          signal->list_string_value().strings().end()};
}

template <> void SignalHandle::setValue<double>(double value) {
  if (type != proton::Signal::SignalCase::kDoubleValue) {
    throw std::runtime_error("Signal " + name + " is not of double type");
  }
  signal->set_double_value(value);
}

template <> void SignalHandle::setValue<float>(float value) {
  if (type != proton::Signal::SignalCase::kFloatValue) {
    throw std::runtime_error("Signal " + name + " is not of float type");
  }
  signal->set_float_value(value);
}

template <> void SignalHandle::setValue<int32_t>(int32_t value) {
  if (type != proton::Signal::SignalCase::kInt32Value) {
    throw std::runtime_error("Signal " + name + " is not of int32_t type");
  }
  signal->set_int32_value(value);
}

template <> void SignalHandle::setValue<int64_t>(int64_t value) {
  if (type != proton::Signal::SignalCase::kInt64Value) {
    throw std::runtime_error("Signal " + name + " is not of int64_t type");
  }
  signal->set_int64_value(value);
}

template <> void SignalHandle::setValue<uint32_t>(uint32_t value) {
  if (type != proton::Signal::SignalCase::kUint32Value) {
    throw std::runtime_error("Signal " + name + " is not of uint32_t type");
  }
  signal->set_uint32_value(value);
}

template <> void SignalHandle::setValue<uint64_t>(uint64_t value) {
  if (type != proton::Signal::SignalCase::kUint64Value) {
    throw std::runtime_error("Signal " + name + " is not of uint64_t type");
  }
  signal->set_uint64_value(value);
}

template <> void SignalHandle::setValue<bool>(bool value) {
  if (type != proton::Signal::SignalCase::kBoolValue) {
    throw std::runtime_error("Signal " + name + " is not of bool type");
  }
  signal->set_bool_value(value);
}

template <> void SignalHandle::setValue<std::string>(std::string value) {
  if (type != proton::Signal::SignalCase::kStringValue) {
    throw std::runtime_error("Signal " + name + " is not of std::string type");
  }
  signal->set_string_value(value);
}

template <> void SignalHandle::setValue<proton::bytes>(proton::bytes value) {
  if (type != proton::Signal::SignalCase::kBytesValue) {
    throw std::runtime_error("Signal " + name +
                             " is not of proton::bytes type");
  }
  signal->mutable_bytes_value()->assign(value.begin(), value.end());
}

template <>
void SignalHandle::setValue<proton::list_double>(proton::list_double value) {
  if (type != proton::Signal::SignalCase::kListDoubleValue) {
    throw std::runtime_error("Signal " + name +
                             " is not of proton::list_double type");
  }
  signal->mutable_list_double_value()->mutable_doubles()->Assign(value.begin(),
                                                                 value.end());
}

template <>
void SignalHandle::setValue<proton::list_float>(proton::list_float value) {
  if (type != proton::Signal::SignalCase::kListFloatValue) {
    throw std::runtime_error("Signal " + name +
                             " is not of proton::list_float type");
  }
  signal->mutable_list_float_value()->mutable_floats()->Assign(value.begin(),
                                                               value.end());
}

template <>
void SignalHandle::setValue<proton::list_int32>(proton::list_int32 value) {
  if (type != proton::Signal::SignalCase::kListInt32Value) {
    throw std::runtime_error("Signal " + name +
                             " is not of proton::list_int32 type");
  }
  signal->mutable_list_int32_value()->mutable_int32s()->Assign(value.begin(),
                                                               value.end());
}

template <>
void SignalHandle::setValue<proton::list_int64>(proton::list_int64 value) {
  if (type != proton::Signal::SignalCase::kListInt64Value) {
    throw std::runtime_error("Signal " + name +
                             " is not of proton::list_int64 type");
  }
  signal->mutable_list_int64_value()->mutable_int64s()->Assign(value.begin(),
                                                               value.end());
}

template <>
void SignalHandle::setValue<proton::list_uint32>(proton::list_uint32 value) {
  if (type != proton::Signal::SignalCase::kListUint32Value) {
    throw std::runtime_error("Signal " + name +
                             " is not of proton::list_uint32 type");
  }
  signal->mutable_list_uint32_value()->mutable_uint32s()->Assign(value.begin(),
                                                                 value.end());
}

template <>
void SignalHandle::setValue<proton::list_uint64>(proton::list_uint64 value) {
  if (type != proton::Signal::SignalCase::kListUint64Value) {
    throw std::runtime_error("Signal " + name +
                             " is not of proton::list_uint64 type");
  }
  signal->mutable_list_uint64_value()->mutable_uint64s()->Assign(value.begin(),
                                                                 value.end());
}

template <>
void SignalHandle::setValue<proton::list_bool>(proton::list_bool value) {
  if (type != proton::Signal::SignalCase::kListBoolValue) {
    throw std::runtime_error("Signal " + name +
                             " is not of proton::list_bool type");
  }
  signal->mutable_list_bool_value()->mutable_bools()->Assign(value.begin(),
                                                             value.end());
}

template <>
void SignalHandle::setValue<proton::list_string>(proton::list_string value) {
  if (type != proton::Signal::SignalCase::kListStringValue) {
    throw std::runtime_error("Signal " + name +
                             " is not of proton::list_string type");
  }
  signal->mutable_list_string_value()->mutable_strings()->Assign(value.begin(),
                                                                 value.end());
}
