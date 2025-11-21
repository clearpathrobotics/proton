# Software License Agreement (BSD)
#
# @author    Roni Kreinin <rkreinin@clearpathrobotics.com>
# @copyright (c) 2025, Clearpath Robotics, Inc., All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# * Redistributions of source code must retain the above copyright notice,
#   this list of conditions and the following disclaimer.
# * Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
# * Neither the name of Clearpath Robotics nor the names of its contributors
#   may be used to endorse or promote products derived from this software
#   without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

from enum import StrEnum, auto
from typing import List


class ProtonConfig:
    # Top level keys
    NODES = "nodes"
    BUNDLES = "bundles"

    class Signal:
        # Signal keys
        NAME = "name"
        TYPE = "type"
        LENGTH = "length"
        CAPACITY = "capacity"
        VALUE = "value"

        SIGNAL_ENUM_PREFIX = "PROTON_SIGNALS__"
        LENGTH_SUFFIX = "__LENGTH"
        CAPACITY_SUFFIX = "__CAPACITY"
        VALUE_SUFFIX = "__DEFAULT_VALUE"

        # Signal types
        class SignalTypes(StrEnum):
            DOUBLE = auto()
            FLOAT = auto()
            INT32 = auto()
            INT64 = auto()
            UINT32 = auto()
            UINT64 = auto()
            BOOL = auto()
            STRING = auto()
            BYTES = auto()
            LIST_DOUBLE = auto()
            LIST_FLOAT = auto()
            LIST_INT32 = auto()
            LIST_INT64 = auto()
            LIST_UINT32 = auto()
            LIST_UINT64 = auto()
            LIST_BOOL = auto()
            LIST_STRING = auto()
            LIST_BYTES = auto()

        DEFAULT_VALUES = {
            SignalTypes.DOUBLE: 0.0,
            SignalTypes.FLOAT: 0.0,
            SignalTypes.INT32: 0,
            SignalTypes.INT64: 0,
            SignalTypes.UINT32: 0,
            SignalTypes.UINT64: 0,
            SignalTypes.BOOL: False,
            SignalTypes.STRING: "",
            SignalTypes.BYTES: {},
            SignalTypes.LIST_DOUBLE: {},
            SignalTypes.LIST_FLOAT: {},
            SignalTypes.LIST_INT32: {},
            SignalTypes.LIST_INT64: {},
            SignalTypes.LIST_UINT32: {},
            SignalTypes.LIST_UINT64: {},
            SignalTypes.LIST_BOOL: {},
            SignalTypes.LIST_STRING: {},
            SignalTypes.LIST_BYTES: {}
        }

        def __init__(self, bundle: str, signal: dict):
            self.bundle: str = bundle
            self.name: str = signal[self.NAME]
            self.type: ProtonConfig.Signal.SignalTypes = (
                ProtonConfig.Signal.SignalTypes(signal[self.TYPE])
            )
            self.length: int = 0
            self.capacity: int = 0

            try:
                self.value = signal[self.VALUE]
            except KeyError:
                self.value = None

            try:
                self.length = signal[self.LENGTH]
            except KeyError:
                if (self.value is not None and
                   isinstance(self.value, List) and
                   self.type is not ProtonConfig.Signal.SignalTypes.BYTES):
                    self.length = len(self.value)

            try:
                self.capacity = signal[self.CAPACITY]
            except KeyError:
                if self.value is not None:
                    if self.type == ProtonConfig.Signal.SignalTypes.STRING:
                        if isinstance(self.value, List):
                            for s in self.value:
                                if len(s) + 1 > self.capacity:
                                    self.capacity = len(s) + 1
                        else:
                            self.capacity = len(self.value) + 1 # +1 for null termination
                    elif self.type == ProtonConfig.Signal.SignalTypes.BYTES:
                        self.capacity = len(self.value)

            match self.type:
                case (ProtonConfig.Signal.SignalTypes.BYTES | ProtonConfig.Signal.SignalTypes.STRING):
                    assert self.capacity > 0, (
                        f"{self.type} type signals must have a non-zero capacity"
                    )
                case (
                    ProtonConfig.Signal.SignalTypes.LIST_DOUBLE
                    | ProtonConfig.Signal.SignalTypes.LIST_FLOAT
                    | ProtonConfig.Signal.SignalTypes.LIST_INT32
                    | ProtonConfig.Signal.SignalTypes.LIST_INT64
                    | ProtonConfig.Signal.SignalTypes.LIST_UINT32
                    | ProtonConfig.Signal.SignalTypes.LIST_UINT64
                    | ProtonConfig.Signal.SignalTypes.LIST_BOOL
                ):
                    assert self.length > 0, (
                        f"{self.type} type signals must have a non-zero length"
                    )
                case (ProtonConfig.Signal.SignalTypes.LIST_STRING | ProtonConfig.Signal.SignalTypes.LIST_BYTES):
                    assert self.length > 0, (
                        f"{self.type} type signals must have a non-zero length"
                    )
                    assert self.capacity > 0, (
                        f"{self.type} type signals must have a non-zero capacity"
                    )

            self.signal_enum_name = f'{self.SIGNAL_ENUM_PREFIX}{self.bundle.upper()}__{self.name.upper()}'
            self.capacity_define = f'{self.SIGNAL_ENUM_PREFIX}{self.bundle.upper()}__{self.name.upper()}{self.CAPACITY_SUFFIX}'
            self.length_define = f'{self.SIGNAL_ENUM_PREFIX}{self.bundle.upper()}__{self.name.upper()}{self.LENGTH_SUFFIX}'
            self.value_define = f'{self.SIGNAL_ENUM_PREFIX}{self.bundle.upper()}__{self.name.upper()}{self.VALUE_SUFFIX}'

            self.is_const = self.value is not None

            if not self.is_const:
                self.value = self.DEFAULT_VALUES[self.type]

            self.c_value = None

            match self.type:
                case (ProtonConfig.Signal.SignalTypes.DOUBLE |
                      ProtonConfig.Signal.SignalTypes.INT32 |
                      ProtonConfig.Signal.SignalTypes.INT64 |
                      ProtonConfig.Signal.SignalTypes.UINT32 |
                      ProtonConfig.Signal.SignalTypes.UINT64):
                    self.c_value = self.value
                case (ProtonConfig.Signal.SignalTypes.FLOAT):
                    self.c_value = f"{self.value}f"
                case ProtonConfig.Signal.SignalTypes.BOOL:
                    self.c_value = "true" if self.value else "false"
                case ProtonConfig.Signal.SignalTypes.STRING:
                    self.c_value = f"\"{self.value}\""
                case (ProtonConfig.Signal.SignalTypes.LIST_FLOAT):
                    list_def = ""
                    for (i, v) in enumerate(self.value):
                        if i != len(self.value) - 1:
                            list_def += f"{v}f, "
                        else:
                            list_def += f"{v}f"
                    self.c_value =f"{{{list_def}}}"
                case (ProtonConfig.Signal.SignalTypes.LIST_BOOL):
                    list_def = ""
                    for (i, v) in enumerate(self.value):
                        if i != len(self.value) - 1:
                            list_def += f"{"true" if v else "false"}, "
                        else:
                            list_def += f"{"true" if v else "false"}"
                    self.c_value =f"{{{list_def}}}"
                case (ProtonConfig.Signal.SignalTypes.LIST_STRING):
                    list_def = ""
                    for (i, v) in enumerate(self.value):
                        if i != len(self.value) - 1:
                            list_def += f"\"{v}\", "
                        else:
                            list_def += f"\"{v}\""
                    self.c_value =f"{{{list_def}}}"
                case _:
                    list_def = ""
                    for (i, v) in enumerate(self.value):
                        if i != len(self.value) - 1:
                            list_def += f"{v}, "
                        else:
                            list_def += f"{v}"
                    self.c_value = f"{{{list_def}}}"


    class Bundle:
        # Bundle keys
        NAME = "name"
        ID = "id"
        PRODUCER = "producer"
        CONSUMER = "consumer"
        SIGNALS = "signals"
        BUNDLE_SUFFIX = "_bundle"
        HANDLE_SUFFIX = "_handle"
        HEARTBEAT_STRUCT_SUFFIX = "_heartbeat"
        BUNDLE_STRUCT_PREFIX = "PROTON_BUNDLE__"
        BUNDLE_SIGNAL_ENUM_PREFIX = "PROTON_SIGNALS__"
        BUNDLE_ID_PREFIX = "PROTON_BUNDLE_ID__"
        SIGNAL_HANDLES_SUFFIX = "_signal_handles"
        INIT_FUNCTION_SUFFIX = "PROTON_BUNDLE_Init"
        CALLBACK_PREFIX = "PROTON_BUNDLE_"
        CALLBACK_SUFFIX = "Callback"
        DEFAULT_VALUE_SUFFIX = "__DEFAULT_VALUE"
        PRODUCERS_SUFFIX = "__PRODUCERS"
        CONSUMERS_SUFFIX = "__CONSUMERS"
        HEARTBEAT_BUNDLE_ID = "PROTON_BUNDLE__HEARTBEAT"

        def __init__(self, bundle: dict):
            self.name = bundle[self.NAME]
            self.id = bundle[self.ID]
            self.producer = bundle[self.PRODUCER]
            self.consumer = bundle[self.CONSUMER]
            self.signals: List[ProtonConfig.Signal] = []
            self.needs_init = False

            self.bundle_enum_name = f'{self.BUNDLE_STRUCT_PREFIX}{self.name.upper()}'
            self.internal_handle_variable_name = f'_{self.name}{self.HANDLE_SUFFIX}'
            self.signal_handles_variable_name = f'_{self.name}{self.SIGNAL_HANDLES_SUFFIX}'
            self.struct_name = f'{self.BUNDLE_STRUCT_PREFIX}{self.name}'
            self.bundle_variable_name = f'{self.name}{self.BUNDLE_SUFFIX}'
            self.signals_enum_name = f'{self.BUNDLE_SIGNAL_ENUM_PREFIX}{self.name}'
            self.signals_enum_count = f'{self.signals_enum_name.upper()}_COUNT'
            self.init_function_name = f'{self.INIT_FUNCTION_SUFFIX}{self.name.title().replace('_', '')}'
            self.callback_function_name = f'{self.CALLBACK_PREFIX}{self.name.title().replace('_', '')}{self.CALLBACK_SUFFIX}'
            self.default_value_define = f'{self.bundle_enum_name}{self.DEFAULT_VALUE_SUFFIX}'
            self.default_value = []
            self.producers_define = f'{self.BUNDLE_STRUCT_PREFIX}{self.name.upper()}{self.PRODUCERS_SUFFIX}'
            self.consumers_define = f'{self.BUNDLE_STRUCT_PREFIX}{self.name.upper()}{self.CONSUMERS_SUFFIX}'

            try:
                for signal in bundle[self.SIGNALS]:
                    s = ProtonConfig.Signal(self.name, signal)
                    self.signals.append(s)
                    if s.type == ProtonConfig.Signal.SignalTypes.LIST_STRING:
                        self.needs_init = True
            except KeyError:
                pass

    class Node:
        # Node keys
        NAME = "name"
        TRANSPORT = "transport"
        TYPE = "type"
        IP = "ip"
        PORT = "port"
        DEVICE = "device"
        HEARTBEAT = "heartbeat"
        ENABLED = "enabled"
        PERIOD = "period"

        # Transport types
        UDP4 = "udp4"
        SERIAL = "serial"

        NODE_PREFIX = "PROTON_NODE__"
        NODE_ID_PREFIX = "PROTON_NODE_ID__"
        NAME_SUFFIX = "__NAME"
        IP_SUFFIX = "__IP"
        PORT_SUFFIX = "__PORT"
        DEVICE_SUFFIX = "__DEVICE"
        DEFAULT_VALUE_SUFFIX = "__DEFAULT_VALUE"
        PEERS_SUFFIX = "__PEERS"
        PEER_PREFIX = "PROTON_PEER__"
        INIT_SUFFIX = "Init"
        RECEIVE_SUFFIX = "Receive"

        TRANSPORT_PREFIX = "PROTON_TRANSPORT__"
        TRANSPORT_CONNECT = "Connect"
        TRANSPORT_DISCONNECT = "Disconnect"
        TRANSPORT_READ = "Read"
        TRANSPORT_Write = "Write"

        HEARTBEAT_PREFIX = "PROTON_HEARTBEAT__"
        HEARTBEAT_ENABLED_SUFFIX = "__HEARTBEAT_ENABLED"
        HEARTBEAT_PERIOD_SUFFIX = "__HEARTBEAT_PERIOD"

        MUTEX_PREFIX = "PROTON_MUTEX__"
        MUTEX_LOCK = "Lock"
        MUTEX_UNLOCK = "Unlock"

        def __init__(self, node: dict, peers: List[dict]):
            self.id = 0
            self.peers = peers
            self.name = node[self.NAME]
            transport = node[self.TRANSPORT]
            self.type = transport[self.TYPE]
            self.name_define = f'{self.NODE_PREFIX}{self.name.upper()}{self.NAME_SUFFIX}'
            self.peer_define = f'{self.PEER_PREFIX}{self.name.upper()}'
            self.node_id_define = f'{self.NODE_ID_PREFIX}{self.name.upper()}'
            self.default_value_define = f'{self.NODE_PREFIX}{self.name.upper()}{self.DEFAULT_VALUE_SUFFIX}'
            self.peers_value_define = f'{self.NODE_PREFIX}{self.name.upper()}{self.PEERS_SUFFIX}{self.DEFAULT_VALUE_SUFFIX}'
            self.heartbeat_value_define = f'{self.HEARTBEAT_PREFIX}{self.name.upper()}{self.DEFAULT_VALUE_SUFFIX}'
            self.transport_value_define = f'{self.TRANSPORT_PREFIX}{self.name.upper()}{self.DEFAULT_VALUE_SUFFIX}'
            self.node_variable_name = f'{self.name}_node'
            self.peer_variable_name = f'{self.name}_peers'
            self.transport_connect_func = f'{self.TRANSPORT_PREFIX}{self.name.title()}{self.TRANSPORT_CONNECT}'
            self.transport_disconnect_func = f'{self.TRANSPORT_PREFIX}{self.name.title()}{self.TRANSPORT_DISCONNECT}'
            self.transport_read_func = f'{self.TRANSPORT_PREFIX}{self.name.title()}{self.TRANSPORT_READ}'
            self.transport_write_func = f'{self.TRANSPORT_PREFIX}{self.name.title()}{self.TRANSPORT_Write}'
            self.mutex_lock_func = f'{self.MUTEX_PREFIX}{self.name.title()}{self.MUTEX_LOCK}'
            self.mutex_unlock_func = f'{self.MUTEX_PREFIX}{self.name.title()}{self.MUTEX_UNLOCK}'
            self.peer_init_func = f'{self.PEER_PREFIX}{self.INIT_SUFFIX}{self.name.title()}'
            self.receive_func = f'{self.PEER_PREFIX}{self.RECEIVE_SUFFIX}{self.name.title()}'
            self.buffer_variable_name = f'proton_{self.name.lower()}_buffer'

            self.heartbeat_enabled = False
            self.heartbeat_period = 1000
            self.heartbeat_enabled_define = f'{self.NODE_PREFIX}{self.name.upper()}{self.HEARTBEAT_ENABLED_SUFFIX}'
            self.heartbeat_period_define = f'{self.NODE_PREFIX}{self.name.upper()}{self.HEARTBEAT_PERIOD_SUFFIX}'
            self.heartbeat_callback_function_name = f'{self.HEARTBEAT_PREFIX}{self.name.title()}Callback'

            try:
                heartbeat = node[self.HEARTBEAT]

                try:
                    if not isinstance(heartbeat[self.ENABLED], bool):
                        raise RuntimeError(f"Invalid heartbeat enabled {heartbeat[self.ENABLED]}")
                    self.heartbeat_enabled = heartbeat[self.ENABLED]
                except KeyError:
                    pass

                try:
                    if not isinstance(heartbeat[self.PERIOD], int) or heartbeat[self.PERIOD] <= 0:
                        raise RuntimeError(f"Invalid heartbeat period {heartbeat[self.PERIOD]}")
                    self.heartbeat_period = heartbeat[self.PERIOD]
                except KeyError:
                    pass
            except KeyError:
                pass

            match self.type:
                case self.UDP4:
                    self.ip = transport[self.IP]
                    self.port = transport[self.PORT]
                    self.ip_define = f'{self.NODE_PREFIX}{self.name.upper()}{self.IP_SUFFIX}'
                    self.port_define = f'{self.NODE_PREFIX}{self.name.upper()}{self.PORT_SUFFIX}'
                case self.SERIAL:
                    self.device = transport[self.DEVICE]
                    self.device_define = f'{self.NODE_PREFIX}{self.name.upper()}{self.DEVICE_SUFFIX}'

    def __init__(self, dictionary: dict):
        self.dictionary = dictionary
        self.nodes: List[ProtonConfig.Node] = []
        self.peers: List[ProtonConfig.Node] = []
        self.target_node: ProtonConfig.Node = None
        self.bundles: List[ProtonConfig.Bundle] = []
        self.heartbeats: List[ProtonConfig.Bundle] = []
        self.parse_nodes()
        self.parse_bundles()
        self.parse_heartbeats()

    def parse_nodes(self):
        try:
            nodes = self.dictionary[self.NODES]
        except KeyError:
            print("Nodes key missing")
            return
        for node in nodes:
            peers = [n for n in nodes if n != node]
            self.nodes.append(ProtonConfig.Node(node, peers))

    def parse_bundles(self):
        try:
            bundles = self.dictionary[self.BUNDLES]
        except KeyError:
            print("Bundles key missing")
            return
        for bundle in bundles:
            self.bundles.append(
                ProtonConfig.Bundle(bundle)
            )

    def parse_heartbeats(self):
        for node in self.nodes:
            if node.heartbeat_enabled:
                heartbeat_bundle_dict: dict = {}
                heartbeat_bundle_dict[ProtonConfig.Bundle.NAME] = f'{node.name}{ProtonConfig.Bundle.HEARTBEAT_STRUCT_SUFFIX}'
                heartbeat_bundle_dict[ProtonConfig.Bundle.ID] = 0
                heartbeat_bundle_dict[ProtonConfig.Bundle.PRODUCER] = node.name
                heartbeat_bundle_dict[ProtonConfig.Bundle.CONSUMER] = node.peers[0][ProtonConfig.Node.NAME]

                heartbeat_signal_dict: dict = {}
                heartbeat_signal_dict[ProtonConfig.Signal.NAME] = 'heartbeat'
                heartbeat_signal_dict[ProtonConfig.Signal.TYPE] = ProtonConfig.Signal.SignalTypes.UINT32

                heartbeat_bundle_dict[ProtonConfig.Bundle.SIGNALS] = [heartbeat_signal_dict]
                self.heartbeats.append(ProtonConfig.Bundle(heartbeat_bundle_dict))


    def set_target(self, target: str):
        for n in self.nodes:
            if n.name == target:
                self.target_node = n
            else:
                self.peers.append(n)

