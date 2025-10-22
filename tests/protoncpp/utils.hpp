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

#ifndef PROTONCPP_TESTS_PROTONCPP_UTILS_HPP
#define PROTONCPP_TESTS_PROTONCPP_UTILS_HPP

#include "protoncpp/proton.hpp"
#include <gtest/gtest.h>
#include <google/protobuf/util/message_differencer.h>

std::map<std::string, proton::BundleHandle> getBundles(std::string config_file);

template<typename T>
void AssertSignalValue(proton::SignalHandle& handle, const T& value)
{
  ASSERT_EQ(handle.getValue<T>(), value);
}

template<>
inline void AssertSignalValue<std::string>(proton::SignalHandle& handle, const std::string& value)
{
  ASSERT_STREQ(handle.getValue<std::string>().c_str(), value.c_str());
}

#endif  // PROTONCPP_TESTS_PROTONCPP_UTILS_HPP