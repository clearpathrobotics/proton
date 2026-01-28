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
