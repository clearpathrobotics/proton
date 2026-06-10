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

#include "proton/proton_config.h"

#if PROTON_NODE_BUILDER

#include "protoncpp/node_builder/config_tree.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>

#include <yaml-cpp/yaml.h>

#if PROTON_NODE_BUILDER_JSON_PARSER

#include <nlohmann/json.hpp>

#endif  // PROTON_NODE_BUILDER_JSON_PARSER

namespace proton::node_builder
{

// Helper to convert YAML::Node to ConfigValue recursively
static ConfigValue yaml_to_config_value(const YAML::Node & node)
{
  if (!node.IsDefined() || node.IsNull())
  {
    return ConfigValue(std::monostate{});
  }

  if (node.IsScalar())
  {
    const std::string & scalar = node.Scalar();

    // Try to parse as bool
    if (scalar == "true" || scalar == "True" || scalar == "TRUE")
    {
      return ConfigValue(true);
    }
    if (scalar == "false" || scalar == "False" || scalar == "FALSE")
    {
      return ConfigValue(false);
    }

    // Try to parse as integer (handle hex with 0x prefix)
    try
    {
      size_t pos = 0;
      if (scalar.size() > 2 && scalar[0] == '0' && (scalar[1] == 'x' || scalar[1] == 'X'))
      {
        // Hex number
        uint64_t val = std::stoull(scalar, &pos, 16);
        if (pos == scalar.size())
        {
          return ConfigValue(val);
        }
      }
      else if (!scalar.empty() && (std::isdigit(scalar[0]) || scalar[0] == '-'))
      {
        // Check for decimal point
        if (scalar.find('.') != std::string::npos)
        {
          double val = std::stod(scalar, &pos);
          if (pos == scalar.size())
          {
            return ConfigValue(val);
          }
        }
        else
        {
          // Try signed first
          int64_t val = std::stoll(scalar, &pos, 10);
          if (pos == scalar.size())
          {
            return ConfigValue(val);
          }
        }
      }
    }
    catch (...)
    {
      // Fall through to string
    }

    // Default to string
    return ConfigValue(scalar);
  }

  if (node.IsSequence())
  {
    ConfigSequence seq;
    seq.reserve(node.size());
    for (const auto & child : node)
    {
      seq.push_back(yaml_to_config_value(child));
    }
    return ConfigValue(std::move(seq));
  }

  if (node.IsMap())
  {
    ConfigMap map;
    for (const auto & pair : node)
    {
      std::string key = pair.first.as<std::string>();
      map.emplace(std::move(key), yaml_to_config_value(pair.second));
    }
    return ConfigValue(std::move(map));
  }

  return ConfigValue(std::monostate{});
}

#if PROTON_NODE_BUILDER_JSON_PARSER

static ConfigValue json_to_config_value(const nlohmann::json & json)
{
  if (json.is_null())
  {
    return ConfigValue(std::monostate{});
  }

  if (json.is_primitive())
  {
    if (json.is_boolean())
    {
      return ConfigValue(json.get<bool>());
    }

    if (json.is_number_integer())
    {
      if (json.is_number_unsigned())
      {
        return ConfigValue(json.get<uint64_t>());
      }
      else
      {
        return ConfigValue(json.get<int64_t>());
      }
    }

    if (json.is_number_float())
    {
      return ConfigValue(json.get<double>());
    }

    if (json.is_string())
    {
      return ConfigValue(json.get<std::string>());
    }
  }

  if (json.is_array())
  {
    ConfigSequence seq;
    seq.reserve(json.size());
    for (const auto & child : json)
    {
      seq.push_back(json_to_config_value(child));
    }
    return ConfigValue(std::move(seq));
  }

  if (json.is_object())
  {
    ConfigMap map;
    for (const auto & [key, val] : json.items())
    {
      map.emplace(std::move(key), json_to_config_value(val));
    }
    return ConfigValue(std::move(map));
  }

  return ConfigValue(std::monostate{});
}

#endif  // PROTON_NODE_BUILDER_JSON_PARSER

// ConfigNode conversion methods

std::string ConfigNode::as_string() const
{
  if (!value_)
  {
    throw std::runtime_error("Cannot convert null node to string");
  }

  if (auto * s = std::get_if<std::string>(&value_->data))
  {
    return *s;
  }

  // Allow conversion from numeric types
  if (auto * i = std::get_if<int64_t>(&value_->data))
  {
    return std::to_string(*i);
  }
  if (auto * u = std::get_if<uint64_t>(&value_->data))
  {
    return std::to_string(*u);
  }
  if (auto * d = std::get_if<double>(&value_->data))
  {
    return std::to_string(*d);
  }
  if (auto * b = std::get_if<bool>(&value_->data))
  {
    return *b ? "true" : "false";
  }

  throw std::runtime_error("Cannot convert node to string");
}

uint32_t ConfigNode::as_uint32() const
{
  if (!value_)
  {
    throw std::runtime_error("Cannot convert null node to uint32");
  }

  if (auto * u = std::get_if<uint64_t>(&value_->data))
  {
    return static_cast<uint32_t>(*u);
  }
  if (auto * i = std::get_if<int64_t>(&value_->data))
  {
    return static_cast<uint32_t>(*i);
  }
  if (auto * d = std::get_if<double>(&value_->data))
  {
    return static_cast<uint32_t>(*d);
  }

  throw std::runtime_error("Cannot convert node to uint32");
}

int64_t ConfigNode::as_int64() const
{
  if (!value_)
  {
    throw std::runtime_error("Cannot convert null node to int64");
  }

  if (auto * i = std::get_if<int64_t>(&value_->data))
  {
    return *i;
  }
  if (auto * u = std::get_if<uint64_t>(&value_->data))
  {
    return static_cast<int64_t>(*u);
  }
  if (auto * d = std::get_if<double>(&value_->data))
  {
    return static_cast<int64_t>(*d);
  }

  throw std::runtime_error("Cannot convert node to int64");
}

uint64_t ConfigNode::as_uint64() const
{
  if (!value_)
  {
    throw std::runtime_error("Cannot convert null node to uint64");
  }

  if (auto * u = std::get_if<uint64_t>(&value_->data))
  {
    return *u;
  }
  if (auto * i = std::get_if<int64_t>(&value_->data))
  {
    return static_cast<uint64_t>(*i);
  }
  if (auto * d = std::get_if<double>(&value_->data))
  {
    return static_cast<uint64_t>(*d);
  }

  throw std::runtime_error("Cannot convert node to uint64");
}

double ConfigNode::as_double() const
{
  if (!value_)
  {
    throw std::runtime_error("Cannot convert null node to double");
  }

  if (auto * d = std::get_if<double>(&value_->data))
  {
    return *d;
  }
  if (auto * i = std::get_if<int64_t>(&value_->data))
  {
    return static_cast<double>(*i);
  }
  if (auto * u = std::get_if<uint64_t>(&value_->data))
  {
    return static_cast<double>(*u);
  }

  throw std::runtime_error("Cannot convert node to double");
}

bool ConfigNode::as_bool() const
{
  if (!value_)
  {
    throw std::runtime_error("Cannot convert null node to bool");
  }

  if (auto * b = std::get_if<bool>(&value_->data))
  {
    return *b;
  }

  throw std::runtime_error("Cannot convert node to bool");
}

// ConfigTree factory methods

ConfigTree ConfigTree::from_yaml_file(const std::string & path)
{
  YAML::Node yaml_node = YAML::LoadFile(path);
  return ConfigTree(yaml_to_config_value(yaml_node));
}

ConfigTree ConfigTree::from_yaml_string(std::string_view yaml)
{
  YAML::Node yaml_node = YAML::Load(std::string(yaml));
  return ConfigTree(yaml_to_config_value(yaml_node));
}

#if PROTON_NODE_BUILDER_JSON_PARSER

ConfigTree ConfigTree::from_json_file(const std::string & path)
{
  std::ifstream f(path);
  nlohmann::json config = nlohmann::json::parse(f);
  return ConfigTree(json_to_config_value(config));
}

ConfigTree ConfigTree::from_json_string(std::string_view json)
{
  nlohmann::json config = nlohmann::json::parse(json);
  return ConfigTree(json_to_config_value(config));
}

#endif  // PROTON_NODE_BUILDER_JSON_PARSER

}  // namespace proton::node_builder

#endif  // PROTON_NODE_BUILDER
