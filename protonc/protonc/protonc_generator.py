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
        for b in self.config.bundles:
            vars = []
            for s in b.signals:
                if s.type == ProtonConfig.Signal.SignalTypes.LIST_STRING and not s.is_const:
                    string_struct = Struct(
                        s.name,
                        [
                            Variable("list", "char *", s.length_define),
                            Variable(
                                "strings", "char", s.length_define, s.capacity_define, const=s.is_const
                            ),
                        ],
                    )
                    vars.append(string_struct)
                else:
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
        for b in self.config.bundles:
            self.header_writer.write_extern_variable(
                Variable(b.bundle_variable_name, f"{b.struct_name}_t")
            )
        self.header_writer.write_newline()

    def generate_node(self):
        node = Variable(self.config.target_node.node_variable_name, "proton_node_t")

        self.header_writer.write_comment("External Node", indent_level=0)
        self.header_writer.write_newline()
        self.header_writer.write_extern_variable(node)
        self.header_writer.write_newline()

        self.src_writer.write_comment("Node", indent_level=0)
        self.src_writer.write_newline()
        self.src_writer.write_variable(node)
        self.src_writer.write_newline()

    def generate_signal_enums(self):
        self.header_writer.write_comment("Signal Enums", indent_level=0)
        self.header_writer.write_newline()
        for b in self.config.bundles:
            e = [s.name for s in b.signals if not s.is_const]
            self.header_writer.write_enum(b.signals_enum_name, e)
            self.header_writer.write_newline()
        self.header_writer.write_newline()

    def generate_defines(self):
        self.header_writer.write_comment("Constant definitions", indent_level=0)
        self.header_writer.write_newline()
        for b in self.config.bundles:
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
        for b in self.config.bundles:
            signals = Variable(
                b.signals_variable_name,
                "static proton_signal_t",
                length=b.signals_enum_count,
            )
            self.src_writer.write_variable(signals)
        self.src_writer.write_newline()

    def generate_bundle_variable(self):
        self.src_writer.write_comment("Internal Bundles", indent_level=0)
        self.src_writer.write_newline()
        for b in self.config.bundles:
            proton = Variable(b.internal_bundle_variable_name, "static proton_bundle_t")
            self.src_writer.write_variable(proton)
        self.src_writer.write_newline()

    def generate_bundle_structs(self):
        self.src_writer.write_comment("External Bundle Structures", indent_level=0)
        self.src_writer.write_newline()
        for b in self.config.bundles:
            self.src_writer.write_variable(
                Variable(b.bundle_variable_name, f"{b.struct_name}_t", init=b.default_value_define)
            )
        self.src_writer.write_newline()

    def generate_bundle_init_prototypes(self):
        self.src_writer.write_comment("Bundle Init Prototype", indent_level=0)
        self.src_writer.write_newline()
        for b in self.config.bundles:
            self.src_writer.write_function_prototype(
                Function(b.init_function_name, [], "void")
            )
        self.src_writer.write_newline()

        self.header_writer.write_comment("Bundle Init Prototype", indent_level=0)
        self.header_writer.write_newline()
        self.header_writer.write_function_prototype(
            Function("PROTON_BUNDLE_Init", [], "void")
        )
        self.header_writer.write_newline()

    def generate_bundle_init_functions(self):
        self.src_writer.write_comment("Bundle Init Functions", indent_level=0)
        self.src_writer.write_newline()
        for b in self.config.bundles:
            self.src_writer.write_function_start(
                Function(b.init_function_name, [], "void")
            )
            for s in b.signals:
                if s.is_const:
                    continue
                self.src_writer.write(
                    f"{b.signals_variable_name}[{s.signal_enum_name}].signal.which_signal = {self.SIGNAL_TAG_MAP[s.type]};"
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
                            f"{b.signals_variable_name}[{s.signal_enum_name}].arg.data = &{b.bundle_variable_name}.{s.name};"
                        )
                    case (
                        ProtonConfig.Signal.SignalTypes.STRING
                        | ProtonConfig.Signal.SignalTypes.BYTES
                    ):
                        self.src_writer.write(
                            f"{b.signals_variable_name}[{s.signal_enum_name}].signal.signal.{s.type}_value = &{b.signals_variable_name}[{s.signal_enum_name}].arg;"
                        )
                        self.src_writer.write(
                            f"{b.signals_variable_name}[{s.signal_enum_name}].arg.data = {b.bundle_variable_name}.{s.name};"
                        )
                        self.src_writer.write(
                            f"{b.signals_variable_name}[{s.signal_enum_name}].arg.capacity = {s.capacity_define};"
                        )
                        self.src_writer.write(
                            f"{b.signals_variable_name}[{s.signal_enum_name}].arg.size = 0;"
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
                            f"{b.signals_variable_name}[{s.signal_enum_name}].signal.signal.{s.type}_value.{self.SIGNAL_VARIABLE_MAP[s.type]} = &{b.signals_variable_name}[{s.signal_enum_name}].arg;"
                        )
                        self.src_writer.write(
                            f"{b.signals_variable_name}[{s.signal_enum_name}].arg.data = {b.bundle_variable_name}.{s.name};"
                        )
                        self.src_writer.write(
                            f"{b.signals_variable_name}[{s.signal_enum_name}].arg.capacity = {s.length_define};"
                        )
                        self.src_writer.write(
                            f"{b.signals_variable_name}[{s.signal_enum_name}].arg.size = 0;"
                        )

                    case ProtonConfig.Signal.SignalTypes.LIST_STRING:
                        self.src_writer.write(
                            f"{b.signals_variable_name}[{s.signal_enum_name}].signal.signal.{s.type}_value.{self.SIGNAL_VARIABLE_MAP[s.type]} = &{b.signals_variable_name}[{s.signal_enum_name}].arg;"
                        )
                        self.src_writer.write(
                            f"{b.signals_variable_name}[{s.signal_enum_name}].arg.data = {b.bundle_variable_name}.{s.name}.list;"
                        )
                        self.src_writer.write(
                            f"{b.signals_variable_name}[{s.signal_enum_name}].arg.capacity = {s.length_define};"
                        )
                        self.src_writer.write(
                            f"{b.signals_variable_name}[{s.signal_enum_name}].arg.size = 0;"
                        )
                        self.src_writer.write_for_loop_start(s.length, indent_level=1)
                        self.src_writer.write(
                            f"{b.bundle_variable_name}.{s.name}.list[i] = {b.bundle_variable_name}.{s.name}.strings[i];",
                            indent_level=2,
                        )
                        self.src_writer.write_for_loop_end(indent_level=1)
                self.src_writer.write_newline()
            self.src_writer.write(
                f"PROTON_InitBundle(&{b.internal_bundle_variable_name}, {b.bundle_enum_name}, {b.signals_variable_name}, {b.signals_enum_count});"
            )
            self.src_writer.write_function_end()

        self.src_writer.write_function_start(Function("PROTON_BUNDLE_Init", [], "void"))
        for b in self.config.bundles:
            self.src_writer.write(f"{b.init_function_name}();", indent_level=1)
        self.src_writer.write_function_end()

    def generate_consumer_callbacks(self):
        self.header_writer.write_comment("Consumer callbacks", indent_level=0)
        self.header_writer.write_newline()
        for b in self.config.bundles:
            if b.consumer == self.target:
                self.header_writer.write_function_prototype(
                    Function(b.callback_function_name, [], "void")
                )
        self.header_writer.write_newline()

    def generate_transport_prototypes(self):
        self.header_writer.write_comment("Transport Buffers", indent_level=0)
        self.header_writer.write_newline()

        self.header_writer.write_extern_variable(
            Variable(
                f"proton_{self.config.target_node.name}_read_buffer", "proton_buffer_t"
            )
        )
        self.header_writer.write_extern_variable(
            Variable(
                f"proton_{self.config.target_node.name}_write_buffer", "proton_buffer_t"
            )
        )

        self.header_writer.write_newline()
        self.header_writer.write_comment("Transport Prototypes", indent_level=0)
        self.header_writer.write_newline()

        self.header_writer.write_function_prototype(
            Function(self.config.target_node.transport_connect_func, [], "bool")
        )

        self.header_writer.write_function_prototype(
            Function(self.config.target_node.transport_disconnect_func, [], "bool")
        )

        self.header_writer.write_function_prototype(
            Function(
                self.config.target_node.transport_read_func,
                [Variable("buf", "uint8_t *"), Variable("len", "size_t")],
                "size_t",
            )
        )

        self.header_writer.write_function_prototype(
            Function(
                self.config.target_node.transport_write_func,
                [Variable("buf", "const uint8_t *"), Variable("len", "size_t")],
                "size_t",
            )
        )

        self.header_writer.write_newline()

    def generate_receive_function(self):
        # Name, parameters, and return type of the decode function
        receive_function = Function(
            "PROTON_BUNDLE_Receive",
            [
                Variable("buffer", "const uint8_t*"),
                Variable("length", "size_t"),
            ],
            "bool",
        )

        # Generate prototype in header file for users to use
        self.header_writer.write_comment("Bundle Receive Prototype", indent_level=0)
        self.header_writer.write_newline()
        self.header_writer.write_function_prototype(receive_function)
        self.header_writer.write_newline()

        # Generate function source
        self.src_writer.write_comment("Bundle Receive Function", indent_level=0)
        self.src_writer.write_newline()
        self.src_writer.write_function_start(receive_function)

        # Initialise variables
        self.src_writer.write("proton_bundle_t * bundle;")
        self.src_writer.write("PROTON_BUNDLE_e id;")
        self.src_writer.write("proton_callback_t callback;")
        self.src_writer.write_newline()

        # Attempt to decode bundle ID
        self.src_writer.write_comment("Decode bundle ID")
        self.src_writer.write_if_statement_start(
            "!PROTON_DecodeId(&id, buffer, length)"
        )
        self.src_writer.write("return false;", indent_level=2)
        self.src_writer.write_if_statement_end()
        self.src_writer.write_newline()

        # Check which bundle we received
        self.src_writer.write_switch_start("id")

        for b in self.config.bundles:
            if b.consumer == self.target:
                # Assign bundle and callback to appropriate values for this case
                self.src_writer.write_case_start(b.bundle_enum_name)
                self.src_writer.write(
                    f"bundle = &{b.internal_bundle_variable_name};", indent_level=3
                )
                self.src_writer.write(
                    f"callback = {b.callback_function_name};", indent_level=3
                )
                self.src_writer.write("break;", indent_level=3)
                self.src_writer.write_case_end()
                self.src_writer.write_newline()

        # Default case is invalid, return false
        self.src_writer.write_case_default_start()
        self.src_writer.write("return false;", indent_level=3)
        self.src_writer.write_case_end()
        self.src_writer.write_switch_end()

        # Decode the bundle
        self.src_writer.write_comment("Decode bundle")
        self.src_writer.write_if_statement_start(
            "PROTON_Decode(bundle, buffer, length) != 0"
        )
        self.src_writer.write("return false;", indent_level=2)
        self.src_writer.write_if_statement_end()
        self.src_writer.write_newline()

        # Execute the callback for this bundle
        self.src_writer.write_comment("Execute callback")
        self.src_writer.write_if_statement_start("callback")
        self.src_writer.write("callback();", indent_level=2)
        self.src_writer.write_if_statement_end()
        self.src_writer.write_newline()

        self.src_writer.write("return true;")
        self.src_writer.write_function_end()

    def generate_send_function(self):
        # Name, parameters, and return type of the encode function
        send_function = Function(
            "PROTON_BUNDLE_Send",
            [Variable("bundle", "PROTON_BUNDLE_e")],
            "bool",
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

        # Initialise variables
        self.src_writer.write("proton_bundle_t * bundle_;")
        self.src_writer.write_newline()

        # Check which bundle we received
        self.src_writer.write_switch_start("bundle")

        for b in self.config.bundles:
            if b.producer == self.target:
                # Assign bundle and callback to appropriate values for this case
                self.src_writer.write_case_start(b.bundle_enum_name)
                self.src_writer.write(
                    f"bundle_= &{b.internal_bundle_variable_name};", indent_level=3
                )
                self.src_writer.write("break;", indent_level=3)
                self.src_writer.write_case_end()
                self.src_writer.write_newline()

        # Default case is invalid, return false
        self.src_writer.write_case_default_start()
        self.src_writer.write("return false;", indent_level=3)
        self.src_writer.write_case_end()
        self.src_writer.write_switch_end()

        # Decode the bundle
        self.src_writer.write_comment("Encode bundle")
        self.src_writer.write("bool ret = false;")
        self.src_writer.write_newline()

        self.src_writer.write_if_statement_start(
            f"{self.config.target_node.mutex_lock_func}()"
        )
        self.src_writer.write(
            f"int bytes_written = PROTON_Encode(bundle_, {self.config.target_node.node_variable_name}.write_buf.data, {self.config.target_node.node_variable_name}.write_buf.len);",
            indent_level=2,
        )
        self.src_writer.write_if_statement_start(
            f"bytes_written > 0 && {self.config.target_node.node_variable_name}.connected && {self.config.target_node.node_variable_name}.transport.write",
            indent_level=2,
        )
        self.src_writer.write_comment("Send bundle", indent_level=3)
        self.src_writer.write(
            f"ret = {self.config.target_node.node_variable_name}.transport.write({self.config.target_node.node_variable_name}.write_buf.data, bytes_written) > 0;",
            indent_level=3,
        )
        self.src_writer.write_if_statement_end(indent_level=2)

        self.src_writer.write_newline()
        self.src_writer.write(
            f"{self.config.target_node.mutex_unlock_func}();", indent_level=2
        )

        self.src_writer.write_if_statement_end(indent_level=1)
        self.src_writer.write_newline()

        self.src_writer.write("return ret;")
        self.src_writer.write_function_end()

    def generate_node_info(self):
        self.header_writer.write_comment("Node Info", indent_level=0)
        self.header_writer.write_newline()
        for node in self.config.nodes:
            self.header_writer.write_define(f'{node.name_define} "{node.name}"')
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
        self.header_writer.write_function_prototype(Function("PROTON_Init", [], "void"))
        self.header_writer.write_newline()

    def generate_init_function(self):
        self.src_writer.write_comment("Proton Init", indent_level=0)
        self.src_writer.write_newline()
        self.src_writer.write_function_start(Function("PROTON_Init", [], "void"))

        self.src_writer.write_variable(
            Variable(f"{self.config.target_node.name}_transport", "proton_transport_t"),
            indent_level=1,
        )
        self.src_writer.write(
            f"{self.config.target_node.name}_transport.connect = {self.config.target_node.transport_connect_func};"
        )
        self.src_writer.write(
            f"{self.config.target_node.name}_transport.disconnect = {self.config.target_node.transport_disconnect_func};"
        )
        self.src_writer.write(
            f"{self.config.target_node.name}_transport.read = {self.config.target_node.transport_read_func};"
        )
        self.src_writer.write(
            f"{self.config.target_node.name}_transport.write = {self.config.target_node.transport_write_func};"
        )
        self.src_writer.write_newline()

        self.src_writer.write("PROTON_BUNDLE_Init();")
        self.src_writer.write_newline()

        self.src_writer.write(
            f"PROTON_InitNode(&{self.config.target_node.node_variable_name}, "
            f"{self.config.target_node.name}_transport, PROTON_BUNDLE_Receive, "
            f"proton_{self.config.target_node.name}_read_buffer, proton_{self.config.target_node.name}_write_buffer);"
        )

        self.src_writer.write_function_end()

    def generate_mutex_prototypes(self):
        self.header_writer.write_comment("Mutex prototypes", indent_level=0)
        self.header_writer.write_newline()

        self.header_writer.write_function_prototype(
            Function(self.config.target_node.mutex_lock_func, [], "bool")
        )
        self.header_writer.write_function_prototype(
            Function(self.config.target_node.mutex_unlock_func, [], "bool")
        )
        self.header_writer.write_newline()

        self.src_writer.write_comment("Weak Mutex functions", indent_level=0)
        self.src_writer.write_newline()

        self.src_writer.write_function_start(
            Function(self.config.target_node.mutex_lock_func, [], "__attribute__((weak)) bool")
        )
        self.src_writer.write('return true;')
        self.src_writer.write_function_end()

        self.src_writer.write_function_start(
            Function(self.config.target_node.mutex_unlock_func, [], "__attribute__((weak)) bool")
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
        self.generate_bundle_ids()
        self.generate_signal_enums()
        self.generate_defines()
        self.generate_bundle_struct_typedefs()
        self.generate_node()
        self.generate_extern_bundle_structs()
        self.generate_bundle_structs()
        self.generate_signal_variables()
        self.generate_bundle_variable()

        self.generate_bundle_init_prototypes()
        self.generate_bundle_init_functions()
        self.generate_receive_function()
        self.generate_send_function()
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
        default="/home/rkreinin/proto_ws/src/proton/examples/j100/j100.yaml",
        help="Configuration file path.",
    )

    parser.add_argument(
        "-d",
        "--destination",
        type=str,
        action="store",
        default="/home/rkreinin/proto_ws/src/proton/build/examples/j100/generated",
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
