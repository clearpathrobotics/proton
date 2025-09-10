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

#include "protoncpp/signal.hpp"

using namespace proton;

SignalHandle::SignalHandle(SignalConfig config, std::shared_ptr<Signal> ptr) {
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
    throw std::runtime_error("Invalid signal type " + config.type_string +
                             " for signal " + name_);
  }

  signal_ = ptr;

  switch (type_) {
  case Signal::SignalCase::kDoubleValue: {
    signal_->set_double_value(0.0);
    break;
  }

  case Signal::SignalCase::kFloatValue: {
    signal_->set_float_value(0.0f);
    break;
  }

  case Signal::SignalCase::kInt32Value: {
    signal_->set_int32_value(0);
    break;
  }

  case Signal::SignalCase::kInt64Value: {
    signal_->set_int64_value(0);
    break;
  }

  case Signal::SignalCase::kUint32Value: {
    signal_->set_uint32_value(0);
    break;
  }

  case Signal::SignalCase::kUint64Value: {
    signal_->set_uint64_value(0);
    break;
  }

  case Signal::SignalCase::kBoolValue: {
    signal_->set_bool_value(false);
    break;
  }

  case Signal::SignalCase::kStringValue: {
    auto str = signal_->mutable_string_value();
    str->resize(capacity_);
    break;
  }

  case Signal::SignalCase::kBytesValue: {
    auto bytes = signal_->mutable_bytes_value();
    bytes->resize(capacity_);
    break;
  }

  case Signal::SignalCase::kListDoubleValue: {
    auto *list = signal_->mutable_list_double_value();
    list->mutable_doubles()->Resize(length_, 0.0);
    break;
  }

  case Signal::SignalCase::kListFloatValue: {
    auto *list = signal_->mutable_list_float_value();
    list->mutable_floats()->Resize(length_, 0.0f);
    break;
  }

  case Signal::SignalCase::kListInt32Value: {
    auto *list = signal_->mutable_list_int32_value();
    list->mutable_int32s()->Resize(length_, 0);
    break;
  }

  case Signal::SignalCase::kListInt64Value: {
    auto *list = signal_->mutable_list_int64_value();
    list->mutable_int64s()->Resize(length_, 0);
    break;
  }

  case Signal::SignalCase::kListUint32Value: {
    auto *list = signal_->mutable_list_uint32_value();
    list->mutable_uint32s()->Resize(length_, 0);
    break;
  }

  case Signal::SignalCase::kListUint64Value: {
    auto *list = signal_->mutable_list_uint64_value();
    list->mutable_uint64s()->Resize(length_, 0);
    break;
  }

  case Signal::SignalCase::kListBoolValue: {
    auto *list = signal_->mutable_list_bool_value();
    list->mutable_bools()->Resize(length_, false);
    break;
  }

  case Signal::SignalCase::kListStringValue: {
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

template <> double SignalHandle::getValue<double>() {
  if (type_ != proton::Signal::SignalCase::kDoubleValue) {
    throw std::runtime_error("Signal " + name_ + " is not of double type");
  }
  return signal_->double_value();
}

template <> float SignalHandle::getValue<float>() {
  if (type_ != proton::Signal::SignalCase::kFloatValue) {
    throw std::runtime_error("Signal " + name_ + " is not of float type");
  }
  return signal_->float_value();
}

template <> int32_t SignalHandle::getValue<int32_t>() {
  if (type_ != proton::Signal::SignalCase::kInt32Value) {
    throw std::runtime_error("Signal " + name_ + " is not of int32_t type");
  }
  return signal_->int32_value();
}

template <> int64_t SignalHandle::getValue<int64_t>() {
  if (type_ != proton::Signal::SignalCase::kInt64Value) {
    throw std::runtime_error("Signal " + name_ + " is not of int64_t type");
  }
  return signal_->int64_value();
}

template <> uint32_t SignalHandle::getValue<uint32_t>() {
  if (type_ != proton::Signal::SignalCase::kUint32Value) {
    throw std::runtime_error("Signal " + name_ + " is not of uint32_t type");
  }
  return signal_->uint32_value();
}

template <> uint64_t SignalHandle::getValue<uint64_t>() {
  if (type_ != proton::Signal::SignalCase::kUint64Value) {
    throw std::runtime_error("Signal " + name_ + " is not of uint64_t type");
  }
  return signal_->uint64_value();
}

template <> bool SignalHandle::getValue<bool>() {
  if (type_ != proton::Signal::SignalCase::kBoolValue) {
    throw std::runtime_error("Signal " + name_ + " is not of bool type");
  }
  return signal_->bool_value();
}

template <> std::string SignalHandle::getValue<std::string>() {
  if (type_ != proton::Signal::SignalCase::kStringValue) {
    throw std::runtime_error("Signal " + name_ + " is not of std::string type");
  }
  return signal_->string_value();
}

template <> proton::bytes SignalHandle::getValue<proton::bytes>() {
  if (type_ != proton::Signal::SignalCase::kBytesValue) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::bytes type");
  }
  const std::string &bytes = signal_->bytes_value();
  return {bytes.begin(), bytes.end()};
}

template <> proton::list_double SignalHandle::getValue<proton::list_double>() {
  if (type_ != proton::Signal::SignalCase::kListDoubleValue) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::list_double type");
  }
  return {signal_->list_double_value().doubles().begin(),
          signal_->list_double_value().doubles().end()};
}

template <> proton::list_float SignalHandle::getValue<proton::list_float>() {
  if (type_ != proton::Signal::SignalCase::kListFloatValue) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::list_float type");
  }
  return {signal_->list_float_value().floats().begin(),
          signal_->list_float_value().floats().end()};
}

template <> proton::list_int32 SignalHandle::getValue<proton::list_int32>() {
  if (type_ != proton::Signal::SignalCase::kListInt32Value) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::list_int32 type");
  }
  return {signal_->list_int32_value().int32s().begin(),
          signal_->list_int32_value().int32s().end()};
}

template <> proton::list_int64 SignalHandle::getValue<proton::list_int64>() {
  if (type_ != proton::Signal::SignalCase::kListInt64Value) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::list_int64 type");
  }
  return {signal_->list_int64_value().int64s().begin(),
          signal_->list_int64_value().int64s().end()};
}

template <> proton::list_uint32 SignalHandle::getValue<proton::list_uint32>() {
  if (type_ != proton::Signal::SignalCase::kListUint32Value) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::list_uint32 type");
  }
  return {signal_->list_uint32_value().uint32s().begin(),
          signal_->list_uint32_value().uint32s().end()};
}

template <> proton::list_uint64 SignalHandle::getValue<proton::list_uint64>() {
  if (type_ != proton::Signal::SignalCase::kListUint64Value) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::list_uint64 type");
  }
  return {signal_->list_uint64_value().uint64s().begin(),
          signal_->list_uint64_value().uint64s().end()};
}

template <> proton::list_bool SignalHandle::getValue<proton::list_bool>() {
  if (type_ != proton::Signal::SignalCase::kListBoolValue) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::list_bool type");
  }
  return {signal_->list_bool_value().bools().begin(),
          signal_->list_bool_value().bools().end()};
}

template <> proton::list_string SignalHandle::getValue<proton::list_string>() {
  if (type_ != proton::Signal::SignalCase::kListStringValue) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::list_string type");
  }
  return {signal_->list_string_value().strings().begin(),
          signal_->list_string_value().strings().end()};
}

template <> void SignalHandle::setValue<double>(double value) {
  if (type_ != proton::Signal::SignalCase::kDoubleValue) {
    throw std::runtime_error("Signal " + name_ + " is not of double type");
  }
  signal_->set_double_value(value);
}

template <> void SignalHandle::setValue<float>(float value) {
  if (type_ != proton::Signal::SignalCase::kFloatValue) {
    throw std::runtime_error("Signal " + name_ + " is not of float type");
  }
  signal_->set_float_value(value);
}

template <> void SignalHandle::setValue<int32_t>(int32_t value) {
  if (type_ != proton::Signal::SignalCase::kInt32Value) {
    throw std::runtime_error("Signal " + name_ + " is not of int32_t type");
  }
  signal_->set_int32_value(value);
}

template <> void SignalHandle::setValue<int64_t>(int64_t value) {
  if (type_ != proton::Signal::SignalCase::kInt64Value) {
    throw std::runtime_error("Signal " + name_ + " is not of int64_t type");
  }
  signal_->set_int64_value(value);
}

template <> void SignalHandle::setValue<uint32_t>(uint32_t value) {
  if (type_ != proton::Signal::SignalCase::kUint32Value) {
    throw std::runtime_error("Signal " + name_ + " is not of uint32_t type");
  }
  signal_->set_uint32_value(value);
}

template <> void SignalHandle::setValue<uint64_t>(uint64_t value) {
  if (type_ != proton::Signal::SignalCase::kUint64Value) {
    throw std::runtime_error("Signal " + name_ + " is not of uint64_t type");
  }
  signal_->set_uint64_value(value);
}

template <> void SignalHandle::setValue<bool>(bool value) {
  if (type_ != proton::Signal::SignalCase::kBoolValue) {
    throw std::runtime_error("Signal " + name_ + " is not of bool type");
  }
  signal_->set_bool_value(value);
}

template <> void SignalHandle::setValue<std::string>(std::string value) {
  if (type_ != proton::Signal::SignalCase::kStringValue) {
    throw std::runtime_error("Signal " + name_ + " is not of std::string type");
  }
  signal_->set_string_value(value);
}

template <> void SignalHandle::setValue<proton::bytes>(proton::bytes value) {
  if (type_ != proton::Signal::SignalCase::kBytesValue) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::bytes type");
  }
  signal_->mutable_bytes_value()->assign(value.begin(), value.end());
}

template <>
void SignalHandle::setValue<proton::list_double>(proton::list_double value) {
  if (type_ != proton::Signal::SignalCase::kListDoubleValue) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::list_double type");
  }
  signal_->mutable_list_double_value()->mutable_doubles()->Assign(value.begin(),
                                                                  value.end());
}

template <>
void SignalHandle::setValue<proton::list_float>(proton::list_float value) {
  if (type_ != proton::Signal::SignalCase::kListFloatValue) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::list_float type");
  }
  signal_->mutable_list_float_value()->mutable_floats()->Assign(value.begin(),
                                                                value.end());
}

template <>
void SignalHandle::setValue<proton::list_int32>(proton::list_int32 value) {
  if (type_ != proton::Signal::SignalCase::kListInt32Value) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::list_int32 type");
  }
  signal_->mutable_list_int32_value()->mutable_int32s()->Assign(value.begin(),
                                                                value.end());
}

template <>
void SignalHandle::setValue<proton::list_int64>(proton::list_int64 value) {
  if (type_ != proton::Signal::SignalCase::kListInt64Value) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::list_int64 type");
  }
  signal_->mutable_list_int64_value()->mutable_int64s()->Assign(value.begin(),
                                                                value.end());
}

template <>
void SignalHandle::setValue<proton::list_uint32>(proton::list_uint32 value) {
  if (type_ != proton::Signal::SignalCase::kListUint32Value) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::list_uint32 type");
  }
  signal_->mutable_list_uint32_value()->mutable_uint32s()->Assign(value.begin(),
                                                                  value.end());
}

template <>
void SignalHandle::setValue<proton::list_uint64>(proton::list_uint64 value) {
  if (type_ != proton::Signal::SignalCase::kListUint64Value) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::list_uint64 type");
  }
  signal_->mutable_list_uint64_value()->mutable_uint64s()->Assign(value.begin(),
                                                                  value.end());
}

template <>
void SignalHandle::setValue<proton::list_bool>(proton::list_bool value) {
  if (type_ != proton::Signal::SignalCase::kListBoolValue) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::list_bool type");
  }
  signal_->mutable_list_bool_value()->mutable_bools()->Assign(value.begin(),
                                                              value.end());
}

template <>
void SignalHandle::setValue<proton::list_string>(proton::list_string value) {
  if (type_ != proton::Signal::SignalCase::kListStringValue) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::list_string type");
  }
  signal_->mutable_list_string_value()->mutable_strings()->Assign(value.begin(),
                                                                  value.end());
}
