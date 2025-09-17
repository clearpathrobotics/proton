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

SignalHandle::SignalHandle(SignalConfig config, std::string bundle_name,
                           std::shared_ptr<Signal> ptr) {
  name_ = config.name;
  bundle_name_ = bundle_name;
  length_ = config.length;
  capacity_ = config.capacity;
  const_ = config.is_const;

  try {
    type_ = signal_map::SignalMap.at(config.type_string);
  } catch (std::out_of_range &e) {
    throw std::runtime_error("Invalid signal type " + config.type_string +
                             " for signal " + name_);
  }

  signal_ = ptr;

  switch (type_) {
  case Signal::SignalCase::kDoubleValue: {
    if (const_) {
      signal_->set_double_value(config.value.as<double>());
    } else {
      signal_->set_double_value(proton::default_values::DOUBLE);
    }
    break;
  }

  case Signal::SignalCase::kFloatValue: {
    if (const_) {
      signal_->set_float_value(config.value.as<float>());
    } else {
      signal_->set_float_value(proton::default_values::FLOAT);
    }
    break;
  }

  case Signal::SignalCase::kInt32Value: {
    if (const_) {
      signal_->set_int32_value(config.value.as<int32_t>());
    } else {
      signal_->set_int32_value(proton::default_values::INT32);
    }
    break;
  }

  case Signal::SignalCase::kInt64Value: {
    if (const_) {
      signal_->set_int64_value(config.value.as<int64_t>());
    } else {
      signal_->set_int64_value(proton::default_values::INT64);
    }
    break;
  }

  case Signal::SignalCase::kUint32Value: {
    if (const_) {
      signal_->set_uint32_value(config.value.as<uint32_t>());
    } else {
      signal_->set_uint32_value(proton::default_values::UINT32);
    }
    break;
  }

  case Signal::SignalCase::kUint64Value: {
    if (const_) {
      signal_->set_uint64_value(config.value.as<uint64_t>());
    } else {
      signal_->set_uint64_value(proton::default_values::UINT64);
    }
    break;
  }

  case Signal::SignalCase::kBoolValue: {
    if (const_) {
      signal_->set_bool_value(config.value.as<bool>());
    } else {
      signal_->set_bool_value(proton::default_values::BOOL);
    }
    break;
  }

  case Signal::SignalCase::kStringValue: {
    auto str = signal_->mutable_string_value();
    if (const_) {
      str->assign(config.value.as<std::string>());
    } else {
      str->resize(capacity_);
    }
    break;
  }

  case Signal::SignalCase::kBytesValue: {
    auto bytes = signal_->mutable_bytes_value();
    bytes->resize(capacity_);
    if (const_) {
      auto v = config.value.as<proton::bytes>();
      bytes->assign(v.begin(), v.end());
    }
    break;
  }

  case Signal::SignalCase::kListDoubleValue: {
    auto *list = signal_->mutable_list_double_value();
    list->mutable_doubles()->Resize(length_, proton::default_values::DOUBLE);
    if (const_) {
      auto v = config.value.as<proton::list_double>();
      list->mutable_doubles()->Assign(v.begin(), v.end());
    }
    break;
  }

  case Signal::SignalCase::kListFloatValue: {
    auto *list = signal_->mutable_list_float_value();
    list->mutable_floats()->Resize(length_, proton::default_values::FLOAT);
    if (const_) {
      auto v = config.value.as<proton::list_float>();
      list->mutable_floats()->Assign(v.begin(), v.end());
    }
    break;
  }

  case Signal::SignalCase::kListInt32Value: {
    auto *list = signal_->mutable_list_int32_value();
    list->mutable_int32s()->Resize(length_, proton::default_values::INT32);
    if (const_) {
      auto v = config.value.as<proton::list_int32>();
      list->mutable_int32s()->Assign(v.begin(), v.end());
    }
    break;
  }

  case Signal::SignalCase::kListInt64Value: {
    auto *list = signal_->mutable_list_int64_value();
    list->mutable_int64s()->Resize(length_, proton::default_values::INT64);
    if (const_) {
      auto v = config.value.as<proton::list_int64>();
      list->mutable_int64s()->Assign(v.begin(), v.end());
    }
    break;
  }

  case Signal::SignalCase::kListUint32Value: {
    auto *list = signal_->mutable_list_uint32_value();
    list->mutable_uint32s()->Resize(length_, proton::default_values::UINT32);
    if (const_) {
      auto v = config.value.as<proton::list_uint32>();
      list->mutable_uint32s()->Assign(v.begin(), v.end());
    }
    break;
  }

  case Signal::SignalCase::kListUint64Value: {
    auto *list = signal_->mutable_list_uint64_value();
    list->mutable_uint64s()->Resize(length_, proton::default_values::UINT64);
    if (const_) {
      auto v = config.value.as<proton::list_uint64>();
      list->mutable_uint64s()->Assign(v.begin(), v.end());
    }
    break;
  }

  case Signal::SignalCase::kListBoolValue: {
    auto *list = signal_->mutable_list_bool_value();
    list->mutable_bools()->Resize(length_, proton::default_values::BOOL);
    if (const_) {
      auto v = config.value.as<proton::list_bool>();
      list->mutable_bools()->Assign(v.begin(), v.end());
    }
    break;
  }

  case Signal::SignalCase::kListStringValue: {
    auto *list = signal_->mutable_list_string_value();

    for (uint32_t i = 0; i < length_; i++) {
      list->add_strings("");
    }

    if (const_) {
      auto v = config.value.as<proton::list_string>();
      for (uint32_t i = 0; i < length_; i++) {
        list->set_strings(i, v.at(i));
      }
    }
    break;
  }

  default:
    break;
  }
}

template <> const double SignalHandle::getValue<double>() const {
  if (type_ != proton::Signal::SignalCase::kDoubleValue) {
    throw std::runtime_error("Signal " + name_ + " is not of double type");
  }
  return signal_->double_value();
}

template <> const float SignalHandle::getValue<float>() const {
  if (type_ != proton::Signal::SignalCase::kFloatValue) {
    throw std::runtime_error("Signal " + name_ + " is not of float type");
  }
  return signal_->float_value();
}

template <> const int32_t SignalHandle::getValue<int32_t>() const {
  if (type_ != proton::Signal::SignalCase::kInt32Value) {
    throw std::runtime_error("Signal " + name_ + " is not of int32_t type");
  }
  return signal_->int32_value();
}

template <> const int64_t SignalHandle::getValue<int64_t>() const {
  if (type_ != proton::Signal::SignalCase::kInt64Value) {
    throw std::runtime_error("Signal " + name_ + " is not of int64_t type");
  }
  return signal_->int64_value();
}

template <> const uint32_t SignalHandle::getValue<uint32_t>() const {
  if (type_ != proton::Signal::SignalCase::kUint32Value) {
    throw std::runtime_error("Signal " + name_ + " is not of uint32_t type");
  }
  return signal_->uint32_value();
}

template <> const uint64_t SignalHandle::getValue<uint64_t>() const {
  if (type_ != proton::Signal::SignalCase::kUint64Value) {
    throw std::runtime_error("Signal " + name_ + " is not of uint64_t type");
  }
  return signal_->uint64_value();
}

template <> const bool SignalHandle::getValue<bool>() const {
  if (type_ != proton::Signal::SignalCase::kBoolValue) {
    throw std::runtime_error("Signal " + name_ + " is not of bool type");
  }
  return signal_->bool_value();
}

template <> const std::string SignalHandle::getValue<std::string>() const {
  if (type_ != proton::Signal::SignalCase::kStringValue) {
    throw std::runtime_error("Signal " + name_ + " is not of std::string type");
  }
  return signal_->string_value();
}

template <> const proton::bytes SignalHandle::getValue<proton::bytes>() const {
  if (type_ != proton::Signal::SignalCase::kBytesValue) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::bytes type");
  }
  const std::string &bytes = signal_->bytes_value();
  return {bytes.begin(), bytes.end()};
}

template <>
const proton::list_double SignalHandle::getValue<proton::list_double>() const {
  if (type_ != proton::Signal::SignalCase::kListDoubleValue) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::list_double type");
  }
  return {signal_->list_double_value().doubles().begin(),
          signal_->list_double_value().doubles().end()};
}

template <>
const proton::list_float SignalHandle::getValue<proton::list_float>() const {
  if (type_ != proton::Signal::SignalCase::kListFloatValue) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::list_float type");
  }
  return {signal_->list_float_value().floats().begin(),
          signal_->list_float_value().floats().end()};
}

template <>
const proton::list_int32 SignalHandle::getValue<proton::list_int32>() const {
  if (type_ != proton::Signal::SignalCase::kListInt32Value) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::list_int32 type");
  }
  return {signal_->list_int32_value().int32s().begin(),
          signal_->list_int32_value().int32s().end()};
}

template <>
const proton::list_int64 SignalHandle::getValue<proton::list_int64>() const {
  if (type_ != proton::Signal::SignalCase::kListInt64Value) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::list_int64 type");
  }
  return {signal_->list_int64_value().int64s().begin(),
          signal_->list_int64_value().int64s().end()};
}

template <>
const proton::list_uint32 SignalHandle::getValue<proton::list_uint32>() const {
  if (type_ != proton::Signal::SignalCase::kListUint32Value) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::list_uint32 type");
  }
  return {signal_->list_uint32_value().uint32s().begin(),
          signal_->list_uint32_value().uint32s().end()};
}

template <>
const proton::list_uint64 SignalHandle::getValue<proton::list_uint64>() const {
  if (type_ != proton::Signal::SignalCase::kListUint64Value) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::list_uint64 type");
  }
  return {signal_->list_uint64_value().uint64s().begin(),
          signal_->list_uint64_value().uint64s().end()};
}

template <>
const proton::list_bool SignalHandle::getValue<proton::list_bool>() const {
  if (type_ != proton::Signal::SignalCase::kListBoolValue) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::list_bool type");
  }
  return {signal_->list_bool_value().bools().begin(),
          signal_->list_bool_value().bools().end()};
}

template <>
const proton::list_string SignalHandle::getValue<proton::list_string>() const {
  if (type_ != proton::Signal::SignalCase::kListStringValue) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::list_string type");
  }
  return {signal_->list_string_value().strings().begin(),
          signal_->list_string_value().strings().end()};
}

template <> void SignalHandle::setValue<double>(const double value) {
  if (type_ != proton::Signal::SignalCase::kDoubleValue) {
    throw std::runtime_error("Signal " + name_ + " is not of double type");
  }
  if (const_) {
    throw std::runtime_error("Signal " + name_ +
                             " is constant and cannot be set");
  }
  signal_->set_double_value(value);
}

template <> void SignalHandle::setValue<float>(const float value) {
  if (type_ != proton::Signal::SignalCase::kFloatValue) {
    throw std::runtime_error("Signal " + name_ + " is not of float type");
  }
  if (const_) {
    throw std::runtime_error("Signal " + name_ +
                             " is constant and cannot be set");
  }
  signal_->set_float_value(value);
}

template <> void SignalHandle::setValue<int32_t>(const int32_t value) {
  if (type_ != proton::Signal::SignalCase::kInt32Value) {
    throw std::runtime_error("Signal " + name_ + " is not of int32_t type");
  }
  if (const_) {
    throw std::runtime_error("Signal " + name_ +
                             " is constant and cannot be set");
  }
  signal_->set_int32_value(value);
}

template <> void SignalHandle::setValue<int64_t>(const int64_t value) {
  if (type_ != proton::Signal::SignalCase::kInt64Value) {
    throw std::runtime_error("Signal " + name_ + " is not of int64_t type");
  }
  if (const_) {
    throw std::runtime_error("Signal " + name_ +
                             " is constant and cannot be set");
  }
  signal_->set_int64_value(value);
}

template <> void SignalHandle::setValue<uint32_t>(const uint32_t value) {
  if (type_ != proton::Signal::SignalCase::kUint32Value) {
    throw std::runtime_error("Signal " + name_ + " is not of uint32_t type");
  }
  if (const_) {
    throw std::runtime_error("Signal " + name_ +
                             " is constant and cannot be set");
  }
  signal_->set_uint32_value(value);
}

template <> void SignalHandle::setValue<uint64_t>(const uint64_t value) {
  if (type_ != proton::Signal::SignalCase::kUint64Value) {
    throw std::runtime_error("Signal " + name_ + " is not of uint64_t type");
  }
  if (const_) {
    throw std::runtime_error("Signal " + name_ +
                             " is constant and cannot be set");
  }
  signal_->set_uint64_value(value);
}

template <> void SignalHandle::setValue<bool>(const bool value) {
  if (type_ != proton::Signal::SignalCase::kBoolValue) {
    throw std::runtime_error("Signal " + name_ + " is not of bool type");
  }
  if (const_) {
    throw std::runtime_error("Signal " + name_ +
                             " is constant and cannot be set");
  }
  signal_->set_bool_value(value);
}

template <> void SignalHandle::setValue<std::string>(const std::string value) {
  if (type_ != proton::Signal::SignalCase::kStringValue) {
    throw std::runtime_error("Signal " + name_ + " is not of std::string type");
  }
  if (const_) {
    throw std::runtime_error("Signal " + name_ +
                             " is constant and cannot be set");
  }
  signal_->set_string_value(value);
}

template <>
void SignalHandle::setValue<proton::bytes>(const proton::bytes value) {
  if (type_ != proton::Signal::SignalCase::kBytesValue) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::bytes type");
  }
  if (const_) {
    throw std::runtime_error("Signal " + name_ +
                             " is constant and cannot be set");
  }
  signal_->mutable_bytes_value()->assign(value.begin(), value.end());
}

template <>
void SignalHandle::setValue<proton::list_double>(
    const proton::list_double value) {
  if (type_ != proton::Signal::SignalCase::kListDoubleValue) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::list_double type");
  }
  if (const_) {
    throw std::runtime_error("Signal " + name_ +
                             " is constant and cannot be set");
  }
  signal_->mutable_list_double_value()->mutable_doubles()->Assign(value.begin(),
                                                                  value.end());
}

template <>
void SignalHandle::setValue<proton::list_float>(
    const proton::list_float value) {
  if (type_ != proton::Signal::SignalCase::kListFloatValue) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::list_float type");
  }
  if (const_) {
    throw std::runtime_error("Signal " + name_ +
                             " is constant and cannot be set");
  }
  signal_->mutable_list_float_value()->mutable_floats()->Assign(value.begin(),
                                                                value.end());
}

template <>
void SignalHandle::setValue<proton::list_int32>(
    const proton::list_int32 value) {
  if (type_ != proton::Signal::SignalCase::kListInt32Value) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::list_int32 type");
  }
  if (const_) {
    throw std::runtime_error("Signal " + name_ +
                             " is constant and cannot be set");
  }
  signal_->mutable_list_int32_value()->mutable_int32s()->Assign(value.begin(),
                                                                value.end());
}

template <>
void SignalHandle::setValue<proton::list_int64>(
    const proton::list_int64 value) {
  if (type_ != proton::Signal::SignalCase::kListInt64Value) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::list_int64 type");
  }
  if (const_) {
    throw std::runtime_error("Signal " + name_ +
                             " is constant and cannot be set");
  }
  signal_->mutable_list_int64_value()->mutable_int64s()->Assign(value.begin(),
                                                                value.end());
}

template <>
void SignalHandle::setValue<proton::list_uint32>(
    const proton::list_uint32 value) {
  if (type_ != proton::Signal::SignalCase::kListUint32Value) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::list_uint32 type");
  }
  if (const_) {
    throw std::runtime_error("Signal " + name_ +
                             " is constant and cannot be set");
  }
  signal_->mutable_list_uint32_value()->mutable_uint32s()->Assign(value.begin(),
                                                                  value.end());
}

template <>
void SignalHandle::setValue<proton::list_uint64>(
    const proton::list_uint64 value) {
  if (type_ != proton::Signal::SignalCase::kListUint64Value) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::list_uint64 type");
  }
  if (const_) {
    throw std::runtime_error("Signal " + name_ +
                             " is constant and cannot be set");
  }
  signal_->mutable_list_uint64_value()->mutable_uint64s()->Assign(value.begin(),
                                                                  value.end());
}

template <>
void SignalHandle::setValue<proton::list_bool>(const proton::list_bool value) {
  if (type_ != proton::Signal::SignalCase::kListBoolValue) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::list_bool type");
  }
  if (const_) {
    throw std::runtime_error("Signal " + name_ +
                             " is constant and cannot be set");
  }
  signal_->mutable_list_bool_value()->mutable_bools()->Assign(value.begin(),
                                                              value.end());
}

template <>
void SignalHandle::setValue<proton::list_string>(
    const proton::list_string value) {
  if (type_ != proton::Signal::SignalCase::kListStringValue) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::list_string type");
  }
  if (const_) {
    throw std::runtime_error("Signal " + name_ +
                             " is constant and cannot be set");
  }
  signal_->mutable_list_string_value()->mutable_strings()->Assign(value.begin(),
                                                                  value.end());
}

template <>
void SignalHandle::setValue<double>(uint16_t index, const double value) {
  if (type_ != proton::Signal::SignalCase::kListDoubleValue) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::list_double type");
  }
  if (index >= length_) {
    throw std::out_of_range("Index " + std::to_string(index) + " out of range");
  }
  if (const_) {
    throw std::runtime_error("Signal " + name_ +
                             " is constant and cannot be set");
  }
  signal_->mutable_list_double_value()->mutable_doubles()->Set(index, value);
}

template <>
void SignalHandle::setValue<float>(uint16_t index, const float value) {
  if (type_ != proton::Signal::SignalCase::kListFloatValue) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::list_float type");
  }
  if (index >= length_) {
    throw std::out_of_range("Index " + std::to_string(index) + " out of range");
  }
  if (const_) {
    throw std::runtime_error("Signal " + name_ +
                             " is constant and cannot be set");
  }
  signal_->mutable_list_float_value()->mutable_floats()->Set(index, value);
}

template <>
void SignalHandle::setValue<int32_t>(uint16_t index, int32_t value) {
  if (type_ != proton::Signal::SignalCase::kListInt32Value) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::list_int32 type");
  }
  if (index >= length_) {
    throw std::out_of_range("Index " + std::to_string(index) + " out of range");
  }
  if (const_) {
    throw std::runtime_error("Signal " + name_ + " is constant and cannot be set");
  }
  signal_->mutable_list_int32_value()->mutable_int32s()->Set(index, value);
}

template <>
void SignalHandle::setValue<int64_t>(uint16_t index, const int64_t value) {
  if (type_ != proton::Signal::SignalCase::kListInt64Value) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::list_int64 type");
  }
  if (index >= length_) {
    throw std::out_of_range("Index " + std::to_string(index) + " out of range");
  }
  if (const_) {
    throw std::runtime_error("Signal " + name_ + " is constant and cannot be set");
  }
  signal_->mutable_list_int64_value()->mutable_int64s()->Set(index, value);
}

template <>
void SignalHandle::setValue<uint32_t>(uint16_t index, const uint32_t value) {
  if (type_ != proton::Signal::SignalCase::kListUint32Value) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::list_uint32 type");
  }
  if (index >= length_) {
    throw std::out_of_range("Index " + std::to_string(index) + " out of range");
  }
  if (const_) {
    throw std::runtime_error("Signal " + name_ + " is constant and cannot be set");
  }
  signal_->mutable_list_uint32_value()->mutable_uint32s()->Set(index, value);
}

template <>
void SignalHandle::setValue<uint64_t>(uint16_t index, const uint64_t value) {
  if (type_ != proton::Signal::SignalCase::kListUint64Value) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::list_uint64 type");
  }
  if (index >= length_) {
    throw std::out_of_range("Index " + std::to_string(index) + " out of range");
  }
  if (const_) {
    throw std::runtime_error("Signal " + name_ + " is constant and cannot be set");
  }
  signal_->mutable_list_uint64_value()->mutable_uint64s()->Set(index, value);
}

template <>
void SignalHandle::setValue<bool>(uint16_t index, const bool value) {
  if (type_ != proton::Signal::SignalCase::kListBoolValue) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::list_bool type");
  }
  if (index >= length_) {
    throw std::out_of_range("Index " + std::to_string(index) + " out of range");
  }
  if (const_) {
    throw std::runtime_error("Signal " + name_ + " is constant and cannot be set");
  }
  signal_->mutable_list_bool_value()->mutable_bools()->Set(index, value);
}

template <>
void SignalHandle::setValue<std::string>(uint16_t index,
                                         const std::string value) {
  if (type_ != proton::Signal::SignalCase::kListStringValue) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::list_string type");
  }
  if (index >= length_) {
    throw std::out_of_range("Index " + std::to_string(index) + " out of range");
  }
  if (const_) {
    throw std::runtime_error("Signal " + name_ + " is constant and cannot be set");
  }
  *signal_->mutable_list_string_value()->mutable_strings(index) = value;
}

template <>
void SignalHandle::setValue<uint8_t>(uint16_t index, const uint8_t value) {
  if (type_ != proton::Signal::SignalCase::kBytesValue) {
    throw std::runtime_error("Signal " + name_ +
                             " is not of proton::bytes type");
  }
  if (index >= capacity_) {
    throw std::out_of_range("Index " + std::to_string(index) + " out of range");
  }
  if (const_) {
    throw std::runtime_error("Signal " + name_ + " is constant and cannot be set");
  }
  (*signal_->mutable_bytes_value())[index] = value;
}
