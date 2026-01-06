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

#include "protoncpp/utils.hpp"

using namespace proton;

std::map<std::string, BundleHandle> bundles;

TEST(BundleTests, Serialize)
{
  auto& handle = bundles.at("value_test");
  auto bundle = handle.getBundlePtr();

  // Serialize bundle to string
  std::string serialized = bundle->SerializeAsString();

  // Create copy of bundle by parsing serialized string
  auto deserialized_bundle = Bundle();
  deserialized_bundle.ParseFromString(serialized);

  EXPECT_TRUE(google::protobuf::util::MessageDifferencer::Equals(*bundle, deserialized_bundle));
}

int main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);
  bundles = getBundles(CONFIG_FILE);
  return RUN_ALL_TESTS();
}
