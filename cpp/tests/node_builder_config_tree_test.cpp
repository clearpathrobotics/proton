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
 * @author Tom Wallis (thomas.wallis@rockwellautomation.com)
 */

#include <gtest/gtest.h>
#include <cstring>
#include <functional>
#include <sstream>
#include <string>

#include "protoncpp/node_builder/config_tree.hpp"

#include <yaml-cpp/yaml.h>

using namespace proton::node_builder;

// ============================================================================
// ConfigValue Tests
// ============================================================================

TEST(ConfigValueTest, DefaultConstructorCreatesNull)
{
  ConfigValue val;
  EXPECT_TRUE(std::holds_alternative<std::monostate>(val.data));
}

TEST(ConfigValueTest, BoolConstruction)
{
  ConfigValue val_true(true);
  ConfigValue val_false(false);
  EXPECT_TRUE(std::get<bool>(val_true.data));
  EXPECT_FALSE(std::get<bool>(val_false.data));
}

TEST(ConfigValueTest, IntegerConstruction)
{
  ConfigValue val_i32(int32_t{-42});
  ConfigValue val_i64(int64_t{-9223372036854775807LL});
  ConfigValue val_u32(uint32_t{42});
  ConfigValue val_u64(uint64_t{18446744073709551615ULL});

  EXPECT_EQ(std::get<int64_t>(val_i32.data), -42);
  EXPECT_EQ(std::get<int64_t>(val_i64.data), -9223372036854775807LL);
  EXPECT_EQ(std::get<uint64_t>(val_u32.data), 42);
  EXPECT_EQ(std::get<uint64_t>(val_u64.data), 18446744073709551615ULL);
}

TEST(ConfigValueTest, DoubleConstruction)
{
  ConfigValue val_d(3.14159);
  ConfigValue val_f(float{2.5f});
  EXPECT_DOUBLE_EQ(std::get<double>(val_d.data), 3.14159);
  EXPECT_DOUBLE_EQ(std::get<double>(val_f.data), 2.5);
}

TEST(ConfigValueTest, StringConstruction)
{
  ConfigValue val_cstr("hello");
  ConfigValue val_str(std::string("world"));
  EXPECT_EQ(std::get<std::string>(val_cstr.data), "hello");
  EXPECT_EQ(std::get<std::string>(val_str.data), "world");
}

TEST(ConfigValueTest, SequenceConstruction)
{
  ConfigSequence seq = {ConfigValue(1), ConfigValue(2), ConfigValue(3)};
  ConfigValue val(seq);
  EXPECT_EQ(std::get<ConfigSequence>(val.data).size(), 3);
}

TEST(ConfigValueTest, MapConstruction)
{
  ConfigMap map;
  map["key1"] = ConfigValue("value1");
  map["key2"] = ConfigValue(42);
  ConfigValue val(map);
  EXPECT_EQ(std::get<ConfigMap>(val.data).size(), 2);
}

// ============================================================================
// ConfigNode Type Check Tests
// ============================================================================

TEST(ConfigNodeTest, DefaultConstructorIsNull)
{
  ConfigNode node;
  EXPECT_TRUE(node.is_null());
  EXPECT_FALSE(node.is_defined());
}

TEST(ConfigNodeTest, NullValueIsNull)
{
  ConfigValue val;
  ConfigNode node(val);
  EXPECT_TRUE(node.is_null());
  EXPECT_FALSE(node.is_defined());
}

TEST(ConfigNodeTest, BoolValueIsDefined)
{
  ConfigValue val(true);
  ConfigNode node(val);
  EXPECT_TRUE(node.is_defined());
  EXPECT_FALSE(node.is_null());
  EXPECT_TRUE(node.is_scalar());
  EXPECT_FALSE(node.is_sequence());
  EXPECT_FALSE(node.is_map());
}

TEST(ConfigNodeTest, IntValueIsDefined)
{
  ConfigValue val(42);
  ConfigNode node(val);
  EXPECT_TRUE(node.is_defined());
  EXPECT_TRUE(node.is_scalar());
}

TEST(ConfigNodeTest, SequenceValueIsSequence)
{
  ConfigSequence seq = {ConfigValue(1), ConfigValue(2)};
  ConfigValue val(seq);
  ConfigNode node(val);
  EXPECT_TRUE(node.is_defined());
  EXPECT_TRUE(node.is_sequence());
  EXPECT_FALSE(node.is_scalar());
  EXPECT_FALSE(node.is_map());
}

TEST(ConfigNodeTest, MapValueIsMap)
{
  ConfigMap map;
  map["key"] = ConfigValue("value");
  ConfigValue val(map);
  ConfigNode node(val);
  EXPECT_TRUE(node.is_defined());
  EXPECT_TRUE(node.is_map());
  EXPECT_FALSE(node.is_scalar());
  EXPECT_FALSE(node.is_sequence());
}

// ============================================================================
// ConfigNode Access Tests
// ============================================================================

TEST(ConfigNodeTest, MapAccessByKey)
{
  ConfigMap map;
  map["name"] = ConfigValue("test");
  map["value"] = ConfigValue(123);
  ConfigValue val(map);
  ConfigNode node(val);

  EXPECT_EQ(node["name"].as_string(), "test");
  EXPECT_EQ(node["value"].as_uint32(), 123);
  EXPECT_TRUE(node["nonexistent"].is_null());
}

TEST(ConfigNodeTest, SequenceSize)
{
  ConfigSequence seq = {ConfigValue(1), ConfigValue(2), ConfigValue(3)};
  ConfigValue val(seq);
  ConfigNode node(val);
  EXPECT_EQ(node.size(), 3);
}

TEST(ConfigNodeTest, NonSequenceSizeIsZero)
{
  ConfigValue val(42);
  ConfigNode node(val);
  EXPECT_EQ(node.size(), 0);
}

TEST(ConfigNodeTest, MapAccessOnNonMapReturnsNull)
{
  ConfigValue val(42);
  ConfigNode node(val);
  EXPECT_TRUE(node["key"].is_null());
}

// ============================================================================
// ConfigNode Conversion Tests
// ============================================================================

TEST(ConfigNodeTest, AsStringFromString)
{
  ConfigValue val("hello");
  ConfigNode node(val);
  EXPECT_EQ(node.as_string(), "hello");
}

TEST(ConfigNodeTest, AsStringFromInt)
{
  ConfigValue val(int64_t{42});
  ConfigNode node(val);
  EXPECT_EQ(node.as_string(), "42");
}

TEST(ConfigNodeTest, AsStringFromUint)
{
  ConfigValue val(uint64_t{42});
  ConfigNode node(val);
  EXPECT_EQ(node.as_string(), "42");
}

TEST(ConfigNodeTest, AsStringFromBool)
{
  ConfigValue val_true(true);
  ConfigValue val_false(false);
  ConfigNode node_true(val_true);
  ConfigNode node_false(val_false);
  EXPECT_EQ(node_true.as_string(), "true");
  EXPECT_EQ(node_false.as_string(), "false");
}

TEST(ConfigNodeTest, AsStringFromNullThrows)
{
  ConfigNode node;
  EXPECT_THROW(node.as_string(), std::runtime_error);
}

TEST(ConfigNodeTest, AsStringFromMapThrows)
{
  ConfigMap map;
  ConfigValue val(map);
  ConfigNode node(val);
  EXPECT_THROW(node.as_string(), std::runtime_error);
}

TEST(ConfigNodeTest, AsUint32FromUint)
{
  ConfigValue val(uint64_t{12345});
  ConfigNode node(val);
  EXPECT_EQ(node.as_uint32(), 12345);
}

TEST(ConfigNodeTest, AsUint32FromInt)
{
  ConfigValue val(int64_t{42});
  ConfigNode node(val);
  EXPECT_EQ(node.as_uint32(), 42);
}

TEST(ConfigNodeTest, AsUint32FromDouble)
{
  ConfigValue val(42.9);
  ConfigNode node(val);
  EXPECT_EQ(node.as_uint32(), 42);
}

TEST(ConfigNodeTest, AsUint32FromNullThrows)
{
  ConfigNode node;
  EXPECT_THROW(node.as_uint32(), std::runtime_error);
}

TEST(ConfigNodeTest, AsInt64FromInt)
{
  ConfigValue val(int64_t{-123456});
  ConfigNode node(val);
  EXPECT_EQ(node.as_int64(), -123456);
}

TEST(ConfigNodeTest, AsInt64FromUint)
{
  ConfigValue val(uint64_t{999});
  ConfigNode node(val);
  EXPECT_EQ(node.as_int64(), 999);
}

TEST(ConfigNodeTest, AsUint64FromUint)
{
  ConfigValue val(uint64_t{0xDEADBEEF});
  ConfigNode node(val);
  EXPECT_EQ(node.as_uint64(), 0xDEADBEEF);
}

TEST(ConfigNodeTest, AsDoubleFromDouble)
{
  ConfigValue val(3.14159);
  ConfigNode node(val);
  EXPECT_DOUBLE_EQ(node.as_double(), 3.14159);
}

TEST(ConfigNodeTest, AsDoubleFromInt)
{
  ConfigValue val(int64_t{42});
  ConfigNode node(val);
  EXPECT_DOUBLE_EQ(node.as_double(), 42.0);
}

TEST(ConfigNodeTest, AsBoolFromBool)
{
  ConfigValue val_true(true);
  ConfigValue val_false(false);
  ConfigNode node_true(val_true);
  ConfigNode node_false(val_false);
  EXPECT_TRUE(node_true.as_bool());
  EXPECT_FALSE(node_false.as_bool());
}

TEST(ConfigNodeTest, AsBoolFromNullThrows)
{
  ConfigNode node;
  EXPECT_THROW(node.as_bool(), std::runtime_error);
}

TEST(ConfigNodeTest, AsBoolFromIntThrows)
{
  ConfigValue val(42);
  ConfigNode node(val);
  EXPECT_THROW(node.as_bool(), std::runtime_error);
}

// ============================================================================
// ConfigNode Iterator Tests
// ============================================================================

TEST(ConfigNodeTest, IterateSequence)
{
  ConfigSequence seq = {ConfigValue(10), ConfigValue(20), ConfigValue(30)};
  ConfigValue val(seq);
  ConfigNode node(val);

  std::vector<int64_t> values;
  for (const auto & item : node)
  {
    values.push_back(item.as_int64());
  }

  EXPECT_EQ(values.size(), 3);
  EXPECT_EQ(values[0], 10);
  EXPECT_EQ(values[1], 20);
  EXPECT_EQ(values[2], 30);
}

TEST(ConfigNodeTest, IterateNonSequenceIsEmpty)
{
  ConfigValue val(42);
  ConfigNode node(val);

  int count = 0;
  for (const auto & item : node)
  {
    (void)item;
    ++count;
  }
  EXPECT_EQ(count, 0);
}

// ============================================================================
// ConfigNode Boolean Operator Tests
// ============================================================================

TEST(ConfigNodeTest, BoolOperatorTrueWhenDefined)
{
  ConfigValue val(42);
  ConfigNode node(val);
  EXPECT_TRUE(static_cast<bool>(node));
  if (node)
  {
    SUCCEED();
  }
  else
  {
    FAIL() << "Expected node to be truthy";
  }
}

TEST(ConfigNodeTest, BoolOperatorFalseWhenNull)
{
  ConfigNode node;
  EXPECT_FALSE(static_cast<bool>(node));
}

// ============================================================================
// ConfigTree YAML Parsing Tests
// ============================================================================

TEST(ConfigTreeTest, ParseYamlString)
{
  const char * yaml = R"(
name: test
value: 42
)";
  auto tree = ConfigTree::from_yaml_string(yaml);
  EXPECT_EQ(tree["name"].as_string(), "test");
  EXPECT_EQ(tree["value"].as_uint32(), 42);
}

TEST(ConfigTreeTest, ParseYamlBooleans)
{
  const char * yaml = R"(
bool_true: true
bool_True: True
bool_TRUE: TRUE
bool_false: false
bool_False: False
bool_FALSE: FALSE
)";
  auto tree = ConfigTree::from_yaml_string(yaml);
  EXPECT_TRUE(tree["bool_true"].as_bool());
  EXPECT_TRUE(tree["bool_True"].as_bool());
  EXPECT_TRUE(tree["bool_TRUE"].as_bool());
  EXPECT_FALSE(tree["bool_false"].as_bool());
  EXPECT_FALSE(tree["bool_False"].as_bool());
  EXPECT_FALSE(tree["bool_FALSE"].as_bool());
}

TEST(ConfigTreeTest, ParseYamlIntegers)
{
  const char * yaml = R"(
positive: 12345
negative: -67890
zero: 0
)";
  auto tree = ConfigTree::from_yaml_string(yaml);
  EXPECT_EQ(tree["positive"].as_int64(), 12345);
  EXPECT_EQ(tree["negative"].as_int64(), -67890);
  EXPECT_EQ(tree["zero"].as_int64(), 0);
}

TEST(ConfigTreeTest, ParseYamlHexIntegers)
{
  const char * yaml = R"(
hex_lower: 0x1000
hex_upper: 0X2000
hex_mixed: 0xDEADBEEF
)";
  auto tree = ConfigTree::from_yaml_string(yaml);
  EXPECT_EQ(tree["hex_lower"].as_uint64(), 0x1000);
  EXPECT_EQ(tree["hex_upper"].as_uint64(), 0x2000);
  EXPECT_EQ(tree["hex_mixed"].as_uint64(), 0xDEADBEEF);
}

TEST(ConfigTreeTest, ParseYamlDoubles)
{
  const char * yaml = R"(
pi: 3.14159
negative: -2.5
scientific: 1.0e10
)";
  auto tree = ConfigTree::from_yaml_string(yaml);
  EXPECT_DOUBLE_EQ(tree["pi"].as_double(), 3.14159);
  EXPECT_DOUBLE_EQ(tree["negative"].as_double(), -2.5);
}

TEST(ConfigTreeTest, ParseYamlStrings)
{
  const char * yaml = R"(
simple: hello
quoted: "world"
with_spaces: "hello world"
)";
  auto tree = ConfigTree::from_yaml_string(yaml);
  EXPECT_EQ(tree["simple"].as_string(), "hello");
  EXPECT_EQ(tree["quoted"].as_string(), "world");
  EXPECT_EQ(tree["with_spaces"].as_string(), "hello world");
}

TEST(ConfigTreeTest, ParseYamlSequence)
{
  const char * yaml = R"(
items:
  - 1
  - 2
  - 3
)";
  auto tree = ConfigTree::from_yaml_string(yaml);
  auto items = tree["items"];
  EXPECT_TRUE(items.is_sequence());
  EXPECT_EQ(items.size(), 3);

  std::vector<int64_t> values;
  for (const auto & item : items)
  {
    values.push_back(item.as_int64());
  }
  EXPECT_EQ(values, (std::vector<int64_t>{1, 2, 3}));
}

TEST(ConfigTreeTest, ParseYamlNestedMap)
{
  const char * yaml = R"(
outer:
  inner:
    value: 42
)";
  auto tree = ConfigTree::from_yaml_string(yaml);
  EXPECT_EQ(tree["outer"]["inner"]["value"].as_uint32(), 42);
}

TEST(ConfigTreeTest, ParseYamlNull)
{
  const char * yaml = R"(
null_value: null
empty_value:
)";
  auto tree = ConfigTree::from_yaml_string(yaml);
  EXPECT_TRUE(tree["null_value"].is_null());
  EXPECT_TRUE(tree["empty_value"].is_null());
  EXPECT_TRUE(tree["nonexistent"].is_null());
}

TEST(ConfigTreeTest, ParseYamlMixedSequence)
{
  const char * yaml = R"(
mixed:
  - string_value
  - 42
  - 3.14
  - true
)";
  auto tree = ConfigTree::from_yaml_string(yaml);
  auto mixed = tree["mixed"];
  EXPECT_TRUE(mixed.is_sequence());
  EXPECT_EQ(mixed.size(), 4);
}

TEST(ConfigTreeTest, RootAccess)
{
  const char * yaml = R"(
key: value
)";
  auto tree = ConfigTree::from_yaml_string(yaml);
  auto root = tree.root();
  EXPECT_TRUE(root.is_map());
  EXPECT_EQ(root["key"].as_string(), "value");
}

TEST(ConfigTreeTest, EmptyYaml)
{
  auto tree = ConfigTree::from_yaml_string("");
  EXPECT_TRUE(tree.root().is_null());
}

int main(int argc, char ** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
