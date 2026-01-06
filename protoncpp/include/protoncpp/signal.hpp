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
using list_bytes = std::vector<bytes>;

namespace default_values {
  inline static const double DOUBLE = 0.0;
  inline static const float FLOAT = 0.0f;
  inline static const int32_t INT32 = 0;
  inline static const int64_t INT64 = 0;
  inline static const uint32_t UINT32 = 0;
  inline static const uint64_t UINT64 = 0;
  inline static const bool BOOL = false;
  inline static const std::string STRING = "";
  inline static const proton::bytes BYTES = {};
  inline static const proton::list_double LIST_DOUBLE = {};
  inline static const proton::list_float LIST_FLOAT = {};
  inline static const proton::list_int32 LIST_INT32 = {};
  inline static const proton::list_int64 LIST_INT64 = {};
  inline static const proton::list_uint32 LIST_UINT32 = {};
  inline static const proton::list_uint64 LIST_UINT64 = {};
  inline static const proton::list_bool LIST_BOOL = {};
  inline static const proton::list_string LIST_STRING = {};
  inline static const proton::list_bytes LIST_BYTES = {};
}

class SignalHandle {

public:
  SignalHandle(SignalConfig config, std::string bundle_name, Signal* signal);

  ~SignalHandle();

  // Value getter and setter
  template <typename T> const T getValue() const;
  template <typename T> void setValue(const T value);
  template <typename T> void setValue(uint16_t index, const T value);
  template <typename T> void setValue(uint16_t index, uint16_t subindex, const T value);

  std::string getName() { return name_; }
  std::string getBundleName() { return bundle_name_; }
  Signal::SignalCase getType() { return type_; }
  uint32_t getLength() const { return length_; }
  uint32_t getCapacity() const { return capacity_; }
  Signal* getSignalPtr() { return signal_; }

private:
  std::string name_;
  std::string bundle_name_;
  Signal::SignalCase type_;
  uint32_t length_;
  uint32_t capacity_;
  Signal* signal_;
  bool const_;
};


} // namespace proton

#endif // INC_PROTONCPP_BUNDLE_HPP_
