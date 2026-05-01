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

"""Configuration validation"""

from typing import List

from internal_types import INTERNAL_TYPE_MAP


def validate_node_elements(node: dict):
    """Validate that required configuration elements are in place
    ARGS:
        nodes: node stanza in proton config
    """

    required_top_level_elements = {"name": str, "endpoints": List[dict]}
    required_endpoint_elements = {"id": str, "type": "str"}
    required_endpoint_configs = {"serial": ["ip", "port"], "udp4": ["device"]}

    for tl, tl_type in required_top_level_elements.items():
        if tl not in node:
            raise RuntimeError(f"Element {tl} not in {node}")
        if not isinstance(node[tl], tl_type):
            raise RuntimeError(f"Element {tl} is not type {tl_type}")

    node_name = node["name"]
    endpoints = node["endpoints"]
    for endpoint in endpoints:
        for ep in required_endpoint_elements:
            if ep not in endpoint:
                raise RuntimeError(
                    f"Endpoint element {ep} not in {node_name}/endpoints"
                )

        endpoint_type = endpoint["type"]
        if endpoint_type not in required_endpoint_configs:
            raise RuntimeError(
                f"Endpoint type {endpoint_type} not in {required_endpoint_configs.keys()}"
            )

        for ep_c in required_endpoint_configs[endpoint_type]:
            if ep_c not in endpoint:
                raise RuntimeError(
                    f"Required endpoint element {ep_c} not found in {node_name}/{endpoint}"
                )


def validate_signal_elements(signal: dict):
    """Validate that required elements of the signal config
    ARGS:
        signal: signal element from the bundles/signals config
    """

    signal_required_elements = ["name", "type"]
    for rq_elem in signal_required_elements:
        if rq_elem not in signal:
            raise RuntimeError(f"Required element {rq_elem} not in signal {signal}")

    signal_type = signal["type"]
    if signal_type not in INTERNAL_TYPE_MAP:
        raise RuntimeError(f"Signal type {signal_type} is not supported")
