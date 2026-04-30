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


"""Nnormalize elements of the proton config"""

from typing import List

from config import validate_signal_elements
from internal_types import DEFAULT_VALUE_MAP, INTERNAL_TYPE_MAP


def normalize_node_heartbeats(nodes: List[dict]):
    """Normalize the configuration for whether or not heartbeats are present
    ARGS:
        nodes: "nodes" stanza in proton config
    """

    default_heartbeat = {"enabled": False, "period": 0}

    for node in nodes:
        node.setdefault("heartbeat", default_heartbeat)


def normalize_signals(bundle: dict):
    """Normalize signal configuration for optional node elements
    ARGS:
        bundle: a bundle from proton config
    """

    if "signals" in bundle:
        for signal in bundle["signals"]:
            validate_signal_elements(signal)

            signal.setdefault("length", 0)
            signal.setdefault("capacity", 0)

            signal_type = signal["type"]

            is_list_type = signal_type.startswith("list_")
            is_capacity_type = "bytes" in signal_type or "string" in signal_type
            # TODO CORE-37812 remote const-ness in signals
            is_const = signal.get("value") is not None

            signal["is_list_type"] = is_list_type
            signal["is_capacity_type"] = is_capacity_type
            signal["is_repeated_type"] = is_list_type or is_capacity_type
            signal["is_const"] = is_const

            signal["internal_type"] = INTERNAL_TYPE_MAP[signal_type]
            if signal_type in DEFAULT_VALUE_MAP:
                signal["value"] = DEFAULT_VALUE_MAP[signal_type]

            if is_list_type and not isinstance(signal["value"], List):
                val = signal["value"]
                signal["value"] = list(val)

            if is_const:
                # Special case for capacity types: they must have a capacity variable generated in the template,
                # Even if they're already defined as consts. The length is equal to the strlen + 1 for the nullchar in C
                if is_capacity_type:
                    if is_list_type:
                        max_cap = 0
                        for elem in signal["value"]:
                            max_cap = len(elem) + 1 if len(elem) > max_cap else max_cap
                        signal["capacity"] = max_cap
                    else:
                        signal["capacity"] = len(signal["value"]) + 1

                if is_list_type:
                    # Special case for const repeated signals that aren't strings:
                    # the config doesn't contain the length field
                    signal["length"] = len(signal["value"])


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
                for i, octet in enumerate(ip_split):
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
                        node["heartbeat"]["consumers"].append(
                            connection["second"]["node"]
                        )
                    if connection["second"]["node"] == node["name"]:
                        node["heartbeat"]["consumers"].append(
                            connection["first"]["node"]
                        )


def set_signal_properties(bundles: List[dict]):
    """Set properties for signals. Default values, repeated type lengths, constness
    ARGS:
        bundles: "bundles" stanza in proton config
    """
    for bundle in bundles:
        normalize_signals(bundle)
