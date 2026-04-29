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


def parse_node_endpoint_address(nodes: List[dict]):
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


def set_signal_repeated_types(bundles: List[dict]):
    """Check signals to determine if they're a repeated type, as that affects code generation
    ARGS:
        bundles: "bundles" stanza in proton config
    """
    for bundle in bundles:
        if "signals" in bundle:
            for signal in bundle["signals"]:
                signal_type = signal["type"]
                signal["is_repeated_type"] = signal_type.startswith("list_") or signal_type in ("string", "bytes")


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

    parse_node_endpoint_address(config["nodes"])
    set_signal_repeated_types(config["bundles"])

    generate_header(dest_path, config, name, target)
    generate_source(dest_path, config, name, target)


if __name__ == "__main__":
    main()
