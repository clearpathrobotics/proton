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

#include "protoncpp/signal.hpp"

using namespace proton;

SignalHandle::SignalHandle(SignalConfig config, std::string bundle_name, Signal *signal) {
  name_ = config.name;
  bundle_name_ = bundle_name;
  capacity_ = config.capacity;
  const_ = config.is_const;

  try {
    type_ = signal_map::SignalMap.at(config.type_string);
  } catch (std::out_of_range &e) {
    throw std::runtime_error("Invalid signal type " + config.type_string + " for signal " + name_);
  }

  signal_ = signal;

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

    default:
      break;
  }
}

SignalHandle::~SignalHandle() {
  if (const_) {
    // Const signals are created with new, rather than from a Bundle.
    // delete signal_;
  }
}

template <>
const double SignalHandle::getValue<double>() const {
  if (type_ != proton::Signal::SignalCase::kDoubleValue) {
    throw std::runtime_error("Signal " + name_ + " is not of double type");
  }
  return signal_->double_value();
}

template <>
const float SignalHandle::getValue<float>() const {
  if (type_ != proton::Signal::SignalCase::kFloatValue) {
    throw std::runtime_error("Signal " + name_ + " is not of float type");
  }
  return signal_->float_value();
}

template <>
const int32_t SignalHandle::getValue<int32_t>() const {
  if (type_ != proton::Signal::SignalCase::kInt32Value) {
    throw std::runtime_error("Signal " + name_ + " is not of int32_t type");
  }
  return signal_->int32_value();
}

template <>
const int64_t SignalHandle::getValue<int64_t>() const {
  if (type_ != proton::Signal::SignalCase::kInt64Value) {
    throw std::runtime_error("Signal " + name_ + " is not of int64_t type");
  }
  return signal_->int64_value();
}

template <>
const uint32_t SignalHandle::getValue<uint32_t>() const {
  if (type_ != proton::Signal::SignalCase::kUint32Value) {
    throw std::runtime_error("Signal " + name_ + " is not of uint32_t type");
  }
  return signal_->uint32_value();
}

template <>
const uint64_t SignalHandle::getValue<uint64_t>() const {
  if (type_ != proton::Signal::SignalCase::kUint64Value) {
    throw std::runtime_error("Signal " + name_ + " is not of uint64_t type");
  }
  return signal_->uint64_value();
}

template <>
const bool SignalHandle::getValue<bool>() const {
  if (type_ != proton::Signal::SignalCase::kBoolValue) {
    throw std::runtime_error("Signal " + name_ + " is not of bool type");
  }
  return signal_->bool_value();
}

template <>
const std::string SignalHandle::getValue<std::string>() const {
  if (type_ != proton::Signal::SignalCase::kStringValue) {
    throw std::runtime_error("Signal " + name_ + " is not of std::string type");
  }
  return signal_->string_value();
}

template <>
const proton::bytes SignalHandle::getValue<proton::bytes>() const {
  if (type_ != proton::Signal::SignalCase::kBytesValue) {
    throw std::runtime_error("Signal " + name_ + " is not of proton::bytes type");
  }
  const std::string &bytes = signal_->bytes_value();
  return {bytes.begin(), bytes.end()};
}

template <>
void SignalHandle::setValue<double>(const double value) {
  if (type_ != proton::Signal::SignalCase::kDoubleValue) {
    throw std::runtime_error("Signal " + name_ + " is not of double type");
  }
  if (const_) {
    throw std::runtime_error("Signal " + name_ + " is constant and cannot be set");
  }
  signal_->set_double_value(value);
}

template <>
void SignalHandle::setValue<float>(const float value) {
  if (type_ != proton::Signal::SignalCase::kFloatValue) {
    throw std::runtime_error("Signal " + name_ + " is not of float type");
  }
  if (const_) {
    throw std::runtime_error("Signal " + name_ + " is constant and cannot be set");
  }
  signal_->set_float_value(value);
}

template <>
void SignalHandle::setValue<int32_t>(const int32_t value) {
  if (type_ != proton::Signal::SignalCase::kInt32Value) {
    throw std::runtime_error("Signal " + name_ + " is not of int32_t type");
  }
  if (const_) {
    throw std::runtime_error("Signal " + name_ + " is constant and cannot be set");
  }
  signal_->set_int32_value(value);
}

template <>
void SignalHandle::setValue<int64_t>(const int64_t value) {
  if (type_ != proton::Signal::SignalCase::kInt64Value) {
    throw std::runtime_error("Signal " + name_ + " is not of int64_t type");
  }
  if (const_) {
    throw std::runtime_error("Signal " + name_ + " is constant and cannot be set");
  }
  signal_->set_int64_value(value);
}

template <>
void SignalHandle::setValue<uint32_t>(const uint32_t value) {
  if (type_ != proton::Signal::SignalCase::kUint32Value) {
    throw std::runtime_error("Signal " + name_ + " is not of uint32_t type");
  }
  if (const_) {
    throw std::runtime_error("Signal " + name_ + " is constant and cannot be set");
  }
  signal_->set_uint32_value(value);
}

template <>
void SignalHandle::setValue<uint64_t>(const uint64_t value) {
  if (type_ != proton::Signal::SignalCase::kUint64Value) {
    throw std::runtime_error("Signal " + name_ + " is not of uint64_t type");
  }
  if (const_) {
    throw std::runtime_error("Signal " + name_ + " is constant and cannot be set");
  }
  signal_->set_uint64_value(value);
}

template <>
void SignalHandle::setValue<bool>(const bool value) {
  if (type_ != proton::Signal::SignalCase::kBoolValue) {
    throw std::runtime_error("Signal " + name_ + " is not of bool type");
  }
  if (const_) {
    throw std::runtime_error("Signal " + name_ + " is constant and cannot be set");
  }
  signal_->set_bool_value(value);
}

template <>
void SignalHandle::setValue<std::string>(const std::string value) {
  if (type_ != proton::Signal::SignalCase::kStringValue) {
    throw std::runtime_error("Signal " + name_ + " is not of std::string type");
  }
  if (const_) {
    throw std::runtime_error("Signal " + name_ + " is constant and cannot be set");
  }
  signal_->set_string_value(value);
}

template <>
void SignalHandle::setValue<proton::bytes>(const proton::bytes value) {
  if (type_ != proton::Signal::SignalCase::kBytesValue) {
    throw std::runtime_error("Signal " + name_ + " is not of proton::bytes type");
  }
  if (const_) {
    throw std::runtime_error("Signal " + name_ + " is constant and cannot be set");
  }
  signal_->mutable_bytes_value()->assign(value.begin(), value.end());
}
