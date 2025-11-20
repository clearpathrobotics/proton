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
from ipaddress import ip_address
from typing import List, Dict

class BaseConfig:
    class Define:
        def __init__(self, name: str, value):
            self.name = name
            self.value = value

        def __str__(self):
            return self.name

    def __init__(self, prefix: str = None):
        self.prefix = prefix
        self.defines: List[BaseConfig.Define] = []

    def create_define(self, name: str, value=None, add_quotes=False) -> Define:
        n = f'{self.prefix}{name}'

        # Make booleans lowercase
        if isinstance(value, bool):
            value = str(value).lower()

        if add_quotes:
            v = f'"{value}"'
        else:
            v = value

        define = BaseConfig.Define(n, v)
        self.defines.append(define)
        return define

class ProtonConfig(BaseConfig):
    # Top level keys
    NODES = "nodes"
    CONNECTIONS = "connections"
    BUNDLES = "bundles"

    # Prefixes
    PROTON_PREFIX = "PROTON__"

    class Signal(BaseConfig):
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


    class Bundle(BaseConfig):
        # Bundle keys
        NAME = "name"
        ID = "id"
        PRODUCERS = "producers"
        CONSUMERS = "consumers"
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
            self.producer = bundle[self.PRODUCERS]
            self.consumer = bundle[self.CONSUMERS]
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

    class Connection(BaseConfig):
        # Connection keys
        FIRST = "first"
        SECOND = "second"

        class Endpoint:
            NODE = "node"
            ID = "id"
            def __init__(self, endpoint: dict):
                self.node = endpoint[self.NODE]

                try:
                    self.id = endpoint[self.ID]
                except KeyError:
                    self.id = 0

        def __init__(self, connection: dict):
            self.first = self.Endpoint(connection[self.FIRST])
            self.second = self.Endpoint(connection[self.SECOND])


    class Node(BaseConfig):
        # Node keys
        NAME = "name"
        ENDPOINTS = "endpoints"
        ID = "id"
        TYPE = "type"
        IP = "ip"
        PORT = "port"
        DEVICE = "device"
        HEARTBEAT = "heartbeat"

        NODE_PREFIX = "NODE__"
        ID_SUFFIX = "ID"
        NAME_SUFFIX = "NAME"
        IP_SUFFIX = "__IP"
        PORT_SUFFIX = "__PORT"
        DEVICE_SUFFIX = "__DEVICE"
        DEFAULT_VALUE_SUFFIX = "DEFAULT_VALUE"
        PEERS_SUFFIX = "__PEERS"
        PEER_SUFFIX = "PEER"
        PEER_PREFIX = "PEER__"
        INIT_SUFFIX = "Init"
        RECEIVE_SUFFIX = "Receive"

        TRANSPORT_PREFIX = "PROTON_TRANSPORT__"
        TRANSPORT_CONNECT = "Connect"
        TRANSPORT_DISCONNECT = "Disconnect"
        TRANSPORT_READ = "Read"
        TRANSPORT_Write = "Write"

        HEARTBEAT_SUFFIX = "__HEARTBEAT"

        MUTEX_PREFIX = "PROTON_MUTEX__"
        MUTEX_LOCK = "Lock"
        MUTEX_UNLOCK = "Unlock"

        class Heartbeat(BaseConfig):
            # Heartbeat keys
            ENABLED = "enabled"
            PERIOD = "period"

            # Prefixes
            HEARTBEAT_PREFIX = "HEARTBEAT__"

            # Suffixes
            ENABLED_SUFFIX = "ENABLED"
            PERIOD_SUFFIX = "PERIOD"
            DEFAULT_VALUE_SUFFIX = "DEFAULT_VALUE"

            def __init__(self, heartbeat: dict, prefix=None):
                super().__init__(f'{prefix}{self.HEARTBEAT_PREFIX}')

                self.enabled = False
                self.period = 1000

                try:
                    if not isinstance(heartbeat[self.ENABLED], bool):
                        raise RuntimeError(f"Invalid heartbeat enabled {heartbeat[self.ENABLED]}")
                    self.enabled = heartbeat[self.ENABLED]
                except KeyError:
                    pass

                try:
                    if not isinstance(heartbeat[self.PERIOD], int) or heartbeat[self.PERIOD] <= 0:
                        raise RuntimeError(f"Invalid heartbeat period {heartbeat[self.PERIOD]}")
                    self.period = heartbeat[self.PERIOD]
                except KeyError:
                    pass

                self.enabled_define = self.create_define(self.ENABLED_SUFFIX, self.enabled)
                self.period_define = self.create_define(self.PERIOD_SUFFIX, self.period)
                self.default_value_define = self.create_define(self.DEFAULT_VALUE_SUFFIX, f'{{{self.enabled_define}, {self.period_define}}}')

        class Endpoint(BaseConfig):
            # Endpoint keys
            ID = "id"
            TYPE = "type"
            IP = "ip"
            PORT = "port"
            DEVICE = "device"

            # Transport types
            UDP4 = "udp4"
            SERIAL = "serial"

            # Prefixes
            ENDPOINT_PREFIX = "ENDPOINT__"

            # Suffixes
            ID_SUFFIX = "__ID"
            TYPE_SUFFIX = "__TYPE"
            IP_SUFFIX = "__IP"
            IPHL_SUFFIX = "__IPHL"
            IPNL_SUFFIX = "__IPNL"
            PORT_SUFFIX = "__PORT"
            DEVICE_SUFFIX = "__DEVICE"

            def __init__(self, endpoint: dict, prefix=None):
                try:
                    self.id = endpoint[self.ID]
                except KeyError:
                    self.id = 0

                super().__init__(f'{prefix}{self.ENDPOINT_PREFIX}{self.id}')

                self.type = endpoint[self.TYPE]

                self.id_define = self.create_define(self.ID_SUFFIX, self.id)
                self.type_define = self.create_define(self.TYPE_SUFFIX, self.type, add_quotes=True)

                if self.type == self.UDP4:
                    # Define IP
                    self.ip = endpoint[self.IP]
                    self.ip_define = self.create_define(self.IP_SUFFIX, self.ip, add_quotes=True)
                    # Convert string IP to hostlong and netlong formats
                    ip_split = self.ip.split(".")
                    self.ip_hl = 0
                    self.ip_nl = 0
                    for i in range(0, 4):
                        self.ip_hl |= int(ip_split[i]) << 8 * (3 - i)
                        self.ip_nl |= int(ip_split[i]) << 8 * i

                    self.ip_hl_define = self.create_define(self.IPHL_SUFFIX, hex(self.ip_hl))
                    self.ip_nl_define = self.create_define(self.IPNL_SUFFIX, hex(self.ip_nl))

                    # Define port
                    self.port = endpoint[self.PORT]
                    self.port_define = self.create_define(self.PORT_SUFFIX, self.port)
                elif self.type == self.SERIAL:
                    self.device = endpoint[self.DEVICE]
                    self.device_define = self.create_define(f'{self.DEVICE_SUFFIX}', self.device)
                else:
                    raise RuntimeError(f"Invalid endpoint type {self.type}")

        def __init__(self, node: dict, id: int, prefix=None):
            # Node name
            self.name = node[self.NAME]
            super().__init__(f'{prefix}{self.NODE_PREFIX}{self.name.upper()}__')

            self.id = id

            # Add each endpoint
            self.endpoints: List[ProtonConfig.Node.Endpoint] = []
            for endpoint in node[self.ENDPOINTS]:
                self.endpoints.append(ProtonConfig.Node.Endpoint(endpoint, prefix=self.prefix))

            # Set heartbeat configuration
            try:
                self.heartbeat = self.Heartbeat(node[self.HEARTBEAT], prefix=self.prefix)
            except KeyError:
                self.heartbeat = self.Heartbeat({}, prefix=self.prefix)

            self.name_define = self.create_define(self.NAME_SUFFIX, self.name, add_quotes=True)
            self.id_define = self.create_define(self.ID_SUFFIX, self.id)
            self.peer_define = self.create_define(self.PEER_SUFFIX)
            self.node_default_value_define = self.create_define(self.DEFAULT_VALUE_SUFFIX, f'proton_node_default({self.name_define})')
            self.peer_default_value_define = self.create_define(f'{self.PEER_SUFFIX}__{self.DEFAULT_VALUE_SUFFIX}', f'proton_peer_default({self.name_define})')

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

            # self.heartbeat_enabled = False
            # self.heartbeat_period = 1000
            # self.heartbeat_enabled_define = f'{self.NODE_PREFIX}{self.name.upper()}{self.HEARTBEAT_ENABLED_SUFFIX}'
            # self.heartbeat_period_define = f'{self.NODE_PREFIX}{self.name.upper()}{self.HEARTBEAT_PERIOD_SUFFIX}'
            # self.heartbeat_callback_function_name = f'{self.HEARTBEAT_PREFIX}{self.name.title()}Callback'

            # match self.type:
            #     case self.UDP4:
            #         self.ip = transport[self.IP]
            #         self.port = transport[self.PORT]
            #         self.ip_define = f'{self.NODE_PREFIX}{self.name.upper()}{self.IP_SUFFIX}'
            #         self.port_define = f'{self.NODE_PREFIX}{self.name.upper()}{self.PORT_SUFFIX}'
            #     case self.SERIAL:
            #         self.device = transport[self.DEVICE]
            #         self.device_define = f'{self.NODE_PREFIX}{self.name.upper()}{self.DEVICE_SUFFIX}'

    def __init__(self, dictionary: dict):
        self.dictionary = dictionary
        self.nodes: Dict[str, ProtonConfig.Node] = {}
        self.connections: List[ProtonConfig.Connection] = []
        self.bundles: List[ProtonConfig.Bundle] = []
        self.heartbeats: List[ProtonConfig.Bundle] = []
        self.parse_nodes()
        self.parse_connections()
        self.parse_bundles()
        self.parse_heartbeats()

    def parse_nodes(self):
        nodes = self.dictionary[self.NODES]
        for i, n in enumerate(nodes):
            node = ProtonConfig.Node(n, hex(1 << i), prefix=self.PROTON_PREFIX)
            self.nodes[node.name] = node

    def parse_connections(self):
        connections = self.dictionary[self.CONNECTIONS]
        for connection in connections:
            self.connections.append(ProtonConfig.Connection(connection))

    def parse_bundles(self):
        bundles = self.dictionary[self.BUNDLES]
        for bundle in bundles:
            self.bundles.append(
                ProtonConfig.Bundle(bundle)
            )

    def parse_heartbeats(self):
        pass
        # for [name, node] in self.nodes:
        #     if node.heartbeat_enabled:
        #         heartbeat_bundle_dict: dict = {}
        #         heartbeat_bundle_dict[ProtonConfig.Bundle.NAME] = f'{name}{ProtonConfig.Bundle.HEARTBEAT_STRUCT_SUFFIX}'
        #         heartbeat_bundle_dict[ProtonConfig.Bundle.ID] = 0
        #         heartbeat_bundle_dict[ProtonConfig.Bundle.PRODUCER] = name
        #         heartbeat_bundle_dict[ProtonConfig.Bundle.CONSUMER] = node.peers[0][ProtonConfig.Node.NAME]

        #         heartbeat_signal_dict: dict = {}
        #         heartbeat_signal_dict[ProtonConfig.Signal.NAME] = 'heartbeat'
        #         heartbeat_signal_dict[ProtonConfig.Signal.TYPE] = ProtonConfig.Signal.SignalTypes.UINT32

        #         heartbeat_bundle_dict[ProtonConfig.Bundle.SIGNALS] = [heartbeat_signal_dict]
        #         self.heartbeats.append(ProtonConfig.Bundle(heartbeat_bundle_dict))
