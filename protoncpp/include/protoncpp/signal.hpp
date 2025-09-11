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

#ifndef INC_PROTONCPP_SIGNAL_HPP_
#define INC_PROTONCPP_SIGNAL_HPP_

#include "protoncpp/signal.pb.h"
#include "protoncpp/config.hpp"

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
  SignalHandle(SignalConfig config, std::shared_ptr<Signal> ptr);

  // Value getter and setter
  template <typename T> const T getValue() const;
  template <typename T> void setValue(const T value);
  template <typename T> void setValue(uint16_t index, const T value);

  std::string getName() { return name_; }
  std::string getBundleName() { return bundle_name_; }
  Signal::SignalCase getType() { return type_; }
  uint32_t getLength() const { return length_; }
  uint32_t getCapacity() const { return capacity_; }
  std::shared_ptr<Signal> getSignalPtr() { return signal_; }

private:
  std::string name_;
  std::string bundle_name_;
  Signal::SignalCase type_;
  uint32_t length_;
  uint32_t capacity_;
  std::shared_ptr<Signal> signal_;
};


} // namespace proton

#endif // INC_PROTONCPP_BUNDLE_HPP_
