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

"""Configuration validation."""

from collections import Counter

from internal_types import INTERNAL_TYPE_MAP


def validate_node_elements(node: dict):
    """
    Validate that required configuration elements are in place.

    Args:
        node: node stanza in proton config

    Raises:
        RuntimeError: if required keys are missing

    """
    required_top_level_elements = {'name': str, 'endpoints': list[dict]}
    required_endpoint_elements = {'id': str, 'type': 'str'}
    required_endpoint_configs = {'udp4': ['ip', 'port'], 'serial': ['device', 'baud']}

    for tl, tl_type in required_top_level_elements.items():
        if tl not in node:
            raise RuntimeError(f'Element {tl} not in {node}')
        if not isinstance(node[tl], tl_type):
            raise RuntimeError(f'Element {tl} is not type {tl_type}')

    node_name = node['name']
    endpoints = node['endpoints']
    for endpoint in endpoints:
        for ep in required_endpoint_elements:
            if ep not in endpoint:
                raise RuntimeError(f'Endpoint element {ep} not in {node_name}/endpoints')

        endpoint_type = endpoint['type']
        if endpoint_type not in required_endpoint_configs:
            raise RuntimeError(
                f'Endpoint type {endpoint_type} not in {required_endpoint_configs.keys()}'
            )

        for ep_c in required_endpoint_configs[endpoint_type]:
            if ep_c not in endpoint:
                raise RuntimeError(
                    f'Required endpoint element {ep_c} not found in {node_name}/{endpoint}'
                )


def validate_signal_elements(signal: dict):
    """
    Validate that required elements of the signal config.

    Args:
        signal: signal element from the bundles/signals config
    Raises:
        RuntimeError: if required elements are missing

    """
    signal_required_elements = ['name', 'type']
    for rq_elem in signal_required_elements:
        if rq_elem not in signal:
            raise RuntimeError(f'Required element {rq_elem} not in signal {signal}')

    signal_type = signal['type']
    if signal_type not in INTERNAL_TYPE_MAP:
        raise RuntimeError(f'Signal type {signal_type} is not supported')


def validate_ids(id_list: list, item_name: str):
    """
    Validate that there are no duplicate signal or bundle ID's.

    Args:
        id_list: list of items from the config with ID fields
        item_name: name of the list from the config
    Raises:
        KeyError: if signal or bundle ID is missing
        RuntimeError: if there are duplicate ID's

    """
    ids = [item['id'] for item in id_list]
    counts = Counter(ids)
    duplicates = [item for item, count in counts.items() if count > 1]
    if len(duplicates) > 0:
        formatted = [f'0x{x:02X}' for x in duplicates]
        raise RuntimeError(f'Found duplicate {item_name} IDs: {formatted}')


def validate_target_consumers_connected(
    nodes: list[dict], connections: list[dict], bundles: list[dict], target: str
):
    """
    Validate that every consumer of a bundle produced by the target is reachable from it.

    Args:
        nodes: "nodes" stanza in proton config
        connections: "connections" stanza in proton config
        bundles: "bundles" stanza in proton config
        target: name of node being generated
    Raises:
        RuntimeError: if a target connection references an undefined endpoint, or a
                      bundle produced by the target has a consumer with no connection

    """
    endpoints = {(node['name'], ep['id']) for node in nodes for ep in node['endpoints']}

    connected = set()
    for conn in connections:
        for local, remote in ((conn['first'], conn['second']), (conn['second'], conn['first'])):
            if local['node'] != target:
                continue
            if (remote['node'], remote['id']) not in endpoints:
                raise RuntimeError(
                    f'Connection from {target} references undefined endpoint '
                    f'{remote["node"]}/{remote["id"]}'
                )
            connected.add(remote['node'])

    for bundle in bundles:
        if target not in bundle.get('producers', []):
            continue
        missing = [c for c in bundle.get('consumers', []) if c not in connected]
        if missing:
            raise RuntimeError(
                f'Bundle {bundle["name"]} produced by {target} has consumers '
                f'with no connection to {target}: {missing}'
            )
