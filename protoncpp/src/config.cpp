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

#include "protoncpp/config.hpp"
#include "yaml-cpp/yaml.h"
#include <iostream>

using namespace proton::config;

SignalConfig::SignalConfig()
    : name_(""), type_string_(""), length_(0), capacity_(0) {}

SignalConfig::SignalConfig(std::string name, std::string type,
                           uint32_t length = 0, uint32_t capacity = 0)
    : name_(name), type_string_(type), length_(length), capacity_(capacity) {}

BundleConfig::BundleConfig()
    : name_(""), id_(0), producer_(""), consumer_("") {}

BundleConfig::BundleConfig(std::string name, uint32_t id, std::string producer,
                           std::string consumer,
                           std::vector<SignalConfig> signals)
    : name_(name), id_(id), producer_(producer), consumer_(consumer),
      signals_(signals) {}

Config::Config(std::string file) {
  YAML::Node config = YAML::LoadFile(file);

  for (auto b : config["bundles"]) {
    std::vector<SignalConfig> signal_configs;

    for (auto s : b["schema"]) {
      uint32_t length, capacity;

      try {
        length = s["length"].as<uint32_t>();
      } catch (const YAML::TypedBadConversion<uint32_t> &e) {
        length = 0;
      }

      try {
        capacity = s["capacity"].as<uint32_t>();
      } catch (const YAML::TypedBadConversion<uint32_t> &e) {
        capacity = 0;
      }

      signal_configs.push_back(SignalConfig(s["signal"].as<std::string>(),
                                            s["type"].as<std::string>(), length,
                                            capacity));
    }

    BundleConfig bundle_config =
        BundleConfig(b["name"].as<std::string>(), b["id"].as<uint32_t>(),
                     b["producer"].as<std::string>(),
                     b["consumer"].as<std::string>(), signal_configs);

    bundles_.push_back(bundle_config);
  }
}
