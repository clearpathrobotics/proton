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
    MESSAGES = "messages"

    class Signal:
        # Signal keys
        SIGNAL = "signal"
        TYPE = "type"
        LENGTH = "length"
        CAPACITY = "capacity"

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

        def __init__(self, signal: dict):
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

    class Message:
        # Message keys
        NAME = "name"
        ID = "id"
        PRODUCER = "producer"
        CONSUMER = "consumer"
        SCHEMA = "schema"

        # Message types
        class MessageTypes(StrEnum):
            PERIODIC = auto()
            EVENT = auto()
            TRIGGER = auto()

        def __init__(self, message: dict, type: MessageTypes):
            self.type = type
            self.name = message[self.NAME]
            self.id = message[self.ID]
            self.producer = message[self.PRODUCER]
            self.consumer = message[self.CONSUMER]
            self.signals: List[ProtonConfig.Signal] = []
            self.needs_init = False

            for signal in message[self.SCHEMA]:
                s = ProtonConfig.Signal(signal)
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
        self.messages: List[ProtonConfig.Message] = []
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
            messages = self.dictionary[self.MESSAGES]
        except KeyError:
            print("Messages key missing")
            return
        for t in messages:
            for message in messages[t]:
                self.messages.append(
                    ProtonConfig.Message(message, ProtonConfig.Message.MessageTypes(t))
                )

        for m in self.messages:
            print(
                f"Message: {m.name} type: {m.type} id: {m.id}, producer {m.producer}, consumer {m.consumer}"
            )
            print("Signals:")
            for s in m.signals:
                print(f"\t{s.signal}: type: {s.type}, length: {s.length}")
