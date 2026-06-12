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
        const auto & conn_node = config.nodes.at(*conn_to_add);
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

}  // namespace proton::node_builder

#endif  // PROTON_NODE_BUILDER
