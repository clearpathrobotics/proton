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

#include "protoncpp/node_builder/generator.hpp"

#include <algorithm>
#include <optional>
#include <vector>

namespace proton::node_builder
{

struct ProducerConsumerIds
{
  std::map<uint32_t, std::vector<uint32_t>> producer_ids;
  std::map<uint32_t, std::vector<uint32_t>> consumer_ids;
};

ProducerConsumerIds set_producer_consumer_ids(const Config & config)
{
  ProducerConsumerIds bundle_ids;
  std::map<std::string, uint32_t> node_id_map{};
  for (const auto & [name, node] : config.nodes)
  {
    node_id_map[name] = node.id;
  }

  const auto assign_id_by_name =
    [&node_id_map](const std::vector<std::string> & prod_con_names, std::vector<uint32_t> & id_list)
  {
    for (const auto & name : prod_con_names)
    {
      if (node_id_map.contains(name))
      {
        id_list.push_back(node_id_map[name]);
      }
      else
      {
        throw NodeBuilderException(std::format("Error: {} is not a node name", name));
      }
    }
  };

  for (const auto & bundle : config.bundles)
  {
    assign_id_by_name(bundle.producers, bundle_ids.producer_ids[bundle.id]);
    assign_id_by_name(bundle.consumers, bundle_ids.consumer_ids[bundle.id]);
  }

  return bundle_ids;
}

void validate(const Config & config)
{
  std::vector<uint32_t> signal_ids;
  std::vector<std::string> signal_names;
  for (const auto & signal : config.signals)
  {
    signal_ids.push_back(signal.id);
    signal_names.push_back(signal.name);
  }

  std::vector<uint32_t> bundle_ids;
  std::vector<std::string> bundle_names;
  for (const auto & bundle : config.bundles)
  {
    bundle_ids.push_back(bundle.id);
    bundle_names.push_back(bundle.name);
  }

  std::vector<uint32_t> node_ids;
  std::vector<std::string> node_names;
  for (const auto & [name, node] : config.nodes)
  {
    node_names.push_back(name);
    node_ids.push_back(node.id);
  }

  find_duplicates<uint32_t>(signal_ids, "signal ID");
  find_duplicates<std::string>(signal_names, "signal names");
  find_duplicates<uint32_t>(bundle_ids, "bundle ID");
  find_duplicates<std::string>(bundle_names, "bundle names");
  find_duplicates<uint32_t>(node_ids, "nodes ID");
  find_duplicates<std::string>(node_names, "node names");
}

Config filter_for_target(const Config & config, const std::string & target_name)
{
  validate(config);

  if (!config.nodes.contains(target_name))
  {
    throw NodeBuilderException(std::format("{} is not an available target node", target_name));
  }

  Config filtered_config = Config();

  filtered_config.nodes.insert({target_name, config.nodes.at(target_name)});

  for (const auto & conn : config.connections)
  {
    std::optional<std::string> conn_to_add = std::nullopt;
    if (conn.first.node == target_name)
    {
      conn_to_add = conn.second.node;
    }
    else if (conn.second.node == target_name)
    {
      conn_to_add = conn.first.node;
    }

    if (conn_to_add.has_value())
    {
      filtered_config.connections.push_back(conn);
      try
      {
        if (!filtered_config.nodes.contains(*conn_to_add))
        {
          filtered_config.nodes.insert({*conn_to_add, config.nodes.at(*conn_to_add)});
        }
      }
      catch (const std::exception & e)
      {
        throw NodeBuilderException(std::format("Could not filter connection: {}", e.what()));
      }
    }
  }

  std::unordered_set<uint32_t> filtered_signal_ids;

  for (const auto & bundle : config.bundles)
  {
    bool target_in_prods =
      std::find(bundle.producers.begin(), bundle.producers.end(), target_name) !=
      bundle.producers.end();
    bool target_in_cons =
      std::find(bundle.consumers.begin(), bundle.consumers.end(), target_name) !=
      bundle.consumers.end();
    if (target_in_prods || target_in_cons)
    {
      filtered_config.bundles.push_back(bundle);

      for (const auto & signal_id : bundle.signals)
      {
        filtered_signal_ids.insert(signal_id);
      }
    }
  }

  for (const auto & signal : config.signals)
  {
    if (filtered_signal_ids.contains(signal.id))
    {
      filtered_config.signals.push_back(signal);
    }
  }

  return filtered_config;
}

static proton_transport_type_e string_to_transport(const std::string & t_type)
{
  if (t_type == transport_types::UDP4)
  {
    return TRANSPORT_TYPE_UDP4;
  }
  else if (t_type == transport_types::SERIAL)
  {
    return TRANSPORT_TYPE_SERIAL;
  }
  else
  {
    throw NodeBuilderException(std::format("Transport type is invalid: {}", t_type));
  }
}

// ============================================================================
// GeneratedNode implementation
// ============================================================================

GeneratedNode::GeneratedNode(const Config & config, const std::string & target_name)
{
  generate_endpoints(config, target_name);
  generate_signals(config);
  generate_bundles(config);
  init_registry();
  init_node(config, target_name);
}

void GeneratedNode::generate_endpoints(const Config & config, const std::string & target_name)
{
  for (const auto & [name, node] : config.nodes)
  {
    if (name != target_name)
    {
      for (const auto & [id, endpoint] : node.endpoints)
      {
        proton_endpoint_t ep = {
          .node_id = node.id,
          .endpoint_id = endpoint.id,
          .transport_type = string_to_transport(endpoint.type)};
        node_destination_peers_.push_back(ep);
      }
    }
  }
}

void GeneratedNode::generate_signals(const Config & config)
{
  signal_registry_.reserve(config.signals.size());
  signal_decode_buffer_storage_.reserve(config.signals.size());
  signal_scratch_buffer_.reserve(PROTON_SCRATCH_BUFFER_SIZE);

  for (size_t idx = 0; idx < config.signals.size(); idx++)
  {
    const auto & signal_cfg = config.signals[idx];
    proton_signal_type_e sig_type = string_to_signal_type(signal_cfg.type_string.c_str());
    if (sig_type == PROTON_INVALID_TYPE)
    {
      throw NodeBuilderException(std::format("Signal type is invalid: {}", signal_cfg.type_string));
    }

    uint16_t value_size = get_signal_value_size(sig_type, signal_cfg.capacity);

    // Create signal descriptor
    signal_desc_t sig_desc = {
      .id = signal_cfg.id,
      .type = sig_type,
      .value_size = value_size,
      .signal = proton_Signal_init_zero,
      .signal_decode_buffer = nullptr,
    };

    sig_desc.signal.which_signal = proton_get_tag_from_type(sig_type);

    // Set default value if provided
    if (signal_cfg.has_default_value)
    {
      ConfigNode value_node(signal_cfg.value);
      switch (sig_type)
      {
        case PROTON_DOUBLE:
          sig_desc.signal.signal.double_value = value_node.as_double();
          break;
        case PROTON_FLOAT:
          sig_desc.signal.signal.float_value = static_cast<float>(value_node.as_double());
          break;
        case PROTON_INT32:
          sig_desc.signal.signal.int32_value = static_cast<int32_t>(value_node.as_int64());
          break;
        case PROTON_INT64:
          sig_desc.signal.signal.int64_value = value_node.as_int64();
          break;
        case PROTON_UINT32:
          sig_desc.signal.signal.uint32_value = static_cast<uint32_t>(value_node.as_uint64());
          break;
        case PROTON_UINT64:
          sig_desc.signal.signal.uint64_value = value_node.as_uint64();
          break;
        case PROTON_BOOL:
          sig_desc.signal.signal.bool_value = value_node.as_bool();
          break;
        case PROTON_STRING:
        case PROTON_BYTES:
          // String/bytes default values handled separately via decode buffers
          break;
        default:
          break;
      }
    }

    signal_registry_.push_back(sig_desc);

    // Allocate decode buffer for string/bytes signals
    if (sig_type == PROTON_STRING || sig_type == PROTON_BYTES)
    {
      signal_decode_buffer_storage_.emplace_back(signal_cfg.capacity, 0);
      sig_desc.signal_decode_buffer = signal_decode_buffer_storage_.back().data();
    }
    else
    {
      signal_decode_buffer_storage_.emplace_back();  // Empty vector for non-string/bytes
    }
  }
}

void GeneratedNode::generate_bundles(const Config & config)
{
  ProducerConsumerIds prod_con_ids = set_producer_consumer_ids(config);
  bundle_producer_ids_ = std::move(prod_con_ids.producer_ids);
  bundle_consumer_ids_ = std::move(prod_con_ids.consumer_ids);

  bundle_table_.reserve(config.bundles.size());
  size_t max_signal_count = 0;

  for (size_t idx = 0; idx < config.bundles.size(); idx++)
  {
    const auto & bundle_cfg = config.bundles[idx];

    if (max_signal_count < bundle_cfg.signals.size())
    {
      max_signal_count = bundle_cfg.signals.size();
    }

    // Store signal IDs for this bundle
    bundle_signal_ids_[bundle_cfg.id] = bundle_cfg.signals;

    // Create bundle descriptor
    bundle_desc_t bundle_desc = {
      .bundle_id = bundle_cfg.id,
      .producer_ids =
        {
          .ids = bundle_producer_ids_[bundle_cfg.id].data(),
          .count = static_cast<uint8_t>(bundle_producer_ids_[bundle_cfg.id].size()),
        },
      .consumer_ids =
        {
          .ids = bundle_consumer_ids_[bundle_cfg.id].data(),
          .count = static_cast<uint8_t>(bundle_consumer_ids_[bundle_cfg.id].size()),
        },
      .signal_ids =
        {
          .ids = bundle_signal_ids_[bundle_cfg.id].data(),
          .count = static_cast<uint8_t>(bundle_signal_ids_[bundle_cfg.id].size()),
        },
      .last_send_ms = 0,
      .period_ms = bundle_cfg.period_ms,
      .send_now = false,
      .callback =
        {
          .cb = nullptr,
          .arg = nullptr,
        },
    };
    bundle_table_.push_back(bundle_desc);
  }

  // Build space for encode/decode buffer (largest bundle signal count)
  bundle_encode_decode_buffer_.reserve(max_signal_count);
}

void GeneratedNode::init_registry()
{
  // Bundle table and lookups
  registry_.bundle_table = bundle_table_.data();
  registry_.bundle_count = bundle_table_.size();

  // Signal table and lookups
  registry_.signal_registry = signal_registry_.data();
  registry_.encode_decode_buffer = bundle_encode_decode_buffer_.data();
  registry_.signal_count = signal_registry_.size();

  // Scratch buffer for string/bytes encoding/decoding
  registry_.signal_scratch_buffer =
    signal_scratch_buffer_.empty() ? nullptr : signal_scratch_buffer_.data();
  registry_.signal_scratch_buffer_size = signal_scratch_buffer_.size();

  // No mutex by default (user can set if needed)
  registry_.mutex_handles = {.lock = nullptr, .unlock = nullptr, .mutex = nullptr, .arg = nullptr};
}

void GeneratedNode::init_node(const Config & config, const std::string & target_name)
{
  node_.id = config.nodes.at(target_name).id;
  node_.destination_peers =
    node_destination_peers_.empty() ? nullptr : node_destination_peers_.data();
  node_.num_peers = node_destination_peers_.size();
  node_.registry = &registry_;
  node_.trigger_head = 0;
  node_.trigger_tail = 0;

  // Initialize pending triggers to zero
  for (size_t i = 0; i < PROTON_MAX_PENDING_TRIGGERS; i++)
  {
    node_.pending_triggers[i] = 0;
  }
}

}  // namespace proton::node_builder

#endif  // PROTON_NODE_BUILDER
