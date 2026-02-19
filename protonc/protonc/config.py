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
# @author Roni Kreinin (roni.kreinin@rockwellautomation.com)

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

    class Enum:
        def __init__(self, name: str, value: int=None):
            self.name = name
            self.value = value

        def __str__(self):
            return self.name

    class Struct:
        def __init__(self, name: str):
            self.struct = name.lower()
            self.typedef = f'{name}_t'.lower()

        def __str__(self):
            return self.struct

    class Function:
        def __init__(self, name: str):
            self.name = name

        def __str__(self):
            return self.name

    def __init__(self, prefix: List[str] = None):
        self.prefix = prefix
        self.defines: List[BaseConfig.Define] = []
        self.enum_entries: List[BaseConfig.Enum] = []
        self.structs: List[BaseConfig.Struct] = []
        self.functions: List[BaseConfig.Function] = []


    def create_define(self, name: str, value=None, add_quotes=False, prefix: List[str]=None) -> Define:
        n = ''
        if prefix:
            n = f'{prefix}__{name}'
        else:
            for p in self.prefix:
                n += f'{p}__'
            n += name

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

    def create_enum_entry(self, name: str=None, value: int=None, prefix: List[str]=None) -> Enum:
        n = ''
        if prefix:
            n = prefix
        else:
            for i, p in enumerate(self.prefix):
                if i < len(self.prefix) - 1:
                    n += f'{p}__'
                else:
                    n += p
        if name:
            n += f'__{name}'

        if value:
            enum = BaseConfig.Enum(n, hex(value))
        else:
            enum = BaseConfig.Enum(n)
        self.enum_entries.append(enum)
        return enum

    def create_struct(self, name: str=None, prefix: List[str]=None) -> Struct:
        n = ''
        if prefix:
            n = prefix
        else:
            for i, p in enumerate(self.prefix):
                if i < len(self.prefix) - 1:
                    n += f'{p}_'
                else:
                    n += p
        if name:
            n += f'_{name}'

        struct = BaseConfig.Struct(n)
        self.structs.append(struct)
        return struct

    def create_function(self, name: str, prefix: List[str]=None) -> Function:
        n = ''
        if prefix:
            for p in prefix:
                n += f'{p}_'
            n += name
        else:
            for p in self.prefix:
                n += f'{p}_'
            n += name
        n = n.lower()
        func = BaseConfig.Function(n)
        self.functions.append(func)
        return func


class ProtonConfig(BaseConfig):
    # Top level keys
    NODES = "nodes"
    CONNECTIONS = "connections"
    BUNDLES = "bundles"

    # Prefixes
    PROTON_PREFIX = "PROTON"

    # Enums
    PROTON_BUNDLE_ENUM = "PROTON__BUNDLE_e"

    class Signal(BaseConfig):
        # Signal keys
        NAME = "name"
        TYPE = "type"
        LENGTH = "length"
        CAPACITY = "capacity"
        VALUE = "value"

        SIGNAL_PREFIX = "SIGNAL"
        LENGTH_SUFFIX = "LENGTH"
        CAPACITY_SUFFIX = "CAPACITY"
        VALUE_SUFFIX = "DEFAULT_VALUE"

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

        def __init__(self, bundle: str, signal: dict, prefix: List[str]=None):
            self.bundle: str = bundle
            self.name: str = signal[self.NAME]
            super().__init__(prefix + [self.SIGNAL_PREFIX, self.name.upper()])

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

            self.is_const = self.value is not None

            if not self.is_const:
                self.value = self.DEFAULT_VALUES[self.type]

            self.signal_enum_name = self.create_enum_entry()
            self.capacity_define = self.create_define(self.CAPACITY_SUFFIX, self.capacity)
            self.length_define = self.create_define(self.LENGTH_SUFFIX, self.length)

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
                            list_def += f"{'true' if v else 'false'}, "
                        else:
                            list_def += f"{'true' if v else 'false'}"
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

            self.value_define = self.create_define(self.VALUE_SUFFIX, self.c_value)


    class Bundle(BaseConfig):
        # Bundle keys
        NAME = "name"
        ID = "id"
        PRODUCERS = "producers"
        CONSUMERS = "consumers"
        SIGNALS = "signals"
        BUNDLE_SUFFIX = "_bundle"
        BUNDLE_HANDLE_SUFFIX = "_bundle_handle"
        HEARTBEAT_STRUCT_SUFFIX = "_heartbeat"
        BUNDLE_PREFIX = "BUNDLE"
        BUNDLE_STRUCT_PREFIX = "PROTON_BUNDLE"
        BUNDLE_SIGNAL_ENUM_PREFIX = "PROTON_SIGNALS"
        BUNDLE_ID_PREFIX = "PROTON_BUNDLE_ID"
        SIGNAL_HANDLES_SUFFIX = "_signal_handles"
        INIT_FUNCTION_SUFFIX = "Init"
        CALLBACK_PREFIX = "PROTON_BUNDLE_"
        CALLBACK_SUFFIX = "Callback"
        DEFAULT_VALUE_SUFFIX = "DEFAULT_VALUE"
        PRODUCERS_SUFFIX = "PRODUCERS"
        CONSUMERS_SUFFIX = "CONSUMERS"
        HEARTBEAT_BUNDLE_ID = "PROTON_HEARTBEAT_ID"
        SIGNAL_SUFFIX = "SIGNAL"
        STRUCT_SUFFIX = "STRUCT"

        HEARTBEAT_STRUCT = "proton_bundle_heartbeat_t"
        HEARTBEAT_DEFAULT_VALUE = "PROTON_HEARTBEAT_BUNDLE_DEFAULT_VALUE"

        def __init__(self, bundle: dict, prefix: List[str]=None):
            self.name = bundle[self.NAME]
            super().__init__(prefix + [self.BUNDLE_PREFIX, self.name.upper()])
            self.id = bundle[self.ID]
            self.producers: List[str] = []
            self.consumers: List[str] = []

            if isinstance(bundle[self.PRODUCERS], str):
                self.producers.append(bundle[self.PRODUCERS])
            elif isinstance(bundle[self.PRODUCERS], List):
                self.producers = bundle[self.PRODUCERS]
            else:
                raise RuntimeError(f"Invalid Producers format: {bundle[self.PRODUCERS]}")

            if isinstance(bundle[self.CONSUMERS], str):
                self.consumers.append(bundle[self.CONSUMERS])
            elif isinstance(bundle[self.CONSUMERS], List):
                self.consumers = bundle[self.CONSUMERS]
            else:
                raise RuntimeError(f"Invalid Consumers format: {bundle[self.CONSUMERS]}")

            self.signals: List[ProtonConfig.Signal] = []
            self.needs_init = False
            self.has_signals = False

            self.bundle_enum_name = self.create_enum_entry()
            self.internal_handle_variable_name = f'_{self.name}{self.BUNDLE_HANDLE_SUFFIX}'
            self.signal_handles_variable_name = f'_{self.name}{self.SIGNAL_HANDLES_SUFFIX}'
            self.struct = self.create_struct()
            self.bundle_variable_name = f'{self.name}{self.BUNDLE_SUFFIX}'
            self.signals_enum_name = self.create_enum_entry(self.SIGNAL_SUFFIX)
            self.signals_enum_count = f'{self.signals_enum_name.name}__COUNT'
            self.init_function_name = self.create_function(self.INIT_FUNCTION_SUFFIX)
            self.callback_function_name = self.create_function(self.CALLBACK_SUFFIX)

            try:
                for signal in bundle[self.SIGNALS]:
                    s = ProtonConfig.Signal(self.name, signal, prefix=self.prefix)
                    self.signals.append(s)
                    if s.type == ProtonConfig.Signal.SignalTypes.LIST_STRING:
                        self.needs_init = True
            except KeyError:
                pass

            if len(self.signals) > 0:
                self.has_signals = True

            self.default_value = "{"
            for i, s in enumerate(self.signals):
                if i < len(self.signals) - 1:
                    self.default_value += f'{s.value_define.name}, '
                else:
                    self.default_value += f'{s.value_define.name}'
            self.default_value += "}"
            self.default_value_define = self.create_define(f'{self.DEFAULT_VALUE_SUFFIX}', self.default_value)

            self.producers_define = self.create_define(self.PRODUCERS_SUFFIX, self.producers)
            self.consumers_define = self.create_define(self.CONSUMERS_SUFFIX, self.consumers)

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

        NODE_PREFIX = "NODE"
        ID_SUFFIX = "ID"
        NAME_SUFFIX = "NAME"
        IP_SUFFIX = "IP"
        PORT_SUFFIX = "PORT"
        DEVICE_SUFFIX = "DEVICE"
        DEFAULT_VALUE_SUFFIX = "DEFAULT_VALUE"
        PEERS_SUFFIX = "PEERS"
        PEER_SUFFIX = "PEER"
        PEER_PREFIX = "PEER"
        INIT_SUFFIX = "Init"
        RECEIVE_SUFFIX = "Receive"

        TRANSPORT_PREFIX = "TRANSPORT"
        TRANSPORT_CONNECT = "Connect"
        TRANSPORT_DISCONNECT = "Disconnect"
        TRANSPORT_READ = "Read"
        TRANSPORT_WRITE = "Write"

        HEARTBEAT_SUFFIX = "HEARTBEAT"

        MUTEX_LOCK = "Lock"
        MUTEX_UNLOCK = "Unlock"

        class Heartbeat(BaseConfig):
            # Heartbeat keys
            ENABLED = "enabled"
            PERIOD = "period"

            # Prefixes
            HEARTBEAT_PREFIX = "HEARTBEAT"

            # Suffixes
            ENABLED_SUFFIX = "ENABLED"
            PERIOD_SUFFIX = "PERIOD"
            DEFAULT_VALUE_SUFFIX = "DEFAULT_VALUE"

            def __init__(self, heartbeat: dict, prefix: List[str]=None):
                super().__init__(prefix + [self.HEARTBEAT_PREFIX])

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
            ENDPOINT_PREFIX = "ENDPOINT"

            # Suffixes
            ID_SUFFIX = "ID"
            TYPE_SUFFIX = "TYPE"
            IP_SUFFIX = "IP"
            IPHL_SUFFIX = "IPHL"
            IPNL_SUFFIX = "IPNL"
            PORT_SUFFIX = "PORT"
            DEVICE_SUFFIX = "DEVICE"

            def __init__(self, endpoint: dict, prefix: List[str] =None):
                try:
                    self.id = endpoint[self.ID]
                except KeyError:
                    self.id = 0
                super().__init__(prefix + [self.ENDPOINT_PREFIX, self.id])

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
                    self.device_define = self.create_define(self.DEVICE_SUFFIX, self.device, add_quotes=True)
                else:
                    raise RuntimeError(f"Invalid endpoint type {self.type}")

        def __init__(self, node: dict, id: int, prefix: List[str]=None):
            # Node name
            self.name = node[self.NAME]
            super().__init__(prefix + [self.NODE_PREFIX, self.name.upper()])

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
            self.peer_define = self.create_define(self.name.upper(), prefix=f'{ProtonConfig.PROTON_PREFIX}__{self.PEER_PREFIX}')
            self.node_default_value_define = self.create_define(self.DEFAULT_VALUE_SUFFIX, f'PROTON_NODE_DEFAULT({self.name_define})')
            self.peer_default_value_define = self.create_define(f'{self.PEER_SUFFIX}__{self.DEFAULT_VALUE_SUFFIX}', f'PROTON_PEER_DEFAULT({self.name_define})')

            self.transport_connect_func = self.create_function(f'{self.TRANSPORT_PREFIX}_{self.TRANSPORT_CONNECT}')
            self.transport_disconnect_func = self.create_function(f'{self.TRANSPORT_PREFIX}_{self.TRANSPORT_DISCONNECT}')
            self.transport_read_func = self.create_function(f'{self.TRANSPORT_PREFIX}_{self.TRANSPORT_READ}')
            self.transport_write_func = self.create_function(f'{self.TRANSPORT_PREFIX}_{self.TRANSPORT_WRITE}')
            self.transport_define = self.create_define(f'{self.TRANSPORT_PREFIX}__{self.DEFAULT_VALUE_SUFFIX}', f'{{PROTON_TRANSPORT_DISCONNECTED, {self.transport_connect_func}, {self.transport_disconnect_func}, {self.transport_read_func}, {self.transport_write_func}}}')

            self.node_variable_name = f'{self.name}_node'
            self.peer_variable_name = f'{self.name}_peers'

            self.mutex_lock_func = self.create_function(self.MUTEX_LOCK)
            self.mutex_unlock_func = self.create_function(self.MUTEX_UNLOCK)
            self.peer_init_func = self.create_function(self.INIT_SUFFIX, prefix=[ProtonConfig.PROTON_PREFIX, ProtonConfig.Node.PEER_PREFIX, self.name])
            self.receive_func = self.create_function(self.RECEIVE_SUFFIX)
            self.buffer_variable_name = f'proton_{self.name.lower()}_buffer'

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
            node = ProtonConfig.Node(n, hex(1 << i), prefix=[self.PROTON_PREFIX])
            self.nodes[node.name] = node

    def parse_connections(self):
        connections = self.dictionary[self.CONNECTIONS]
        for connection in connections:
            self.connections.append(ProtonConfig.Connection(connection))

    def parse_bundles(self):
        bundles = self.dictionary[self.BUNDLES]
        for bundle in bundles:
            self.bundles.append(
                ProtonConfig.Bundle(bundle, prefix=[self.PROTON_PREFIX])
            )

    def parse_heartbeats(self):
        for node in self.nodes.values():
            if node.heartbeat.enabled:
                heartbeat_bundle_dict: dict = {}
                heartbeat_bundle_dict[ProtonConfig.Bundle.NAME] = f'{node.name}{ProtonConfig.Bundle.HEARTBEAT_STRUCT_SUFFIX}'
                heartbeat_bundle_dict[ProtonConfig.Bundle.ID] = 0
                heartbeat_bundle_dict[ProtonConfig.Bundle.PRODUCERS] = node.name
                heartbeat_bundle_dict[ProtonConfig.Bundle.CONSUMERS] = [n.name for n in self.nodes.values() if n.name != node.name]

                heartbeat_signal_dict: dict = {}
                heartbeat_signal_dict[ProtonConfig.Signal.NAME] = 'heartbeat'
                heartbeat_signal_dict[ProtonConfig.Signal.TYPE] = ProtonConfig.Signal.SignalTypes.UINT32

                heartbeat_bundle_dict[ProtonConfig.Bundle.SIGNALS] = [heartbeat_signal_dict]
                self.heartbeats.append(ProtonConfig.Bundle(heartbeat_bundle_dict, prefix=[ProtonConfig.PROTON_PREFIX]))
