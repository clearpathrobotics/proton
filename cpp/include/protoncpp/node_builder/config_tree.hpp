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

#ifndef PROTON_NODE_BUILDER_CONFIG_TREE_HPP
#define PROTON_NODE_BUILDER_CONFIG_TREE_HPP

#include "proton/proton_config.h"

#if PROTON_NODE_BUILDER

#include <cstdint>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#if PROTON_NODE_BUILDER_YAML_PARSER
#include <yaml-cpp/yaml.h>
#endif  // PROTON_NODE_BUILDER_YAML_PARSER

#if PROTON_NODE_BUILDER_JSON_PARSER
#include <nlohmann/json.hpp>
#endif  // PROTON_NODE_BUILDER_JSON_PARSER

namespace proton::node_builder
{

class NodeBuilderException;

// Forward declaration for recursive variant
struct ConfigValue;

/**
 * Sequence of config values (array/list)
 */
using ConfigSequence = std::vector<ConfigValue>;

/**
 * Map of string keys to config values (object/dict)
 */
using ConfigMap = std::map<std::string, ConfigValue, std::less<>>;

/**
 * @brief Variant holding all possible config value types
 * std::monostate represents null/undefined
 */
struct ConfigValue
{
  std::variant<
    std::monostate,  // null/undefined
    bool, int64_t, uint64_t, double, std::string, ConfigSequence, ConfigMap>
    data;

  ConfigValue() : data(std::monostate{}) {}
  ConfigValue(std::monostate) : data(std::monostate{}) {}
  ConfigValue(bool v) : data(v) {}
  ConfigValue(int32_t v) : data(static_cast<int64_t>(v)) {}
  ConfigValue(int64_t v) : data(v) {}
  ConfigValue(uint32_t v) : data(static_cast<uint64_t>(v)) {}
  ConfigValue(uint64_t v) : data(v) {}
  ConfigValue(double v) : data(v) {}
  ConfigValue(float v) : data(static_cast<double>(v)) {}
  ConfigValue(const char * v) : data(std::string(v)) {}
  ConfigValue(std::string v) : data(std::move(v)) {}
  ConfigValue(ConfigSequence v) : data(std::move(v)) {}
  ConfigValue(ConfigMap v) : data(std::move(v)) {}
};

/**
 * @class ConfigNode Lightweight wrapper around ConfigValue providing convenient access methods
 */
class ConfigNode
{
public:
  ConfigNode() : value_(nullptr) {}
  explicit ConfigNode(const ConfigValue * value) : value_(value) {}
  explicit ConfigNode(const ConfigValue & value) : value_(&value) {}

  /**
   * @brief Check if the node holds a valid value (not null/undefined)
   */
  bool is_defined() const
  {
    return value_ != nullptr && !std::holds_alternative<std::monostate>(value_->data);
  }

  /**
   * @brief Check if the node is explicitly null or undefined
   */
  bool is_null() const
  {
    return value_ == nullptr || std::holds_alternative<std::monostate>(value_->data);
  }

  /**
   * @brief Check if the node holds a sequence (array)
   */
  bool is_sequence() const
  {
    return value_ != nullptr && std::holds_alternative<ConfigSequence>(value_->data);
  }

  /**
   * @brief Check if the node holds a map (object)
   */
  bool is_map() const
  {
    return value_ != nullptr && std::holds_alternative<ConfigMap>(value_->data);
  }

  /**
   * @brief Check if the node holds a scalar (non-container) value
   */
  bool is_scalar() const
  {
    if (value_ == nullptr)
    {
      return false;
    }
    return !std::holds_alternative<std::monostate>(value_->data) &&
           !std::holds_alternative<ConfigSequence>(value_->data) &&
           !std::holds_alternative<ConfigMap>(value_->data);
  }

  /**
   * @brief Access a child node by key (for maps)
   */
  ConfigNode operator[](std::string_view key) const
  {
    if (!is_map())
    {
      return ConfigNode();
    }
    const auto & map = std::get<ConfigMap>(value_->data);
    auto it = map.find(key);
    if (it == map.end())
    {
      return ConfigNode();
    }
    return ConfigNode(&it->second);
  }

  /**
   * @brief Access a child node by index (for sequences)
   */
  ConfigNode operator[](size_t idx) const
  {
    if (!is_sequence())
    {
      return ConfigNode();
    }
    const auto & sequence = std::get<ConfigSequence>(value_->data);
    return ConfigNode(sequence[idx]);
  }

  /**
   * @brief Get the size of a sequence
   */
  size_t size() const
  {
    if (!is_sequence())
    {
      return 0;
    }
    return std::get<ConfigSequence>(value_->data).size();
  }

  /**
   * @brief Convert to string, throws if not convertible
   */
  std::string as_string() const;

  /**
   * @brief Convert to uint32_t, throws if not convertible
   */
  uint32_t as_uint32() const;

  /**
   * @brief Convert to int64_t, throws if not convertible
   */
  int64_t as_int64() const;

  /**
   * @brief Convert to uint64_t, throws if not convertible
   */
  uint64_t as_uint64() const;

  /**
   * @brief Convert to double, throws if not convertible
   */
  double as_double() const;

  /**
   * @brief Convert to bool, throws if not convertible
   */
  bool as_bool() const;

  /**
   * @brief Get the underlying ConfigValue (for storing default values)
   */
  const ConfigValue & value() const
  {
    static const ConfigValue null_value;
    return value_ ? *value_ : null_value;
  }

  /**
   * @brief Iterator support for sequences
   */
  class Iterator
  {
  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = ConfigNode;
    using difference_type = std::ptrdiff_t;
    using pointer = const ConfigNode *;
    using reference = ConfigNode;

    Iterator() : seq_(nullptr), index_(0) {}
    Iterator(const ConfigSequence * seq, size_t index) : seq_(seq), index_(index) {}

    ConfigNode operator*() const { return ConfigNode(&(*seq_)[index_]); }

    Iterator & operator++()
    {
      ++index_;
      return *this;
    }

    bool operator==(const Iterator & other) const
    {
      return seq_ == other.seq_ && index_ == other.index_;
    }

    bool operator!=(const Iterator & other) const { return !(*this == other); }

  private:
    const ConfigSequence * seq_;
    size_t index_;
  };

  Iterator begin() const
  {
    if (!is_sequence())
    {
      return Iterator();
    }
    return Iterator(&std::get<ConfigSequence>(value_->data), 0);
  }

  Iterator end() const
  {
    if (!is_sequence())
    {
      return Iterator();
    }
    const auto & seq = std::get<ConfigSequence>(value_->data);
    return Iterator(&seq, seq.size());
  }

  /**
   * @brief Boolean conversion for if (node) checks
   */
  explicit operator bool() const { return is_defined(); }

private:
  const ConfigValue * value_;
};

/**
 * @class ConfigTree Container for parsed configuration tree with factory methods for different formats
 */
class ConfigTree
{
public:
  ConfigTree() = default;
  explicit ConfigTree(ConfigValue root) : root_(std::move(root)) {}

  /**
   * @brief Get the root node
   */
  ConfigNode root() const { return ConfigNode(&root_); }

  /**
   * @brief Access top-level key
   */
  ConfigNode operator[](std::string_view key) const { return root()[key]; }

  /**
   * @brief Access index
   */
  ConfigNode operator[](size_t idx) const { return root()[idx]; }

#if PROTON_NODE_BUILDER_YAML_PARSER

  /**
   * @brief Factory: Load from YAML file
   */
  static ConfigTree from_yaml_file(const std::string & path);

  /**
   * @brief Factory: Parse from YAML string
   */
  static ConfigTree from_yaml_string(std::string_view yaml);

  /**
   * @brief Factory: Convert from YAML::Node
   */
  static ConfigTree from_yaml_node(const YAML::Node & yaml);

#endif  // PROTON_NODE_BUILDER_YAML_PARSER

#if PROTON_NODE_BUILDER_JSON_PARSER

  /**
   * @brief Factory: Load from JSON file
   */
  static ConfigTree from_json_file(const std::string & path);

  /**
   * @brief Factory: Parse from JSON string
   */
  static ConfigTree from_json_string(std::string_view json);

  /**
   * @brief Factory: Convert from nlohmann::json
   */
  static ConfigTree from_json_value(const nlohmann::json & json);

#endif  // PROTON_NODE_BUILDER_JSON_PARSER

private:
  ConfigValue root_;
};

}  // namespace proton::node_builder

#endif  // PROTON_NODE_BUILDER
#endif  // PROTON_NODE_BUILDER_CONFIG_TREE_HPP
