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


"""Normalize elements of the proton config."""

from config import validate_signal_elements
from internal_types import DEFAULT_VALUE_MAP, INTERNAL_TYPE_MAP


def normalize_signals(signals: list[dict]):
    """
    Normalize signal configuration for optional node elements.

    Args:
        signals: signals stanza from proton config

    Raises:
        RuntimeError: if specified default value for string/bytes signals
                      is less than a specified capacity

    """
    for signal in signals:
        validate_signal_elements(signal)

        signal.setdefault('capacity', 0)

        signal_type = signal['type']

        is_capacity_type = 'bytes' in signal_type or 'string' in signal_type
        has_default_value = signal.get('value') is not None

        signal['is_capacity_type'] = is_capacity_type
        signal['has_default_value'] = has_default_value
        capacity = signal['capacity']

        signal['internal_type'] = INTERNAL_TYPE_MAP[signal_type]
        if signal_type in DEFAULT_VALUE_MAP and not has_default_value:
            signal['value'] = DEFAULT_VALUE_MAP[signal_type]

        # Special case for capacity types. If a value is specified but no capacity,
        # assume the capacity is the length of the default value. For strings, this is +1
        # due to the null char at the end.
        # If the capacity is not 0 (default) but is less than the length of the value,
        # throw an error
        if is_capacity_type:
            if has_default_value:
                # 0 is default for unspecified capacities
                if 'bytes' in signal_type:
                    if capacity == 0:
                        signal['capacity'] = len(signal['value'])
                    elif capacity < len(signal['value']):
                        raise RuntimeError(
                            f'Signal capacity {capacity} '
                            f'is less than default value: {len(signal["value"])}'
                        )
                elif 'string' in signal_type:
                    if capacity == 0 or capacity == len(signal['value']):
                        signal['capacity'] = len(signal['value']) + 1
                    elif capacity < len(signal['value']):
                        raise RuntimeError(
                            f'Signal capacity {capacity} '
                            f'is less than default value: {len(signal["value"])}'
                        )
                else:
                    raise RuntimeError(f'Unknown signal type {signal_type}')
            elif 'bytes' in signal_type:
                signal['value'] = [0] * capacity


def set_node_endpoint_address(nodes: list[dict]):
    """
    Read node IP configuration and add IPNL and IPHL elements.

    Args:
        nodes: "nodes" stanza in proton config

    """
    for node in nodes:
        for endpoint in node['endpoints']:
            if endpoint['type'] == 'udp4':
                ip_split = endpoint['ip'].split('.')
                ip_hl = 0
                ip_nl = 0
                for i, octet in enumerate(ip_split):
                    ip_hl |= int(octet) << 8 * (3 - i)
                    ip_nl |= int(octet) << 8 * i
                endpoint['iphl'] = ip_hl
                endpoint['ipnl'] = ip_nl


def set_producer_consumer_ids(bundles: list[dict], nodes: list[dict]):
    """
    Set ids of producers and consumers for each bundle.

    Args:
        bundles: "bundles" stanza in proton config
        nodes: "nodes" stanza in proton config

    """
    node_id_map = {node['name']: node['id'] for node in nodes}

    for bundle in bundles:
        bundle['producer_ids'] = [node_id_map[name] for name in bundle.get('producers', [])]
        bundle['consumer_ids'] = [node_id_map[name] for name in bundle.get('consumers', [])]


def set_bundle_periods(bundles: list[dict]):
    """
    Set the transmission period of each bundle, defaulting to 0.

    Args:
        bundles: "bundles" stanza in proton config

    """
    for bundle in bundles:
        bundle.setdefault('period_ms', 0)


def filter_for_target(
    bundles: list[dict], signals: list[dict], target: str
) -> tuple[list[dict], list[dict]]:
    """
    Filter out all signals and bundles that a target does not care about.

    Args:
        bundles: "bundles" stanza in proton config
        signals: "signals" stanza in proton config
        target: name of node being generated

    Returns:
        tuple of bundles and signals that the target cares about

    """
    filtered_bundles = [
        bundle
        for bundle in bundles
        if target in bundle['producers'] or target in bundle['consumers']
    ]
    filtered_signal_ids = set()
    for bundle in filtered_bundles:
        filtered_signal_ids.update(bundle['signals'])

    filtered_signals = [signal for signal in signals if signal['id'] in filtered_signal_ids]

    return (filtered_bundles, filtered_signals)
