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

#include "utils.hpp"

std::map<std::string, proton::BundleHandle> getBundles(std::string config_file)
{
  std::map<std::string, proton::BundleHandle> bundles;

  for (auto& bundle_config: proton::Config(config_file).getBundles())
  {
    bundles.emplace(bundle_config.name, proton::BundleHandle(bundle_config));
  }

  return bundles;
}

// template<typename T>
// void AssertSignalValue(proton::SignalHandle& handle, const T& value)
// {
//   ASSERT_EQ(handle.getValue<T>(), value);
// }

// template<>
// void AssertSignalValue<std::string>(proton::SignalHandle& handle, const std::string& value)
// {
//   ASSERT_STREQ(handle.getValue<std::string>().c_str(), value.c_str());
// }
