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
        SIGNAL = "signal"
        TYPE = "type"
        LENGTH = "length"
        CAPACITY = "capacity"

        SIGNAL_ENUM_PREFIX = "PROTON_SIGNALS__"

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

        def __init__(self, bundle: str, signal: dict):
            self.bundle: str = bundle
            self.signal: str = signal[self.SIGNAL]
            self.type: ProtonConfig.Signal.SignalTypes = (
                ProtonConfig.Signal.SignalTypes(signal[self.TYPE])
            )
            self.length: int = 0
            self.capacity: int = 0

            try:
                self.length = signal[self.LENGTH]
            except KeyError:
                pass

            try:
                self.capacity = signal[self.CAPACITY]
            except KeyError:
                pass

            self.signal_enum_name = f'{self.SIGNAL_ENUM_PREFIX}{self.bundle.upper()}__{self.signal.upper()}'

            print(f'signal {self.signal} type {self.type} length {self.length} cap {self.capacity}')

            match self.type:
                case ProtonConfig.Signal.SignalTypes.BYTES:
                    assert self.capacity > 0, (
                        f"{self.type} type signals must have a non-zero capacity"
                    )
                case ProtonConfig.Signal.SignalTypes.STRING:
                    assert self.capacity > 0, (
                        f"{self.type} type signals must have a non-zero capacity"
                    )
                    # List of strings
                    if self.length > 0:
                        self.type = ProtonConfig.Signal.SignalTypes(f"list_{self.type}")
                case (
                    ProtonConfig.Signal.SignalTypes.DOUBLE
                    | ProtonConfig.Signal.SignalTypes.FLOAT
                    | ProtonConfig.Signal.SignalTypes.INT32
                    | ProtonConfig.Signal.SignalTypes.INT64
                    | ProtonConfig.Signal.SignalTypes.UINT32
                    | ProtonConfig.Signal.SignalTypes.UINT64
                    | ProtonConfig.Signal.SignalTypes.BOOL
                ):
                    # List of type
                    if self.length > 0:
                        self.type = ProtonConfig.Signal.SignalTypes(f"list_{self.type}")

    class Bundle:
        # Bundle keys
        NAME = "name"
        ID = "id"
        PRODUCER = "producer"
        CONSUMER = "consumer"
        SCHEMA = "schema"
        BUNDLE_SUFFIX = "_bundle"
        BUNDLE_STRUCT_PREFIX = "PROTON_BUNDLE__"
        BUNDLE_SIGNAL_ENUM_PREFIX = "PROTON_SIGNALS__"
        SIGNALS_SUFFIX = "_signals"
        INIT_FUNCTION_SUFFIX = "PROTON_BUNDLE_init_"

        # Bundle types
        class MessageTypes(StrEnum):
            PERIODIC = auto()
            EVENT = auto()
            TRIGGER = auto()

        def __init__(self, bundle: dict, type: MessageTypes):
            self.type = type
            self.name = bundle[self.NAME]
            self.id = bundle[self.ID]
            self.producer = bundle[self.PRODUCER]
            self.consumer = bundle[self.CONSUMER]
            self.signals: List[ProtonConfig.Signal] = []
            self.needs_init = False

            self.bundle_variable_name = f'{self.name}{self.BUNDLE_SUFFIX}'
            self.signals_variable_name = f'{self.name}{self.SIGNALS_SUFFIX}'
            self.struct_name = f'{self.BUNDLE_STRUCT_PREFIX}{self.name}'
            self.struct_variable_name = f'{self.name}_struct'
            self.signals_enum_name = f'{self.BUNDLE_SIGNAL_ENUM_PREFIX}{self.name}'
            self.signals_enum_count = f'{self.signals_enum_name.upper()}_COUNT'
            self.init_function_name = f'{self.INIT_FUNCTION_SUFFIX}{self.name}'

            for signal in bundle[self.SCHEMA]:
                s = ProtonConfig.Signal(self.name, signal)
                self.signals.append(s)
                if s.type == ProtonConfig.Signal.SignalTypes.LIST_STRING:
                    self.needs_init = True

    class Node:
        # Node keys
        NAME = "name"
        TRANSPORT = "transport"
        TYPE = "type"
        IP = "ip"
        PORT = "port"
        DEVICE = "device"

        # Transport types
        UDP4 = "udp4"
        SERIAL = "serial"

        def __init__(self, node: dict):
            self.name = node[self.NAME]
            transport = node[self.TRANSPORT]
            self.type = transport[self.TYPE]
            match self.type:
                case self.UDP4:
                    self.ip = transport[self.IP]
                    self.port = transport[self.PORT]
                case self.SERIAL:
                    self.device = transport[self.DEVICE]

    def __init__(self, dictionary: dict):
        self.dictionary = dictionary
        self.nodes: List[ProtonConfig.Node] = []
        self.bundles: List[ProtonConfig.Bundle] = []
        self.parse_nodes()
        self.parse_messages()

    def parse_nodes(self):
        try:
            nodes = self.dictionary[self.NODES]
        except KeyError:
            print("Nodes key missing")
            return
        for node in nodes:
            self.nodes.append(ProtonConfig.Node(node))

        for n in self.nodes:
            print(f"Node: {n.name} Transport: {n.type}")

    def parse_messages(self):
        try:
            bundles = self.dictionary[self.BUNDLES]
        except KeyError:
            print("Bundles key missing")
            return
        for t in bundles:
            for bundle in bundles[t]:
                self.bundles.append(
                    ProtonConfig.Bundle(bundle, ProtonConfig.Bundle.MessageTypes(t))
                )

        for m in self.bundles:
            print(
                f"Bundle: {m.name} type: {m.type} id: {m.id}, producer {m.producer}, consumer {m.consumer}"
            )
            print("Signals:")
            for s in m.signals:
                print(f"\t{s.signal}: type: {s.type}, length: {s.length}")
