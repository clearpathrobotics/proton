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

#ifndef INC_PROTONCPP_SIGNAL_HPP_
#define INC_PROTONCPP_SIGNAL_HPP_

#include "protoncpp/config.hpp"
#include "protoncpp/signal.pb.h"

#include <memory>

namespace proton
{

using bytes = std::vector<uint8_t>;

namespace default_values
{
inline static const double DOUBLE = 0.0;
inline static const float FLOAT = 0.0f;
inline static const int32_t INT32 = 0;
inline static const int64_t INT64 = 0;
inline static const uint32_t UINT32 = 0;
inline static const uint64_t UINT64 = 0;
inline static const bool BOOL = false;
inline static const std::string STRING = "";
inline static const proton::bytes BYTES = {};
}  // namespace default_values

class SignalHandle
{
public:
  SignalHandle(SignalConfig config, const std::string & bundle_name, Signal * signal);

  ~SignalHandle();

  // Value getter and setter
  template <typename T>
  const T getValue() const;
  template <typename T>
  void setValue(const T value);
  template <typename T>
  void setValue(uint16_t index, const T value);
  template <typename T>
  void setValue(uint16_t index, uint16_t subindex, const T value);

  std::string & getName() const { return name_; }
  std::string & getBundleName() const { return bundle_name_; }
  Signal::SignalCase getType() { return type_; }
  uint32_t getCapacity() const { return capacity_; }
  Signal * getSignalPtr() { return signal_; }

private:
  std::string name_;
  std::string bundle_name_;
  Signal::SignalCase type_;
  uint32_t capacity_;
  Signal * signal_;
  bool has_default_value_;
};

}  // namespace proton

#endif  // INC_PROTONCPP_SIGNAL_HPP_
