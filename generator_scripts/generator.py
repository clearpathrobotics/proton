#!/usr/bin/env python3

# Copyright 2026 Rockwell Automation Technologies, Inc., All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# @author Tom Wallis (thomas.wallis@rockwellautomation.com)


"""Generator for protonc C code"""

import argparse
import os
from pathlib import Path
import sys
from typing import List

from jinja2 import Template
import yaml


sys.path.insert(0, os.path.dirname(__file__))

"""Mapping of protobuf Signal types to C internal types"""
INTERNAL_TYPE_MAP = {
    "double": "double",
    "float": "float",
    "int32": "int32_t",
    "int64": "int64_t",
    "uint32": "uint32_t",
    "uint64": "uint64_t",
    "bool": "bool",
    "string": "char",
    "bytes": "uint8_t",
    "list_double": "double",
    "list_float": "float",
    "list_int32": "int32_t",
    "list_int64": "int64_t",
    "list_uint32": "uint32_t",
    "list_uint64": "uint64_t",
    "list_bool": "bool",
    "list_string": "char",
    "list_bytes": "uint8_t",
}

"""Default values for particular types"""
DEFAULT_VALUE_MAP = {
    "double": "0.0f",
    "float": "0.0f",
    "int32": 0,
    "int64": 0,
    "uint32": 0,
    "uint64": 0,
    "bool": "false",
}

def load_config(config_path: str) -> dict:
    """Read and parse proton configs
    ARGS:
        config: string path to proton config file
    THROWS:
        RuntimeError if there are issues with the yaml construction or syntax
        AssertionError if parsed yaml is not a dict
    RETURNS:
        yaml config as dict
    """
    try:
        with open(config_path, "r") as f:
            config = yaml.safe_load(f)
    except yaml.scanner.ScannerError as s:
        raise RuntimeError(f"YAML file {config_path} is not well formed") from s
    except yaml.constructor.ConstructorError as c:
        raise RuntimeError(
            f"YAML file {config_path} is attempting to create unsafe objects"
        ) from c
    # Check contents are a Dictionary
    assert isinstance(config, dict), (
        f"YAML file {config_path} is not a dictionary, is {type(config)}"
    )
    return config


def set_node_endpoint_address(nodes: List[dict]):
    """Read node IP configuration and add IPNL and IPHL elements
    ARGS:
        nodes: "nodes" stanza in proton config
    """

    for node in nodes:
        for endpoint in node["endpoints"]:
            if endpoint["type"] == "udp4":
                ip_split = endpoint["ip"].split(".")
                ip_hl = 0
                ip_nl = 0
                for (i, octet) in enumerate(ip_split):
                    ip_hl |= int(octet) << 8 * (3 - i)
                    ip_nl |= int(octet) << 8 * i
                endpoint["iphl"] = ip_hl
                endpoint["ipnl"] = ip_nl


def set_heartbeat_producers_consumers(nodes: List[dict], connections: List[dict]):
    """Determine which heartbeats go where
    ARGS:
        nodes: "nodes" stanza in proton config
        connections: "connections" stanza in proton config
    """

    for node in nodes:
        if node.get("heartbeat") is not None:
            if node["heartbeat"]["enabled"]:
                node["heartbeat"]["producers"] = node["name"]
                for connection in connections:
                    if node["heartbeat"].get("consumers") is None:
                        node["heartbeat"]["consumers"] = []
                    if connection["first"]["node"] == node["name"]:
                        node["heartbeat"]["consumers"].append(connection["second"]["node"])
                    if connection["second"]["node"] == node["name"]:
                        node["heartbeat"]["consumers"].append(connection["first"]["node"])


def set_signal_properties(bundles: List[dict]):
    """Set properties for signals. Default values, repeated type lengths, constness
    ARGS:
        bundles: "bundles" stanza in proton config
    """
    for bundle in bundles:
        if "signals" in bundle:
            for signal in bundle["signals"]:
                signal_type = signal["type"]

                signal["is_repeated_type"] = signal_type.startswith("list_") or signal_type in ("string", "bytes")

                if signal_type in INTERNAL_TYPE_MAP:
                    signal["internal_type"] = INTERNAL_TYPE_MAP[signal_type]

                signal["is_const"] = signal.get("value") is not None
                if signal["is_const"]:
                    # Special case for strings: they must have a capacity variable generated in the template,
                    # Even if they're already defined as consts. The length is equal to the strlen + 1 for the nullchar in C
                    if signal_type == "string":
                        if "capacity" not in signal:
                            signal["capacity"] = len(signal["value"]) + 1
                    elif signal["is_repeated_type"]:
                        # Special case for const repeated signals that aren't strings:
                        # the config doesn't contain the length field
                        signal["length"] = len(signal["value"])
                elif signal_type in DEFAULT_VALUE_MAP:
                    signal["value"] = DEFAULT_VALUE_MAP[signal_type]


def generate_header(dest_path: Path, config: dict, name: str, target: str):
    """Generate protonc file according to template
    ARGS:
        dest_path: path to destination folder
        config: proton config
        name: name of proton "project" (for lack of a better term)
        target: name of peer on the proton network
    """

    with open(f"resources/proton__header_node.h.jinja", "r") as f:
        template_content = f.read()

    template = Template(template_content)

    output = template.render(name=name, target=target, nodes=config["nodes"], bundles=config["bundles"])

    output_header = dest_path / f"proton__{name}_{target}.h"

    with open(output_header, "w") as f:
        f.write(output)


def generate_source(dest_path: Path, config: dict, name: str, target: str):
    """Generate protonc file according to template
    ARGS:
        dest_path: path to destination folder
        config: proton config
        name: name of proton "project" (for lack of a better term)
        target: name of peer on the proton network
    """

    with open(f"resources/proton__source_node.c.jinja", "r") as f:
        template_content = f.read()

    template = Template(template_content)

    output = template.render(name=name, target=target, nodes=config["nodes"], bundles=config["bundles"])

    output_header = dest_path / f"proton__{name}_{target}.c"

    with open(output_header, "w") as f:
        f.write(output)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-c",
        "--config",
        type=str,
        help="Configuration file path",
    )

    parser.add_argument(
        "-d",
        "--destination",
        type=str,
        help="Destination folder path for generated files",
    )

    parser.add_argument(
        "-t",
        "--target",
        type=str,
        help="Target node for generation",
    )

    parser.add_argument(
      "-n",
      "--node",
      type=bool,
      default=False,
      help="Generate code for node and transport implementation",
    )

    args = parser.parse_args()

    config_path = args.config
    dest_path = Path(args.destination)
    target = args.target
    name = Path(config_path).stem

    config = load_config(config_path)

    # validate node config here

    set_node_endpoint_address(config["nodes"])
    set_heartbeat_producers_consumers(config["nodes"], config["connections"])
    set_signal_properties(config["bundles"])

    generate_header(dest_path, config, name, target)
    generate_source(dest_path, config, name, target)


if __name__ == "__main__":
    main()
