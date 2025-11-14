#!/usr/bin/env python3

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

import argparse
import os
import yaml

from protonc.config import ProtonConfig
from protonc.source_writer import CWriter, Variable, Struct, Function


class ProtonCGenerator:
    SIGNAL_TYPE_MAP = {
        ProtonConfig.Signal.SignalTypes.DOUBLE: "double",
        ProtonConfig.Signal.SignalTypes.FLOAT: "float",
        ProtonConfig.Signal.SignalTypes.INT32: "int32_t",
        ProtonConfig.Signal.SignalTypes.INT64: "int64_t",
        ProtonConfig.Signal.SignalTypes.UINT32: "uint32_t",
        ProtonConfig.Signal.SignalTypes.UINT64: "uint64_t",
        ProtonConfig.Signal.SignalTypes.BOOL: "bool",
        ProtonConfig.Signal.SignalTypes.STRING: "char",
        ProtonConfig.Signal.SignalTypes.BYTES: "uint8_t",
        ProtonConfig.Signal.SignalTypes.LIST_DOUBLE: "double",
        ProtonConfig.Signal.SignalTypes.LIST_FLOAT: "float",
        ProtonConfig.Signal.SignalTypes.LIST_INT32: "int32_t",
        ProtonConfig.Signal.SignalTypes.LIST_INT64: "int64_t",
        ProtonConfig.Signal.SignalTypes.LIST_UINT32: "uint32_t",
        ProtonConfig.Signal.SignalTypes.LIST_UINT64: "uint64_t",
        ProtonConfig.Signal.SignalTypes.LIST_BOOL: "bool",
        ProtonConfig.Signal.SignalTypes.LIST_STRING: "char",
        ProtonConfig.Signal.SignalTypes.LIST_BYTES: "uint8_t",
    }

    SIGNAL_TAG_MAP = {
        ProtonConfig.Signal.SignalTypes.DOUBLE: "proton_Signal_double_value_tag",
        ProtonConfig.Signal.SignalTypes.FLOAT: "proton_Signal_float_value_tag",
        ProtonConfig.Signal.SignalTypes.INT32: "proton_Signal_int32_value_tag",
        ProtonConfig.Signal.SignalTypes.INT64: "proton_Signal_int64_value_tag",
        ProtonConfig.Signal.SignalTypes.UINT32: "proton_Signal_uint32_value_tag",
        ProtonConfig.Signal.SignalTypes.UINT64: "proton_Signal_uint64_value_tag",
        ProtonConfig.Signal.SignalTypes.BOOL: "proton_Signal_bool_value_tag",
        ProtonConfig.Signal.SignalTypes.STRING: "proton_Signal_string_value_tag",
        ProtonConfig.Signal.SignalTypes.BYTES: "proton_Signal_bytes_value_tag",
        ProtonConfig.Signal.SignalTypes.LIST_DOUBLE: "proton_Signal_list_double_value_tag",
        ProtonConfig.Signal.SignalTypes.LIST_FLOAT: "proton_Signal_list_float_value_tag",
        ProtonConfig.Signal.SignalTypes.LIST_INT32: "proton_Signal_list_int32_value_tag",
        ProtonConfig.Signal.SignalTypes.LIST_INT64: "proton_Signal_list_int64_value_tag",
        ProtonConfig.Signal.SignalTypes.LIST_UINT32: "proton_Signal_list_uint32_value_tag",
        ProtonConfig.Signal.SignalTypes.LIST_UINT64: "proton_Signal_list_uint64_value_tag",
        ProtonConfig.Signal.SignalTypes.LIST_BOOL: "proton_Signal_list_bool_value_tag",
        ProtonConfig.Signal.SignalTypes.LIST_STRING: "proton_Signal_list_string_value_tag",
        ProtonConfig.Signal.SignalTypes.LIST_BYTES: "proton_Signal_list_bytes_value_tag",
    }

    SIGNAL_VARIABLE_MAP = {
        ProtonConfig.Signal.SignalTypes.LIST_DOUBLE: "doubles",
        ProtonConfig.Signal.SignalTypes.LIST_FLOAT: "floats",
        ProtonConfig.Signal.SignalTypes.LIST_INT32: "int32s",
        ProtonConfig.Signal.SignalTypes.LIST_INT64: "int64s",
        ProtonConfig.Signal.SignalTypes.LIST_UINT32: "uint32s",
        ProtonConfig.Signal.SignalTypes.LIST_UINT64: "uint64s",
        ProtonConfig.Signal.SignalTypes.LIST_BOOL: "bools",
        ProtonConfig.Signal.SignalTypes.LIST_STRING: "strings",
        ProtonConfig.Signal.SignalTypes.LIST_BYTES: "bytes",
    }

    def __init__(self, config_file: str, destination_path: str):
        self.config_file = config_file
        self.destination_path = destination_path
        assert os.path.exists(self.config_file)
        if not os.path.exists(self.destination_path):
            os.mkdir(self.destination_path)

        self.dictionary = self.read_yaml()
        self.config = ProtonConfig(self.dictionary)

    def read_yaml(self):
        # Check YAML can be Opened
        try:
            config = yaml.load(open(self.config_file), Loader=yaml.SafeLoader)
        except yaml.scanner.ScannerError:
            raise AssertionError(f'YAML file "{self.config_file}" is not well formed')
        except yaml.constructor.ConstructorError:
            raise AssertionError(
                f'YAML file "{self.config_file}" is attempting to create unsafe objects'
            )
        # Check contents are a Dictionary
        assert isinstance(config, dict), (
            f'YAML file "{self.config_file}" is not a dictionary'
        )
        return config

    def generate_bundle_struct_typedefs(self):
        self.header_writer.write_comment("Bundle Structure Definitions", indent_level=0)
        self.header_writer.write_newline()
        for b in self.config.bundles + self.config.heartbeats:
            vars = []
            for s in b.signals:
                vars.append(
                    Variable(
                        name=s.name,
                        type=self.SIGNAL_TYPE_MAP[s.type],
                        length=0 if s.length == 0 else s.length_define,
                        capacity=0 if s.capacity == 0 else s.capacity_define,
                        const=s.is_const
                    )
                )
            s = Struct(b.struct_name, vars)
            self.header_writer.write_typedef_struct(s, indent_level=0)
            self.header_writer.write_newline()

    def generate_extern_bundle_structs(self):
        self.header_writer.write_comment("External Bundle Structures", indent_level=0)
        self.header_writer.write_newline()
        for b in self.config.bundles + self.config.heartbeats:
            self.header_writer.write_extern_variable(
                Variable(b.bundle_variable_name, f"{b.struct_name}_t")
            )
        self.header_writer.write_newline()

    def generate_node(self):
        node = Variable(self.config.target_node.node_variable_name, "proton_node_t", init=self.config.target_node.default_value_define)

        self.header_writer.write_comment("External Node", indent_level=0)
        self.header_writer.write_newline()
        self.header_writer.write_extern_variable(node)
        self.header_writer.write_newline()

        self.src_writer.write_comment("Node", indent_level=0)
        self.src_writer.write_newline()
        self.src_writer.write_variable(node)
        self.src_writer.write_newline()

    def generate_peers(self):
        peers = Variable(self.config.target_node.peer_variable_name,
                         "static proton_peer_t",
                         capacity=len(self.config.target_node.peers),
                         init=self.config.target_node.peers_value_define)

        self.src_writer.write_comment("Peers", indent_level=0)
        self.src_writer.write_newline()
        self.src_writer.write_variable(peers)
        self.src_writer.write_newline()

    def generate_signal_enums(self):
        self.header_writer.write_comment("Signal Enums", indent_level=0)
        self.header_writer.write_newline()
        for b in self.config.bundles + self.config.heartbeats:
            e = [s.name for s in b.signals if not s.is_const]
            self.header_writer.write_enum(b.signals_enum_name, e)
            self.header_writer.write_newline()
        self.header_writer.write_newline()

    def generate_defines(self):
        self.header_writer.write_comment("Constant definitions", indent_level=0)
        self.header_writer.write_newline()

        for n in self.config.nodes:
            self.header_writer.write_define(f'{n.heartbeat_value_define} {{{str(n.heartbeat_enabled).lower()}, {n.heartbeat_period}}}')
            if n.name == self.target:
                self.header_writer.write_define(f'{n.default_value_define} proton_node_default({n.name_define})')
                peer_defaults = '{'
                count = 1
                for n in self.config.nodes:
                    if n.name == self.target:
                        continue
                    if count == len(self.config.nodes) - 1:
                        peer_defaults += f'{n.default_value_define}'
                    else:
                        peer_defaults += f'{n.default_value_define}, '
                    count += 1
                peer_defaults += '}'
                self.header_writer.write_define(f'{n.peers_value_define} {peer_defaults}')
            else:
                self.header_writer.write_define(f'{n.transport_value_define} {{PROTON_TRANSPORT_DISCONNECTED, {n.transport_connect_func}, {n.transport_disconnect_func}, {n.transport_read_func}, {n.transport_write_func}}}')
                self.header_writer.write_define(f'{n.default_value_define} proton_peer_default({n.name_define})')
            self.header_writer.write_newline()

        for b in self.config.bundles + self.config.heartbeats:
            default_value = "{"
            for i, s in enumerate(b.signals):
                if s.length > 0:
                    self.header_writer.write_define(f"{s.length_define} {s.length}")
                if s.capacity > 0:
                    self.header_writer.write_define(f"{s.capacity_define} {s.capacity}")
                self.header_writer.write_define(f"{s.value_define} {s.c_value}")
                if i < len(b.signals) - 1:
                  default_value += f"{s.value_define}, "
                else:
                  default_value += f"{s.value_define}"
            default_value += "}"
            self.header_writer.write_define(f"{b.default_value_define} {default_value}")
            self.header_writer.write_define(f"{b.producers_define} {ProtonConfig.Node.NODE_PREFIX}{b.producer.upper()}")
            self.header_writer.write_define(f"{b.consumers_define} {ProtonConfig.Node.NODE_PREFIX}{b.consumer.upper()}")
            self.header_writer.write_newline()

    def generate_node_ids(self):
        self.header_writer.write_comment("Peers", indent_level=0)
        self.header_writer.write_newline()

        nodes = []
        ids = []
        i = 0
        for n in self.config.nodes:
            if n.name == self.target:
                continue
            nodes.append(n.name.upper())
            n.id = i
            i += 1
            ids.append(n.id)

        nodes.append("COUNT")
        ids.append(i)

        self.header_writer.write_enum(
            "PROTON_PEER",
            nodes,
            ids,
        )
        self.header_writer.write_newline()

    def generate_bundle_ids(self):
        self.header_writer.write_comment("Bundle IDs", indent_level=0)
        self.header_writer.write_newline()
        self.header_writer.write_enum(
            "PROTON_BUNDLE",
            [b.name.upper() for b in self.config.bundles],
            [b.id for b in self.config.bundles],
        )
        self.header_writer.write_newline()

    def generate_signal_variables(self):
        self.src_writer.write_comment("Internal Signals", indent_level=0)
        self.src_writer.write_newline()
        for b in self.config.bundles + self.config.heartbeats:
            signals = Variable(
                b.signal_handles_variable_name,
                "static proton_signal_handle_t",
                length=b.signals_enum_count,
            )
            self.src_writer.write_variable(signals)
        self.src_writer.write_newline()

    def generate_bundle_variable(self):
        self.src_writer.write_comment("Internal Bundles", indent_level=0)
        self.src_writer.write_newline()
        for b in self.config.bundles + self.config.heartbeats:
            proton = Variable(b.internal_handle_variable_name, "static proton_bundle_handle_t")
            self.src_writer.write_variable(proton)
        self.src_writer.write_newline()

    def generate_bundle_structs(self):
        self.src_writer.write_comment("External Bundle Structures", indent_level=0)
        self.src_writer.write_newline()
        for b in self.config.bundles + self.config.heartbeats:
            self.src_writer.write_variable(
                Variable(b.bundle_variable_name, f"{b.struct_name}_t", init=b.default_value_define)
            )
        self.src_writer.write_newline()

    def generate_bundle_init_prototypes(self):
        self.src_writer.write_comment("Bundle Init Prototype", indent_level=0)
        self.src_writer.write_newline()
        for b in self.config.bundles + self.config.heartbeats:
            self.src_writer.write_function_prototype(
                Function(b.init_function_name, [], "proton_status_e")
            )
        self.src_writer.write_newline()

        self.header_writer.write_comment("Bundle Init Prototype", indent_level=0)
        self.header_writer.write_newline()
        self.header_writer.write_function_prototype(
            Function("PROTON_BUNDLE_Init", [], "proton_status_e")
        )
        self.header_writer.write_newline()

    def generate_peer_init_prototypes(self):
        self.src_writer.write_comment("Peer Init Prototype", indent_level=0)
        self.src_writer.write_newline()
        for n in self.config.peers:
            self.src_writer.write_function_prototype(
                Function(n.peer_init_func, [], "proton_status_e")
            )
        self.src_writer.write_newline()

        self.header_writer.write_comment("Peer Init Prototype", indent_level=0)
        self.header_writer.write_newline()
        self.header_writer.write_function_prototype(
            Function("PROTON_PEER_Init", [], "proton_status_e")
        )
        self.header_writer.write_newline()

    def generate_bundle_send_handle_prototype(self):
        self.src_writer.write_comment("Bundle Send Handle Prototype", indent_level=0)
        self.src_writer.write_newline()
        self.src_writer.write_function_prototype(
            Function("PROTON_BUNDLE_SendHandle", [Variable("handle","proton_bundle_handle_t *")], "proton_status_e")
        )
        self.src_writer.write_newline()

    def generate_bundle_init_functions(self):
        self.src_writer.write_comment("Bundle Init Functions", indent_level=0)
        self.src_writer.write_newline()
        for b in self.config.bundles + self.config.heartbeats:
            self.src_writer.write_function_start(
                Function(b.init_function_name, [], "proton_status_e")
            )
            for s in b.signals:
                if s.is_const:
                    continue
                self.src_writer.write(
                    f"{b.signal_handles_variable_name}[{s.signal_enum_name}].signal.which_signal = {self.SIGNAL_TAG_MAP[s.type]};"
                )
                match s.type:
                    case (
                        ProtonConfig.Signal.SignalTypes.DOUBLE
                        | ProtonConfig.Signal.SignalTypes.FLOAT
                        | ProtonConfig.Signal.SignalTypes.INT32
                        | ProtonConfig.Signal.SignalTypes.INT64
                        | ProtonConfig.Signal.SignalTypes.UINT32
                        | ProtonConfig.Signal.SignalTypes.UINT64
                        | ProtonConfig.Signal.SignalTypes.BOOL
                    ):
                        self.src_writer.write(
                            f"{b.signal_handles_variable_name}[{s.signal_enum_name}].arg.data = &{b.bundle_variable_name}.{s.name};"
                        )
                    case (
                        ProtonConfig.Signal.SignalTypes.STRING
                        | ProtonConfig.Signal.SignalTypes.BYTES
                    ):
                        self.src_writer.write(
                            f"{b.signal_handles_variable_name}[{s.signal_enum_name}].signal.signal.{s.type}_value = &{b.signal_handles_variable_name}[{s.signal_enum_name}].arg;"
                        )
                        self.src_writer.write(
                            f"{b.signal_handles_variable_name}[{s.signal_enum_name}].arg.data = {b.bundle_variable_name}.{s.name};"
                        )
                        self.src_writer.write(
                            f"{b.signal_handles_variable_name}[{s.signal_enum_name}].arg.capacity = {s.capacity_define};"
                        )
                        self.src_writer.write(
                            f"{b.signal_handles_variable_name}[{s.signal_enum_name}].arg.size = 0;"
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
                        self.src_writer.write(
                            f"{b.signal_handles_variable_name}[{s.signal_enum_name}].signal.signal.{s.type}_value.{self.SIGNAL_VARIABLE_MAP[s.type]} = &{b.signal_handles_variable_name}[{s.signal_enum_name}].arg;"
                        )
                        self.src_writer.write(
                            f"{b.signal_handles_variable_name}[{s.signal_enum_name}].arg.data = {b.bundle_variable_name}.{s.name};"
                        )
                        self.src_writer.write(
                            f"{b.signal_handles_variable_name}[{s.signal_enum_name}].arg.length = {s.length_define};"
                        )
                        self.src_writer.write(
                            f"{b.signal_handles_variable_name}[{s.signal_enum_name}].arg.size = 0;"
                        )

                    case (ProtonConfig.Signal.SignalTypes.LIST_STRING | ProtonConfig.Signal.SignalTypes.LIST_BYTES):
                        self.src_writer.write(
                            f"{b.signal_handles_variable_name}[{s.signal_enum_name}].signal.signal.{s.type}_value.{self.SIGNAL_VARIABLE_MAP[s.type]} = &{b.signal_handles_variable_name}[{s.signal_enum_name}].arg;"
                        )
                        self.src_writer.write(
                            f"{b.signal_handles_variable_name}[{s.signal_enum_name}].arg.data = {b.bundle_variable_name}.{s.name};"
                        )
                        self.src_writer.write(
                            f"{b.signal_handles_variable_name}[{s.signal_enum_name}].arg.capacity = {s.capacity_define};"
                        )
                        self.src_writer.write(
                            f"{b.signal_handles_variable_name}[{s.signal_enum_name}].arg.length = {s.length_define};"
                        )
                        self.src_writer.write(
                            f"{b.signal_handles_variable_name}[{s.signal_enum_name}].arg.size = 0;"
                        )
                self.src_writer.write_newline()
            self.src_writer.write(
                f"return PROTON_InitBundle(&{b.internal_handle_variable_name}, "
                f"{b.HEARTBEAT_BUNDLE_ID if b.id == 0 else b.bundle_enum_name}, "
                f"{b.signal_handles_variable_name}, {b.signals_enum_count}, "
                f"{b.producers_define}, {b.consumers_define});"
            )
            self.src_writer.write_function_end()

        self.src_writer.write_function_start(Function("PROTON_BUNDLE_Init", [], "proton_status_e"))
        self.src_writer.write("proton_status_e status;")
        self.src_writer.write_newline()
        for b in self.config.bundles + self.config.heartbeats:
            self.src_writer.write(f"status = {b.init_function_name}();", indent_level=1)
            self.src_writer.write_if_statement_start("status != PROTON_OK")
            self.src_writer.write("return status;", indent_level=2)
            self.src_writer.write_if_statement_end()
            self.src_writer.write_newline()
        self.src_writer.write("return PROTON_OK;")
        self.src_writer.write_function_end()

    def generate_peer_init_functions(self):
        self.src_writer.write_comment("Peer Init Functions", indent_level=0)
        self.src_writer.write_newline()

        for n in self.config.peers:
            self.src_writer.write_function_start(Function(n.peer_init_func, [], "proton_status_e"))
            self.src_writer.write(f"return PROTON_InitPeer(&{self.config.target_node.node_variable_name}.peers[PROTON_PEER__{n.name.upper()}], "
                                  f"(proton_heartbeat_t){n.heartbeat_value_define}, (proton_transport_t){n.transport_value_define}, {n.receive_func}, {n.mutex_lock_func}, {n.mutex_unlock_func}, "
                                  f"{n.buffer_variable_name});")
            self.src_writer.write_function_end()

        self.src_writer.write_function_start(Function("PROTON_PEER_Init", [], "proton_status_e"))
        self.src_writer.write("proton_status_e status;")
        self.src_writer.write_newline()

        for n in self.config.peers:
            self.src_writer.write(f"status = {n.peer_init_func}();")
            self.src_writer.write_if_statement_start("status != PROTON_OK")
            self.src_writer.write("return status;", indent_level=2)
            self.src_writer.write_if_statement_end()
            self.src_writer.write_newline()
        self.src_writer.write("return PROTON_OK;")
        self.src_writer.write_function_end()

    def generate_consumer_callbacks(self):
        self.header_writer.write_comment("Consumer callbacks", indent_level=0)
        self.header_writer.write_newline()

        self.src_writer.write_comment("Weak Consumer callbacks", indent_level=0)
        self.src_writer.write_newline()

        for b in self.config.bundles + self.config.heartbeats:
            if b.consumer == self.target:
                self.header_writer.write_function_prototype(
                    Function(b.callback_function_name, [], "void")
                )
                self.src_writer.write_function_start(
                    Function(b.callback_function_name, [], "__attribute__((weak)) void")
                )
                self.src_writer.write_function_end()
        self.header_writer.write_newline()

    def generate_transport_prototypes(self):
        self.header_writer.write_comment("Transport Buffers", indent_level=0)
        self.header_writer.write_newline()

        for n in self.config.nodes:
            self.header_writer.write_extern_variable(
                Variable(
                    f"{n.buffer_variable_name}", "proton_buffer_t"
                )
            )

        self.header_writer.write_newline()
        self.header_writer.write_comment("Transport Prototypes", indent_level=0)
        self.header_writer.write_newline()

        for n in self.config.nodes:
            if n.name == self.target:
                continue

            self.header_writer.write_function_prototype(
                Function(n.transport_connect_func, [], "bool")
            )

            self.header_writer.write_function_prototype(
                Function(n.transport_disconnect_func, [], "bool")
            )

            self.header_writer.write_function_prototype(
                Function(
                    n.transport_read_func,
                    [Variable("buf", "uint8_t *"), Variable("len", "size_t")],
                    "size_t",
                )
            )

            self.header_writer.write_function_prototype(
                Function(
                    n.transport_write_func,
                    [Variable("buf", "const uint8_t *"), Variable("len", "size_t")],
                    "size_t",
                )
            )

        self.header_writer.write_newline()

    def generate_receive_function(self):
        # Generate prototype in header file for users to use
        self.header_writer.write_comment("Bundle Receive Prototype", indent_level=0)
        self.header_writer.write_newline()

        # Generate function source
        self.src_writer.write_comment("Bundle Receive Function", indent_level=0)
        self.src_writer.write_newline()

        for n in self.config.peers:
            # Name, parameters, and return type of the decode function
            receive_function = Function(
                f"PROTON_PEER__Receive{n.name.title()}",
                [
                    Variable("buffer", "const uint8_t*"),
                    Variable("length", "size_t"),
                ],
                "proton_status_e",
            )

            self.header_writer.write_function_prototype(receive_function)
            self.header_writer.write_newline()

            self.src_writer.write_function_start(receive_function)

            # Check for valid buffer
            self.src_writer.write_comment("Check that buffer is not NULL")
            self.src_writer.write_if_statement_start(
                "buffer == NULL"
            )
            self.src_writer.write("return PROTON_NULL_PTR_ERROR;", indent_level=2)
            self.src_writer.write_if_statement_end()
            self.src_writer.write_newline()


            # Initialise variables
            self.src_writer.write("proton_bundle_handle_t * handle;")
            self.src_writer.write("PROTON_BUNDLE_e id;")
            self.src_writer.write("proton_callback_t callback;")
            self.src_writer.write("proton_status_e status;")
            self.src_writer.write(f"proton_peer_t * peer = &{self.config.target_node.node_variable_name}.peers[PROTON_PEER__{n.name.upper()}];")
            self.src_writer.write_newline()

            # Check for peer lock and unlock functions
            self.src_writer.write_comment("Check that peer mutex functions are not NULL")
            self.src_writer.write_if_statement_start(
                "peer->state == PROTON_NODE_UNCONFIGURED"
            )
            self.src_writer.write("return PROTON_INVALID_STATE_ERROR;", indent_level=2)
            self.src_writer.write_if_statement_end()
            self.src_writer.write_newline()

            # Attempt to decode bundle ID
            self.src_writer.write_comment("Decode bundle ID")
            self.src_writer.write("status = PROTON_DecodeId(&id, peer);", indent_level=1)
            self.src_writer.write_newline()
            self.src_writer.write_if_statement_start("status != PROTON_OK", indent_level=1)
            self.src_writer.write("return status;", indent_level=2)
            self.src_writer.write_if_statement_end(indent_level=1)
            self.src_writer.write_newline()

            # Check which bundle we received
            self.src_writer.write_switch_start("id")

            for b in self.config.bundles + self.config.heartbeats:
                if b.producer == n.name:
                    # Assign bundle and callback to appropriate values for this case
                    self.src_writer.write_case_start(b.HEARTBEAT_BUNDLE_ID if b.id == 0 else b.bundle_enum_name)
                    self.src_writer.write(
                        f"handle = &{b.internal_handle_variable_name};", indent_level=3
                    )
                    self.src_writer.write(
                        f"callback = {b.callback_function_name};", indent_level=3
                    )
                    self.src_writer.write("break;", indent_level=3)
                    self.src_writer.write_case_end()
                    self.src_writer.write_newline()

            # Default case is invalid, return error
            self.src_writer.write_case_default_start()
            self.src_writer.write("return PROTON_ERROR;", indent_level=3)
            self.src_writer.write_case_end()
            self.src_writer.write_switch_end()

            # Decode the bundle
            self.src_writer.write_comment("Decode bundle")
            self.src_writer.write("status = PROTON_Decode(handle, peer);", indent_level=1)
            self.src_writer.write_newline()
            self.src_writer.write_if_statement_start("status != PROTON_OK")
            self.src_writer.write("return status;", indent_level=2)
            self.src_writer.write_if_statement_end()
            self.src_writer.write_newline()

            # Execute the callback for this bundle
            self.src_writer.write_comment("Execute callback")
            self.src_writer.write_if_statement_start("callback")
            self.src_writer.write("callback();", indent_level=2)
            self.src_writer.write_if_statement_end()
            self.src_writer.write_newline()

            self.src_writer.write("return PROTON_OK;")
            self.src_writer.write_function_end()

    def generate_send_handle_function(self):
        # Name, parameters, and return type of the encode function
        send_function = Function(
            "PROTON_BUNDLE_SendHandle",
            [Variable("handle", "proton_bundle_handle_t *")],
            "proton_status_e",
        )

        # Generate function source
        self.src_writer.write_comment("Bundle Send Handle Function", indent_level=0)
        self.src_writer.write_newline()
        self.src_writer.write_function_start(send_function)

        # Check that write function is not NULL
        self.src_writer.write_comment("Check that handle and mutex functions are not NULL")
        self.src_writer.write_if_statement_start(
            f"handle == NULL || {self.config.target_node.node_variable_name}.lock == NULL || {self.config.target_node.node_variable_name}.unlock == NULL",
            indent_level=1,
        )
        self.src_writer.write("return PROTON_NULL_PTR_ERROR;", indent_level=2)
        self.src_writer.write_if_statement_end(indent_level=1)
        self.src_writer.write_newline()

        # Check that the node is connected
        self.src_writer.write_comment("Check that node is connected")
        self.src_writer.write_if_statement_start(
            f"{self.config.target_node.node_variable_name}.state != PROTON_NODE_ACTIVE",
            indent_level=1,
        )
        self.src_writer.write("return PROTON_INVALID_STATE_ERROR;", indent_level=2)
        self.src_writer.write_if_statement_end(indent_level=1)
        self.src_writer.write_newline()

        # Encode the bundle
        self.src_writer.write("proton_status_e status = PROTON_WRITE_ERROR;")
        self.src_writer.write_newline()

        self.src_writer.write_if_statement_start(
            f"{self.config.target_node.node_variable_name}.lock()"
        )
        self.src_writer.write_variable(
            Variable("bytes_encoded", "size_t"),
            indent_level=2
        )
        self.src_writer.write_comment("Encode bundle", indent_level=2)
        self.src_writer.write(
            f"status = PROTON_Encode(handle, {self.config.target_node.node_variable_name}.write_buf.data, {self.config.target_node.node_variable_name}.write_buf.len, &bytes_encoded);",
            indent_level=2,
        )
        self.src_writer.write_newline()

        self.src_writer.write_if_statement_start(
            "status == PROTON_OK && bytes_encoded > 0",
            indent_level=2,
        )
        self.src_writer.write_comment("Send bundle", indent_level=3)
        self.src_writer.write_for_loop_start(f"{self.config.target_node.node_variable_name}.peer_count", indent_level=3)
        self.src_writer.write_comment("Send to all bundle consumers", indent_level=4)
        self.src_writer.write_if_statement_start(
            "(handle->consumers & (1 << i))",
            indent_level=4,
        )
        self.src_writer.write_if_statement_start(
            f"{self.config.target_node.node_variable_name}.peers[i].transport.write({self.config.target_node.node_variable_name}.write_buf.data, bytes_encoded) == 0",
            indent_level=5,
        )
        self.src_writer.write_comment("Write failed", indent_level=6)
        self.src_writer.write(
            "status = PROTON_WRITE_ERROR;",
            indent_level=6,
        )
        self.src_writer.write_if_statement_end(indent_level=5)
        self.src_writer.write_if_statement_end(indent_level=4)
        self.src_writer.write_for_loop_end(indent_level=3)
        self.src_writer.write_if_statement_end(indent_level=2)

        self.src_writer.write_newline()
        self.src_writer.write(
            f"{self.config.target_node.node_variable_name}.unlock();", indent_level=2
        )

        self.src_writer.write_if_statement_end(indent_level=1)
        self.src_writer.write_newline()

        self.src_writer.write("return status;")
        self.src_writer.write_function_end()

    def generate_send_function(self):
        # Name, parameters, and return type of the encode function
        send_function = Function(
            "PROTON_BUNDLE_Send",
            [Variable("bundle", "PROTON_BUNDLE_e")],
            "proton_status_e",
        )

        # Generate prototype in header file for users to use
        self.header_writer.write_comment("Bundle Send Prototype", indent_level=0)
        self.header_writer.write_newline()
        self.header_writer.write_function_prototype(send_function)
        self.header_writer.write_newline()

        # Generate function source
        self.src_writer.write_comment("Bundle Send Function", indent_level=0)
        self.src_writer.write_newline()
        self.src_writer.write_function_start(send_function)

        # Check that peers are not NULL
        self.src_writer.write_comment("Check that peers are not NULL")
        self.src_writer.write_if_statement_start(
            f"{self.config.target_node.node_variable_name}.peers == NULL",
            indent_level=1,
        )
        self.src_writer.write("return PROTON_NULL_PTR_ERROR;", indent_level=2)
        self.src_writer.write_if_statement_end(indent_level=1)
        self.src_writer.write_newline()

        # Check that write function is not NULL
        self.src_writer.write_comment("Check that write functions are not NULL")
        self.src_writer.write_for_loop_start(f"{self.config.target_node.node_variable_name}.peer_count", indent_level=1)
        self.src_writer.write_if_statement_start(
            f"{self.config.target_node.node_variable_name}.peers[i].transport.write == NULL",
            indent_level=2,
        )
        self.src_writer.write("return PROTON_NULL_PTR_ERROR;", indent_level=3)
        self.src_writer.write_if_statement_end(indent_level=2)
        self.src_writer.write_for_loop_end(indent_level=1)
        self.src_writer.write_newline()

        # Check that the node is connected
        self.src_writer.write_comment("Check that node is connected")
        self.src_writer.write_if_statement_start(
            f"{self.config.target_node.node_variable_name}.state != PROTON_NODE_ACTIVE",
            indent_level=1,
        )
        self.src_writer.write("return PROTON_INVALID_STATE_ERROR;", indent_level=2)
        self.src_writer.write_if_statement_end(indent_level=1)
        self.src_writer.write_newline()

        # Initialise variables
        self.src_writer.write("proton_bundle_handle_t * handle;")
        self.src_writer.write_newline()

        # Check which bundle we received
        self.src_writer.write_comment("Find correct bundle handle")
        self.src_writer.write_switch_start("bundle")

        for b in self.config.bundles:
            if b.producer == self.target:
                # Assign bundle and callback to appropriate values for this case
                self.src_writer.write_case_start(b.HEARTBEAT_BUNDLE_ID if b.id == 0 else b.bundle_enum_name)
                self.src_writer.write(
                    f"handle = &{b.internal_handle_variable_name};", indent_level=3
                )
                self.src_writer.write("break;", indent_level=3)
                self.src_writer.write_case_end()
                self.src_writer.write_newline()

        # Default case is invalid, return error
        self.src_writer.write_case_default_start()
        self.src_writer.write("return PROTON_ERROR;", indent_level=3)
        self.src_writer.write_case_end()
        self.src_writer.write_switch_end()

        # Send the handle
        self.src_writer.write("return PROTON_BUNDLE_SendHandle(handle);")
        self.src_writer.write_function_end()

    def generate_send_heartbeat_function(self):
        if not self.config.target_node.heartbeat_enabled:
            return

        # Name, parameters, and return type of the encode function
        send_function = Function(
            "PROTON_BUNDLE_SendHeartbeat",
            [],
            "proton_status_e",
        )

        # Generate prototype in header file for users to use
        self.header_writer.write_comment("Bundle Send Heartbeat Prototype", indent_level=0)
        self.header_writer.write_newline()
        self.header_writer.write_function_prototype(send_function)
        self.header_writer.write_newline()

        # Generate function source
        self.src_writer.write_comment("Bundle Send Heartbeat Function", indent_level=0)
        self.src_writer.write_newline()
        self.src_writer.write_function_start(send_function)

        handle: str
        bundle: str
        for b in self.config.heartbeats:
            if b.producer == self.target:
                handle = b.internal_handle_variable_name
                bundle = b.bundle_variable_name
                break

        self.src_writer.write(f"{bundle}.heartbeat++;")
        self.src_writer.write(f"return PROTON_BUNDLE_SendHandle(&{handle});")
        self.src_writer.write_function_end()

    def generate_print_function(self):
        # Name, parameters, and return type of the print function
        print_function = Function(
            "PROTON_BUNDLE_Print",
            [Variable("bundle", "PROTON_BUNDLE_e")],
            "void",
        )

        # Generate prototype in header file for users to use
        self.header_writer.write_comment("Bundle Print Prototype", indent_level=0)
        self.header_writer.write_newline()
        self.header_writer.write_function_prototype(print_function)
        self.header_writer.write_newline()

        # Generate function source
        self.src_writer.write_comment("Bundle Send Function", indent_level=0)
        self.src_writer.write_newline()
        self.src_writer.write_function_start(print_function)

        # Initialise variables
        self.src_writer.write("proton_bundle_handle_t * handle;")
        self.src_writer.write_newline()

        # Check which bundle we received
        self.src_writer.write_switch_start("bundle")

        for b in self.config.bundles:
            # Assign bundle and callback to appropriate values for this case
            self.src_writer.write_case_start(b.bundle_enum_name)
            self.src_writer.write(
                f"handle = &{b.internal_handle_variable_name};", indent_level=3
            )
            self.src_writer.write("break;", indent_level=3)
            self.src_writer.write_case_end()
            self.src_writer.write_newline()

        # Default case is invalid
        self.src_writer.write_case_default_start()
        self.src_writer.write("return;", indent_level=3)
        self.src_writer.write_case_end()
        self.src_writer.write_switch_end()

        # Decode the bundle
        self.src_writer.write_comment("Print bundle")
        self.src_writer.write("PROTON_PrintBundle(handle->bundle);")

        self.src_writer.write_function_end()

    def generate_node_info(self):
        self.header_writer.write_comment("Node Info", indent_level=0)
        self.header_writer.write_newline()
        for node in self.config.nodes:
            if node.name != self.target:
                continue

            self.header_writer.write_define(f'{node.name_define} "{node.name}"')
            self.header_writer.write_define(f'{node.heartbeat_enabled_define} {str(node.heartbeat_enabled).lower()}')
            self.header_writer.write_define(f'{node.heartbeat_period_define} {node.heartbeat_period}')

            if node.type == ProtonConfig.Node.UDP4:
                ip_hex = 0
                ip_split = node.ip.split(".")
                for i in range(0, 4):
                    ip_hex |= int(ip_split[i]) << 8 * (3 - i)
                self.header_writer.write_define(f"{node.ip_define} {hex(ip_hex)}")
                self.header_writer.write_define(f"{node.port_define} {node.port}")
            elif node.type == ProtonConfig.Node.SERIAL:
                self.header_writer.write_define(f'{node.device_define} \"{node.device}\"')
        self.header_writer.write_newline()

    def generate_peer_info(self):
        self.header_writer.write_comment("Peer Info", indent_level=0)
        self.header_writer.write_newline()
        for node in self.config.nodes:
            if node.name == self.target:
                continue

            self.header_writer.write_define(f'{node.name_define} "{node.name}"')
            self.header_writer.write_define(f'{node.heartbeat_enabled_define} {str(node.heartbeat_enabled).lower()}')
            self.header_writer.write_define(f'{node.heartbeat_period_define} {node.heartbeat_period}')

            if node.type == ProtonConfig.Node.UDP4:
                ip_hex = 0
                ip_split = node.ip.split(".")
                for i in range(0, 4):
                    ip_hex |= int(ip_split[i]) << 8 * (3 - i)
                self.header_writer.write_define(f"{node.ip_define} {hex(ip_hex)}")
                self.header_writer.write_define(f"{node.port_define} {node.port}")
            elif node.type == ProtonConfig.Node.SERIAL:
                self.header_writer.write_define(f'{node.device_define} \"{node.device}\"')
        self.header_writer.write_newline()

    def generate_init_prototype(self):
        self.header_writer.write_comment("Proton Init Prototype", indent_level=0)
        self.header_writer.write_newline()
        self.header_writer.write_function_prototype(Function("PROTON_Init", [], "proton_status_e"))
        self.header_writer.write_newline()

    def generate_init_function(self):
        self.src_writer.write_comment("Proton Init", indent_level=0)
        self.src_writer.write_newline()
        self.src_writer.write_function_start(Function("PROTON_Init", [], "proton_status_e"))

        self.src_writer.write_variable(
            Variable("status", "proton_status_e"),
            indent_level=1
        )
        self.src_writer.write_newline()

        self.src_writer.write("status = PROTON_BUNDLE_Init();")
        self.src_writer.write_if_statement_start("status != PROTON_OK")
        self.src_writer.write("return status;", indent_level=2)
        self.src_writer.write_if_statement_end()
        self.src_writer.write_newline()

        self.src_writer.write("status = PROTON_PEER_Init();")
        self.src_writer.write_if_statement_start("status != PROTON_OK")
        self.src_writer.write("return status;", indent_level=2)
        self.src_writer.write_if_statement_end()
        self.src_writer.write_newline()

        self.src_writer.write(
            f"status = PROTON_Configure(&{self.config.target_node.node_variable_name}, "
            f"(proton_heartbeat_t){self.config.target_node.heartbeat_value_define}, {self.config.target_node.mutex_lock_func},"
            f"{self.config.target_node.mutex_unlock_func}, {self.config.target_node.buffer_variable_name},"
            f"{self.config.target_node.peer_variable_name}, PROTON_PEER__COUNT);"
        )
        self.src_writer.write_if_statement_start("status != PROTON_OK")
        self.src_writer.write("return status;", indent_level=2)
        self.src_writer.write_if_statement_end()
        self.src_writer.write_newline()

        self.src_writer.write(
            f"return PROTON_Activate(&{self.config.target_node.node_variable_name});"
        )

        self.src_writer.write_function_end()

    def generate_mutex_prototypes(self):
        self.header_writer.write_comment("Mutex prototypes", indent_level=0)
        self.header_writer.write_newline()

        for n in self.config.nodes:
            self.header_writer.write_function_prototype(
                Function(n.mutex_lock_func, [], "bool")
            )
            self.header_writer.write_function_prototype(
                Function(n.mutex_unlock_func, [], "bool")
            )
            self.header_writer.write_newline()

        self.src_writer.write_comment("Weak Mutex functions", indent_level=0)
        self.src_writer.write_newline()

        for n in self.config.nodes:
            self.src_writer.write_function_start(
                Function(n.mutex_lock_func, [], "__attribute__((weak)) bool")
            )
            self.src_writer.write('return true;')
            self.src_writer.write_function_end()

            self.src_writer.write_function_start(
                Function(n.mutex_unlock_func, [], "__attribute__((weak)) bool")
            )
            self.src_writer.write('return true;')
            self.src_writer.write_function_end()

    def generate(self, name: str, target: str):
        self.target = target
        self.config.set_target(target)
        generated_filename = f"proton__{name}_{target}"

        self.src_writer = CWriter(
            self.destination_path,
            f"{generated_filename}.c",
        )

        self.header_writer = CWriter(
            self.destination_path,
            f"{generated_filename}.h",
        )

        self.header_writer.write_header_guard_open()
        self.header_writer.write_include("stdint.h")
        self.header_writer.write_include("stdbool.h")
        self.header_writer.write_include("protonc/proton.h")
        self.header_writer.write_newline()

        self.src_writer.write_include(generated_filename)
        self.src_writer.write_newline()

        self.generate_node_info()
        self.generate_peer_info()
        self.generate_node_ids()
        self.generate_bundle_ids()
        self.generate_signal_enums()
        self.generate_defines()
        self.generate_bundle_struct_typedefs()
        self.generate_node()
        self.generate_peers()
        self.generate_extern_bundle_structs()
        self.generate_bundle_structs()
        self.generate_signal_variables()
        self.generate_bundle_variable()

        self.generate_bundle_init_prototypes()
        self.generate_peer_init_prototypes()
        self.generate_bundle_send_handle_prototype()
        self.generate_bundle_init_functions()
        self.generate_peer_init_functions()
        self.generate_receive_function()
        self.generate_send_handle_function()
        self.generate_send_function()
        self.generate_send_heartbeat_function()
        self.generate_print_function()
        self.generate_consumer_callbacks()
        self.generate_transport_prototypes()
        self.generate_mutex_prototypes()

        self.generate_init_prototype()
        self.generate_init_function()

        self.header_writer.write_header_guard_close()

        self.header_writer.close_file()
        self.src_writer.close_file()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-c",
        "--config",
        type=str,
        action="store",
        default="/home/rkreinin/proto_ws/src/proton/examples/a300/a300.yaml",
        help="Configuration file path.",
    )

    parser.add_argument(
        "-d",
        "--destination",
        type=str,
        action="store",
        default="/home/rkreinin/proto_ws/src/proton/build/examples/a300/generated",
        help="Destination folder path for generated files.",
    )

    parser.add_argument(
        "-t",
        "--target",
        type=str,
        action="store",
        default="mcu",
        help="Target node for generation.",
    )

    args = parser.parse_args()

    file = args.config
    config_name = file.split("/")[-1].split(".")[0]
    dest = args.destination
    target = args.target
    generator = ProtonCGenerator(file, dest)

    exists = False
    for node in generator.config.nodes:
        if node.name == target:
            exists = True
            break

    if not exists:
        raise Exception(f'Invalid target "{target}"')
    else:
        generator.generate(config_name, target)

if __name__ == "__main__":
    main()
