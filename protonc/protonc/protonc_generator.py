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
from typing import List

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

    BUNDLE_INIT_FUNCTION = 'proton_bundle_init'
    BUNDLE_SEND_FUNCTION = 'proton_bundle_send'
    BUNDLE_SEND_HANDLE_FUNCTION = 'proton_bundle_send_handle'
    BUNDLE_SEND_HEARTBEAT_FUNCTION = 'proton_bundle_send_heartbeat'
    BUNDLE_PRINT_FUNCTION = 'proton_bundle_print'
    PEER_INIT_FUNCTION = 'proton_peer_init'
    INIT_FUNCTION = 'proton_init'
    PROTON_ENCODE_FUNCTION = 'proton_encode'
    PROTON_DECODE_FUNCTION = 'proton_decode'
    PROTON_DECODE_ID_FUNCTION = 'proton_decode_id'
    PROTON_CONFIGURE_FUNCTION = 'proton_configure'
    PROTON_ACTIVATE_FUNCTION = 'proton_activate'
    PROTON_PRINT_BUNDLE_FUNCTION = 'proton_print_bundle'
    PROTON_INIT_BUNDLE = 'proton_init_bundle'
    PROTON_INIT_PEER = 'proton_init_peer'


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
                        length=0 if s.length == 0 else s.length_define.name,
                        capacity=0 if s.capacity == 0 else s.capacity_define.name,
                        const=s.is_const
                    )
                )
            s = Struct(b.struct.struct, vars)
            self.header_writer.write_typedef_struct(s, indent_level=0)
            self.header_writer.write_newline()

    def generate_extern_bundle_structs(self):
        self.header_writer.write_comment("External Bundle Structures", indent_level=0)
        self.header_writer.write_newline()
        for b in self.config.bundles + self.config.heartbeats:
            self.header_writer.write_extern_variable(
                Variable(b.bundle_variable_name, b.struct.typedef)
            )
        self.header_writer.write_newline()

    def generate_node(self):
        node = Variable(self.node.node_variable_name, "proton_node_t", init=self.node.node_default_value_define.name)

        self.header_writer.write_comment("External Node", indent_level=0)
        self.header_writer.write_newline()
        self.header_writer.write_extern_variable(node)
        self.header_writer.write_newline()

        self.src_writer.write_comment("Node", indent_level=0)
        self.src_writer.write_newline()
        self.src_writer.write_variable(node)
        self.src_writer.write_newline()

    def generate_peers(self):
        default_value = "{"
        for i, p in enumerate(self.peers):
            if i < len(self.peers) - 1:
                default_value += f"{p.peer_default_value_define.name}, "
            else:
                default_value += f"{p.peer_default_value_define.name}}}"

        peers = Variable(self.node.peer_variable_name,
                         "static proton_peer_t",
                         capacity='PROTON__PEER__COUNT',
                         init=default_value)

        self.src_writer.write_comment("Peers", indent_level=0)
        self.src_writer.write_newline()
        self.src_writer.write_variable(peers)
        self.src_writer.write_newline()

    def generate_signal_enums(self):
        self.header_writer.write_comment("Signal Enums", indent_level=0)
        self.header_writer.write_newline()
        for b in self.config.bundles + self.config.heartbeats:
            e = [s.signal_enum_name.name for s in b.signals if not s.is_const]
            self.header_writer.write_enum(b.signals_enum_name.name, e, prefix_name=False)
            self.header_writer.write_newline()
        self.header_writer.write_newline()

    def generate_defines(self):
        self.header_writer.write_comment("Constant definitions", indent_level=0)
        self.header_writer.write_newline()

        for b in self.config.bundles + self.config.heartbeats:
            for i, s in enumerate(b.signals):
                if s.length > 0:
                    self.header_writer.write_define(f"{s.length_define} {s.length_define.value}")
                if s.capacity > 0:
                    self.header_writer.write_define(f"{s.capacity_define} {s.capacity_define.value}")
                self.header_writer.write_define(f"{s.value_define} {s.value_define.value}")
            self.header_writer.write_define(f"{b.default_value_define} {b.default_value_define.value}")

            # OR all producer node IDs
            producers = "("
            for i, producer in enumerate(b.producers_define.value):
                define: str
                if producer == self.target:
                   define = self.node.id_define.name
                else:
                    for p in self.peers:
                        if producer == p.name:
                            define = p.id_define.name
                if i < len(b.producers_define.value) - 1:
                    producers += f'{define} | '
                else:
                    producers += f'{define})'
            self.header_writer.write_define(f"{b.producers_define} {producers}")

            # OR all consumer node IDs
            consumers = "("
            for i, consumer in enumerate(b.consumers_define.value):
                define: str
                if consumer == self.target:
                   define = self.node.id_define.name
                else:
                    for p in self.peers:
                        if consumer == p.name:
                            define = p.id_define.name
                if i < len(b.consumers_define.value) - 1:
                    consumers += f'{define} | '
                else:
                    consumers += f'{define})'
            self.header_writer.write_define(f"{b.consumers_define} {consumers}")
            self.header_writer.write_newline()

    def generate_peer_ids(self):
        self.header_writer.write_comment("Peers", indent_level=0)
        self.header_writer.write_newline()

        peers = []
        ids = []
        for i, p in enumerate(self.peers):
            peers.append(p.peer_define.name)
            ids.append(i)

        peers.append("PROTON__PEER__COUNT")
        ids.append(len(self.peers))

        self.header_writer.write_enum(
            "PROTON__PEER",
            peers,
            ids,
            prefix_name=False
        )

        self.header_writer.write_newline()

    def generate_bundle_ids(self):
        self.header_writer.write_comment("Bundle IDs", indent_level=0)
        self.header_writer.write_newline()
        self.header_writer.write_enum(
            "PROTON__BUNDLE",
            [b.bundle_enum_name.name for b in self.config.bundles],
            [b.id for b in self.config.bundles],
            prefix_name=False
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
                Variable(b.bundle_variable_name, b.struct.typedef, init=b.default_value_define)
            )
        self.src_writer.write_newline()

    def generate_bundle_init_prototypes(self):
        self.src_writer.write_comment("Bundle Init Prototype", indent_level=0)
        self.src_writer.write_newline()
        for b in self.config.bundles + self.config.heartbeats:
            self.src_writer.write_function_prototype(
                Function(b.init_function_name.name, [], "proton_status_e")
            )
        self.src_writer.write_newline()

        self.header_writer.write_comment("Bundle Init Prototype", indent_level=0)
        self.header_writer.write_newline()
        self.header_writer.write_function_prototype(
            Function(self.BUNDLE_INIT_FUNCTION, [], "proton_status_e")
        )
        self.header_writer.write_newline()

    def generate_peer_init_prototypes(self):
        self.src_writer.write_comment("Peer Init Prototype", indent_level=0)
        self.src_writer.write_newline()
        for n in self.peers:
            self.src_writer.write_function_prototype(
                Function(n.peer_init_func.name, [], "proton_status_e")
            )
        self.src_writer.write_newline()

        self.header_writer.write_comment("Peer Init Prototype", indent_level=0)
        self.header_writer.write_newline()
        self.header_writer.write_function_prototype(
            Function(self.PEER_INIT_FUNCTION, [], "proton_status_e")
        )
        self.header_writer.write_newline()

    def generate_bundle_send_handle_prototype(self):
        self.src_writer.write_comment("Bundle Send Handle Prototype", indent_level=0)
        self.src_writer.write_newline()
        self.src_writer.write_function_prototype(
            Function(self.BUNDLE_SEND_HANDLE_FUNCTION, [Variable("handle","proton_bundle_handle_t *")], "proton_status_e")
        )
        self.src_writer.write_newline()

    def generate_bundle_init_functions(self):
        self.src_writer.write_comment("Bundle Init Functions", indent_level=0)
        self.src_writer.write_newline()
        for b in self.config.bundles + self.config.heartbeats:
            self.src_writer.write_function_start(
                Function(b.init_function_name.name, [], "proton_status_e")
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
                f"return {self.PROTON_INIT_BUNDLE}(&{b.internal_handle_variable_name}, "
                f"{b.HEARTBEAT_BUNDLE_ID if b.id == 0 else b.bundle_enum_name}, "
                f"{b.signal_handles_variable_name}, {b.signals_enum_count}, "
                f"{b.producers_define}, {b.consumers_define});"
            )
            self.src_writer.write_function_end()

        self.src_writer.write_function_start(Function(self.BUNDLE_INIT_FUNCTION, [], "proton_status_e"))
        self.src_writer.write("proton_status_e status;")
        self.src_writer.write_newline()
        for b in self.config.bundles + self.config.heartbeats:
            self.src_writer.write(f"status = {b.init_function_name.name}();", indent_level=1)
            self.src_writer.write_if_statement_start("status != PROTON_OK")
            self.src_writer.write("return status;", indent_level=2)
            self.src_writer.write_if_statement_end()
            self.src_writer.write_newline()
        self.src_writer.write("return PROTON_OK;")
        self.src_writer.write_function_end()

    def generate_peer_init_functions(self):
        self.src_writer.write_comment("Peer Init Functions", indent_level=0)
        self.src_writer.write_newline()

        for n in self.peers:
            self.src_writer.write_function_start(Function(n.peer_init_func.name, [], "proton_status_e"))
            self.src_writer.write(f"return {self.PROTON_INIT_PEER}(&{self.node.peer_variable_name}[{n.peer_define}], "
                                  f"{n.id_define.name}, "
                                  f"(proton_heartbeat_t){n.heartbeat.default_value_define.name}, "
                                  f"(proton_transport_t){n.transport_define.name}, {n.receive_func.name}, {n.mutex_lock_func.name}, {n.mutex_unlock_func.name}, "
                                  f"{n.buffer_variable_name});")
            self.src_writer.write_function_end()

        self.src_writer.write_function_start(Function(self.PEER_INIT_FUNCTION, [], "proton_status_e"))
        self.src_writer.write("proton_status_e status;")
        self.src_writer.write_newline()

        for n in self.peers:
            self.src_writer.write(f"status = {n.peer_init_func.name}();")
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
            if self.target in b.consumers:
                self.header_writer.write_function_prototype(
                    Function(b.callback_function_name.name, [], "void")
                )
                self.src_writer.write_function_start(
                    Function(b.callback_function_name.name, [], "__attribute__((weak)) void")
                )
                self.src_writer.write_function_end()
        self.header_writer.write_newline()

    def generate_transport_prototypes(self):
        self.header_writer.write_comment("Transport Buffers", indent_level=0)
        self.header_writer.write_newline()

        for n in self.peers + [self.node]:
            self.header_writer.write_extern_variable(
                Variable(
                    f"{n.buffer_variable_name}", "proton_buffer_t"
                )
            )

        self.header_writer.write_newline()
        self.header_writer.write_comment("Transport Prototypes", indent_level=0)
        self.header_writer.write_newline()

        for p in self.peers:
            self.header_writer.write_function_prototype(
                Function(p.transport_connect_func.name, [], "bool")
            )

            self.header_writer.write_function_prototype(
                Function(p.transport_disconnect_func.name, [], "bool")
            )

            self.header_writer.write_function_prototype(
                Function(
                    p.transport_read_func.name,
                    [Variable("buf", "uint8_t *"), Variable("len", "size_t")],
                    "size_t",
                )
            )

            self.header_writer.write_function_prototype(
                Function(
                    p.transport_write_func.name,
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

        for n in self.peers:
            # Name, parameters, and return type of the decode function
            receive_function = Function(
                n.receive_func.name,
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
            self.src_writer.write(f"{ProtonConfig.PROTON_BUNDLE_ENUM} id;")
            self.src_writer.write("proton_callback_t callback;")
            self.src_writer.write("proton_status_e status;")
            self.src_writer.write(f"proton_peer_t * peer = &{self.node.node_variable_name}.peers[{n.peer_define}];")
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

            self.src_writer.write_if_statement_start('peer->atomic_buffer.lock()')
            self.src_writer.write(f"status = {self.PROTON_DECODE_ID_FUNCTION}(peer->atomic_buffer.buffer, &id);", indent_level=2)
            self.src_writer.write_if_statement_start('!peer->atomic_buffer.unlock()', indent_level=2)
            self.src_writer.write('return PROTON_MUTEX_ERROR;', indent_level=3)
            self.src_writer.write_if_statement_end(indent_level=2)
            self.src_writer.write_if_statement_end(indent_level=1)
            self.src_writer.write_else_statement_start(indent_level=1)
            self.src_writer.write('return PROTON_MUTEX_ERROR;', indent_level=2)
            self.src_writer.write_else_statement_end(indent_level=1)
            self.src_writer.write_newline()
            self.src_writer.write_if_statement_start("status != PROTON_OK", indent_level=1)
            self.src_writer.write("return status;", indent_level=2)
            self.src_writer.write_if_statement_end(indent_level=1)
            self.src_writer.write_newline()

            # Check which bundle we received
            self.src_writer.write_switch_start("id")

            for b in self.config.bundles + self.config.heartbeats:
                if n.name in b.producers and self.node.name in b.consumers:
                    # Assign bundle and callback to appropriate values for this case
                    self.src_writer.write_case_start(b.HEARTBEAT_BUNDLE_ID if b.id == 0 else b.bundle_enum_name)
                    self.src_writer.write(
                        f"handle = &{b.internal_handle_variable_name};", indent_level=3
                    )
                    self.src_writer.write(
                        f"callback = {b.callback_function_name.name};", indent_level=3
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

            self.src_writer.write_if_statement_start('peer->atomic_buffer.lock()')
            self.src_writer.write(f"status = {self.PROTON_DECODE_FUNCTION}(handle, peer->atomic_buffer.buffer, length);", indent_level=2)
            self.src_writer.write_if_statement_start('!peer->atomic_buffer.unlock()', indent_level=2)
            self.src_writer.write('return PROTON_MUTEX_ERROR;', indent_level=3)
            self.src_writer.write_if_statement_end(indent_level=2)
            self.src_writer.write_if_statement_end(indent_level=1)
            self.src_writer.write_else_statement_start(indent_level=1)
            self.src_writer.write('return PROTON_MUTEX_ERROR;', indent_level=2)
            self.src_writer.write_else_statement_end(indent_level=1)
            self.src_writer.write_newline()

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
            self.BUNDLE_SEND_HANDLE_FUNCTION,
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
            "handle == NULL",
            indent_level=1,
        )
        self.src_writer.write("return PROTON_NULL_PTR_ERROR;", indent_level=2)
        self.src_writer.write_if_statement_end(indent_level=1)
        self.src_writer.write_newline()

        # Check that the node is connected
        self.src_writer.write_comment("Check that node is connected")
        self.src_writer.write_if_statement_start(
            f"{self.node.node_variable_name}.state != PROTON_NODE_ACTIVE",
            indent_level=1,
        )
        self.src_writer.write("return PROTON_INVALID_STATE_ERROR;", indent_level=2)
        self.src_writer.write_if_statement_end(indent_level=1)
        self.src_writer.write_newline()

        # Encode the bundle
        self.src_writer.write("proton_status_e status = PROTON_WRITE_ERROR;")
        self.src_writer.write_newline()

        self.src_writer.write_variable(
            Variable("bytes_encoded", "size_t"),
            indent_level=1
        )
        self.src_writer.write_newline()

        self.src_writer.write_comment("Encode bundle", indent_level=1)

        self.src_writer.write_if_statement_start(f'{self.node.node_variable_name}.atomic_buffer.lock()')
        self.src_writer.write(
            f"status = {self.PROTON_ENCODE_FUNCTION}(handle, {self.node.node_variable_name}.atomic_buffer.buffer,  &bytes_encoded);",
            indent_level=2,
        )
        self.src_writer.write_if_statement_start(f'!{self.node.node_variable_name}.atomic_buffer.unlock()', indent_level=2)
        self.src_writer.write('return PROTON_MUTEX_ERROR;', indent_level=3)
        self.src_writer.write_if_statement_end(indent_level=2)
        self.src_writer.write_if_statement_end(indent_level=1)
        self.src_writer.write_else_statement_start(indent_level=1)
        self.src_writer.write('return PROTON_MUTEX_ERROR;', indent_level=2)
        self.src_writer.write_else_statement_end(indent_level=1)
        self.src_writer.write_newline()

        self.src_writer.write_if_statement_start(
            "status == PROTON_OK && bytes_encoded > 0",
            indent_level=1,
        )
        self.src_writer.write_comment("Send bundle", indent_level=2)
        self.src_writer.write_for_loop_start(f"{self.node.node_variable_name}.peer_count", indent_level=2)
        self.src_writer.write_comment("Send to all bundle consumers", indent_level=3)
        self.src_writer.write_if_statement_start(
            f"handle->consumers & {self.node.node_variable_name}.peers[i].id",
            indent_level=3,
        )
        self.src_writer.write(f"{self.node.node_variable_name}.atomic_buffer.lock();", indent_level=4)
        self.src_writer.write_if_statement_start(
            f"{self.node.node_variable_name}.peers[i].transport.write({self.node.node_variable_name}.atomic_buffer.buffer.data, bytes_encoded) == 0",
            indent_level=4,
        )
        self.src_writer.write_comment("Write failed", indent_level=5)
        self.src_writer.write(
            "status = PROTON_WRITE_ERROR;",
            indent_level=5,
        )
        self.src_writer.write_if_statement_end(indent_level=4)

        self.src_writer.write(f"{self.node.node_variable_name}.atomic_buffer.unlock();", indent_level=4)
        self.src_writer.write_if_statement_end(indent_level=3)
        self.src_writer.write_for_loop_end(indent_level=2)
        self.src_writer.write_if_statement_end(indent_level=1)

        self.src_writer.write_newline()

        self.src_writer.write("return status;")
        self.src_writer.write_function_end()

    def generate_send_function(self):
        # Name, parameters, and return type of the encode function
        send_function = Function(
            self.BUNDLE_SEND_FUNCTION,
            [Variable("bundle", ProtonConfig.PROTON_BUNDLE_ENUM)],
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
            f"{self.node.node_variable_name}.peers == NULL",
            indent_level=1,
        )
        self.src_writer.write("return PROTON_NULL_PTR_ERROR;", indent_level=2)
        self.src_writer.write_if_statement_end(indent_level=1)
        self.src_writer.write_newline()

        # Check that write function is not NULL
        self.src_writer.write_comment("Check that write functions are not NULL")
        self.src_writer.write_for_loop_start(f"{self.node.node_variable_name}.peer_count", indent_level=1)
        self.src_writer.write_if_statement_start(
            f"{self.node.node_variable_name}.peers[i].transport.write == NULL",
            indent_level=2,
        )
        self.src_writer.write("return PROTON_NULL_PTR_ERROR;", indent_level=3)
        self.src_writer.write_if_statement_end(indent_level=2)
        self.src_writer.write_for_loop_end(indent_level=1)
        self.src_writer.write_newline()

        # Check that the node is connected
        self.src_writer.write_comment("Check that node is connected")
        self.src_writer.write_if_statement_start(
            f"{self.node.node_variable_name}.state != PROTON_NODE_ACTIVE",
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
            if self.target in b.producers:
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
        self.src_writer.write(f"return {self.BUNDLE_SEND_HANDLE_FUNCTION}(handle);")
        self.src_writer.write_function_end()

    def generate_send_heartbeat_function(self):
        if not self.node.heartbeat.enabled:
            return

        # Name, parameters, and return type of the encode function
        send_function = Function(
            self.BUNDLE_SEND_HEARTBEAT_FUNCTION,
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
            if self.target in b.producers:
                handle = b.internal_handle_variable_name
                bundle = b.bundle_variable_name
                break

        self.src_writer.write(f"{bundle}.heartbeat++;")
        self.src_writer.write(f"return {self.BUNDLE_SEND_HANDLE_FUNCTION}(&{handle});")
        self.src_writer.write_function_end()

    def generate_print_function(self):
        # Name, parameters, and return type of the print function
        print_function = Function(
            self.BUNDLE_PRINT_FUNCTION,
            [Variable("bundle", ProtonConfig.PROTON_BUNDLE_ENUM)],
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
        self.src_writer.write(f"{self.PROTON_PRINT_BUNDLE_FUNCTION}(handle->bundle);")

        self.src_writer.write_function_end()

    def generate_node_defines(self, node: ProtonConfig.Node):
        self.header_writer.write_comment(f"{node.name} Node Defines", indent_level=0)
        self.header_writer.write_newline()

        self.header_writer.write_define(f'{node.name_define} {node.name_define.value}')
        self.header_writer.write_define(f'{node.id_define} {node.id_define.value}')

        if node == self.node:
            self.header_writer.write_define(f'{node.node_default_value_define} {node.node_default_value_define.value}')
        else:
            self.header_writer.write_define(f'{node.peer_default_value_define} {node.peer_default_value_define.value}')
            self.header_writer.write_define(f'{node.transport_define.name} {node.transport_define.value}')

        for d in node.heartbeat.defines:
            self.header_writer.write_define(f'{d} {d.value}')

        for e in node.endpoints:
            for d in e.defines:
                self.header_writer.write_define(f'{d} {d.value}')

        self.header_writer.write_newline()

    def generate_init_prototype(self):
        self.header_writer.write_comment("Proton Init Prototype", indent_level=0)
        self.header_writer.write_newline()
        self.header_writer.write_function_prototype(Function(self.INIT_FUNCTION, [], "proton_status_e"))
        self.header_writer.write_newline()

    def generate_init_function(self):
        self.src_writer.write_comment("Proton Init", indent_level=0)
        self.src_writer.write_newline()
        self.src_writer.write_function_start(Function(self.INIT_FUNCTION, [], "proton_status_e"))

        self.src_writer.write_variable(
            Variable("status", "proton_status_e"),
            indent_level=1
        )
        self.src_writer.write_newline()

        self.src_writer.write(f"status = {self.BUNDLE_INIT_FUNCTION}();")
        self.src_writer.write_if_statement_start("status != PROTON_OK")
        self.src_writer.write("return status;", indent_level=2)
        self.src_writer.write_if_statement_end()
        self.src_writer.write_newline()

        self.src_writer.write(f"status = {self.PEER_INIT_FUNCTION}();")
        self.src_writer.write_if_statement_start("status != PROTON_OK")
        self.src_writer.write("return status;", indent_level=2)
        self.src_writer.write_if_statement_end()
        self.src_writer.write_newline()

        self.src_writer.write(
            f"status = {self.PROTON_CONFIGURE_FUNCTION}(&{self.node.node_variable_name}, "
            f"(proton_heartbeat_t){self.node.heartbeat.default_value_define.name}, {self.node.mutex_lock_func.name}, "
            f"{self.node.mutex_unlock_func.name}, {self.node.buffer_variable_name}, "
            f"{self.node.peer_variable_name}, PROTON__PEER__COUNT);"
        )
        self.src_writer.write_if_statement_start("status != PROTON_OK")
        self.src_writer.write("return status;", indent_level=2)
        self.src_writer.write_if_statement_end()
        self.src_writer.write_newline()

        self.src_writer.write(
            f"return {self.PROTON_ACTIVATE_FUNCTION}(&{self.node.node_variable_name});"
        )

        self.src_writer.write_function_end()

    def generate_mutex_prototypes(self):
        self.header_writer.write_comment("Mutex prototypes", indent_level=0)
        self.header_writer.write_newline()

        for n in self.nodes:
            self.header_writer.write_function_prototype(
                Function(n.mutex_lock_func.name, [], "bool")
            )
            self.header_writer.write_function_prototype(
                Function(n.mutex_unlock_func, [], "bool")
            )
            self.header_writer.write_newline()

        self.src_writer.write_comment("Weak Mutex functions", indent_level=0)
        self.src_writer.write_newline()

        for n in self.nodes:
            self.src_writer.write_function_start(
                Function(n.mutex_lock_func.name, [], "__attribute__((weak)) bool")
            )
            self.src_writer.write('return true;')
            self.src_writer.write_function_end()

            self.src_writer.write_function_start(
                Function(n.mutex_unlock_func, [], "__attribute__((weak)) bool")
            )
            self.src_writer.write('return true;')
            self.src_writer.write_function_end()

    def generate(self, name: str, target: str):
        self.configure(target)
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

        self.generate_node_defines(self.node)

        for p in self.peers:
            self.generate_node_defines(p)

        self.generate_peer_ids()
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

    def configure(self, target: str):
        self.target = target
        self.node: ProtonConfig.Node = self.config.nodes[self.target]
        self.peers: List[ProtonConfig.Node] = []
        self.connections: List[ProtonConfig.Connection] = []

        # Add peer from any connection that has target node
        for connection in self.config.connections:
            if connection.first.node == self.target:
                self.peers.append(self.config.nodes[connection.second.node])
                self.connections.append(connection)
            elif connection.second.node == self.target:
                self.peers.append(self.config.nodes[connection.first.node])
                self.connections.append(connection)

        self.nodes = self.peers + [self.node]

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
    for name in generator.config.nodes.keys():
        if name == target:
            exists = True
            break

    if exists:
        generator.generate(config_name, target)
    else:
        raise Exception(f'Invalid target "{target}"')

if __name__ == "__main__":
    main()
