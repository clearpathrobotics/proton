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
from enum import Enum
import os
import sys
import yaml

from config import ProtonConfig
from source_writer import CWriter, Variable, Struct, Function


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

    def __init__(self, config_file: str):
        self.config_file = config_file
        assert os.path.exists(self.config_file)

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

    def get_message_struct_name(self, name) -> str:
        return f"PROTON_MESSAGE_STRUCT__{name}"

    def generate_message_struct_typedefs(self):
        self.header_writer.write_comment(
            "Message Structure Definitions", indent_level=0
        )
        self.header_writer.write_newline()
        for m in self.config.messages:
            vars = []
            for s in m.signals:
                if s.type == ProtonConfig.Signal.SignalTypes.LIST_STRING:
                    string_struct = Struct(
                        s.signal,
                        [
                            Variable("list", "char *", s.length),
                            Variable("strings", "char", s.length, s.capacity),
                        ],
                    )
                    vars.append(string_struct)
                else:
                    vars.append(
                        Variable(
                            s.signal, self.SIGNAL_TYPE_MAP[s.type], s.length, s.capacity
                        )
                    )
            s = Struct(self.get_message_struct_name(m.name), vars)
            self.header_writer.write_typedef_struct(s, indent_level=0)
            self.header_writer.write_newline()

    def generate_extern_message_structs(self):
        self.header_writer.write_comment("External Message Structures", indent_level=0)
        self.header_writer.write_newline()
        for m in self.config.messages:
            self.header_writer.write_extern_variable(
                Variable(m.name, f"{self.get_message_struct_name(m.name)}_t")
            )
        self.header_writer.write_newline()

    def generate_extern_bundle(self):
        self.header_writer.write_comment("External Bundles", indent_level=0)
        self.header_writer.write_newline()
        for m in self.config.messages:
            self.header_writer.write_extern_variable(
                Variable(f"{m.name}_bundle", "proton_bundle_t")
            )
        self.header_writer.write_newline()

    def generate_signal_enums(self):
        self.header_writer.write_comment("Signal Enums", indent_level=0)
        self.header_writer.write_newline()
        for m in self.config.messages:
            e = [s.signal for s in m.signals]
            self.header_writer.write_enum(f"PROTON_SIGNAL__{m.name}", e)
            self.header_writer.write_newline()
        self.header_writer.write_newline()

    def generate_signal_variables(self):
        self.src_writer.write_comment("Signals", indent_level=0)
        self.src_writer.write_newline()
        for m in self.config.messages:
            signals = Variable(
                f"{m.name}_signals",
                "proton_Signal",
                length=f"PROTON_SIGNAL__{m.name.upper()}_COUNT",
            )
            self.src_writer.write_variable(signals)
        self.src_writer.write_newline()

    def generate_signal_schema_variables(self):
        self.src_writer.write_comment("Signal schemas", indent_level=0)
        self.src_writer.write_newline()
        for m in self.config.messages:
            signals = Variable(
                f"{m.name}_signal_schema",
                "proton_signal_schema_t",
                length=f"PROTON_SIGNAL__{m.name.upper()}_COUNT",
            )
            self.src_writer.write_variable(signals)
        self.src_writer.write_newline()

    def generate_bundle_variable(self):
        self.src_writer.write_comment("Bundles", indent_level=0)
        self.src_writer.write_newline()
        for m in self.config.messages:
            proton = Variable(f"{m.name}_bundle", "proton_bundle_t")
            self.src_writer.write_variable(proton)
        self.src_writer.write_newline()

    def generate_message_structs(self):
        self.src_writer.write_comment("Message Structures", indent_level=0)
        self.src_writer.write_newline()
        for m in self.config.messages:
            self.src_writer.write_variable(
                Variable(m.name, f"{self.get_message_struct_name(m.name)}_t")
            )
        self.src_writer.write_newline()

    def generate_message_init_prototypes(self):
        self.src_writer.write_comment("Message Init Prototypes", indent_level=0)
        self.src_writer.write_newline()
        for m in self.config.messages:
            self.src_writer.write_function_prototype(
                Function(f"PROTON_MESSAGE_init_{m.name}", [], "void")
            )
        self.src_writer.write_newline()

        self.header_writer.write_comment("Message Init Prototypes", indent_level=0)
        self.header_writer.write_newline()
        self.header_writer.write_function_prototype(
            Function("PROTON_MESSAGE_init", [], "void")
        )
        self.header_writer.write_newline()

    def generate_message_init_functions(self):
        self.src_writer.write_comment("Message Init Functions", indent_level=0)
        self.src_writer.write_newline()
        for m in self.config.messages:
            self.src_writer.write_function_start(
                Function(f"PROTON_MESSAGE_init_{m.name}", [], "void")
            )
            for s in m.signals:
                self.src_writer.write(
                    f"{m.name}_signal_schema[PROTON_SIGNAL__{m.name.upper()}__{s.signal.upper()}].type = PROTON_SIGNAL_TYPE_{s.type.upper()}_VALUE;"
                )
                match s.type:
                    # case (ProtonConfig.Signal.SignalTypes.DOUBLE | ProtonConfig.Signal.SignalTypes.FLOAT | ProtonConfig.Signal.SignalTypes.INT32 |
                    #      ProtonConfig.Signal.SignalTypes.INT64 | ProtonConfig.Signal.SignalTypes.UINT32 | ProtonConfig.Signal.SignalTypes.UINT64 |
                    #      ProtonConfig.Signal.SignalTypes.BOOL):
                    #     self.src_writer.write(f'{m.name}_signal_schema[PROTON_SIGNAL__{m.name.upper()}__{s.signal.upper()}].arg = proton_list_arg_init_default;')
                    case (
                        ProtonConfig.Signal.SignalTypes.STRING
                        | ProtonConfig.Signal.SignalTypes.BYTES
                    ):
                        self.src_writer.write(
                            f"{m.name}_signal_schema[PROTON_SIGNAL__{m.name.upper()}__{s.signal.upper()}].arg.values = {m.name}.{s.signal};"
                        )
                        self.src_writer.write(
                            f"{m.name}_signal_schema[PROTON_SIGNAL__{m.name.upper()}__{s.signal.upper()}].arg.capacity = {s.capacity};"
                        )
                        self.src_writer.write(
                            f"{m.name}_signal_schema[PROTON_SIGNAL__{m.name.upper()}__{s.signal.upper()}].arg.size = 0;"
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
                            f"{m.name}_signal_schema[PROTON_SIGNAL__{m.name.upper()}__{s.signal.upper()}].arg.values = {m.name}.{s.signal};"
                        )
                        self.src_writer.write(
                            f"{m.name}_signal_schema[PROTON_SIGNAL__{m.name.upper()}__{s.signal.upper()}].arg.capacity = {s.length};"
                        )
                        self.src_writer.write(
                            f"{m.name}_signal_schema[PROTON_SIGNAL__{m.name.upper()}__{s.signal.upper()}].arg.size = 0;"
                        )
                    case ProtonConfig.Signal.SignalTypes.LIST_STRING:
                        self.src_writer.write(
                            f"{m.name}_signal_schema[PROTON_SIGNAL__{m.name.upper()}__{s.signal.upper()}].arg.values = {m.name}.{s.signal}.list;"
                        )
                        self.src_writer.write(
                            f"{m.name}_signal_schema[PROTON_SIGNAL__{m.name.upper()}__{s.signal.upper()}].arg.capacity = {s.length};"
                        )
                        self.src_writer.write(
                            f"{m.name}_signal_schema[PROTON_SIGNAL__{m.name.upper()}__{s.signal.upper()}].arg.size = 0;"
                        )
                        self.src_writer.write_for_loop_start(s.length, indent_level=1)
                        self.src_writer.write(
                            f"{m.name}.{s.signal}.list[i] = {m.name}.{s.signal}.strings[i];",
                            indent_level=2,
                        )
                        self.src_writer.write_for_loop_end(indent_level=1)
            self.src_writer.write(
                f"PROTON_InitProton(&{m.name}_bundle, {m.id}, {m.name}_signals, {m.name}_signal_schema, PROTON_SIGNAL__{m.name.upper()}_COUNT);"
            )
            self.src_writer.write_function_end()

        self.src_writer.write_function_start(
            Function("PROTON_MESSAGE_init", [], "void")
        )
        for m in self.config.messages:
            self.src_writer.write(f"PROTON_MESSAGE_init_{m.name}();", indent_level=1)
        self.src_writer.write_function_end()

    def generate(self, name: str):
        generated_filename = f"proton__{name}"

        self.src_writer = CWriter(
            os.path.join(os.getcwd(), "../tests/sample/generated/"), f"{generated_filename}.c"
        )

        self.header_writer = CWriter(
            os.path.join(os.getcwd(), "../tests/sample/generated/"), f"{generated_filename}.h"
        )

        self.header_writer.write_header_guard_open()
        self.header_writer.write_include("stdint.h")
        self.header_writer.write_include("stdbool.h")
        self.header_writer.write_include("proton.h")
        self.header_writer.write_newline()

        self.src_writer.write_include(generated_filename)
        self.src_writer.write_newline()

        self.generate_signal_enums()
        self.generate_message_struct_typedefs()
        self.generate_extern_message_structs()
        self.generate_extern_bundle()
        self.generate_message_structs()
        self.generate_signal_variables()
        self.generate_signal_schema_variables()
        self.generate_bundle_variable()

        self.generate_message_init_prototypes()
        self.generate_message_init_functions()

        self.header_writer.write_header_guard_close()
        self.src_writer.close_file()


if __name__ == "__main__":
    # parser = argparse.ArgumentParser()
    # parser.add_argument(
    #   '-c',
    #   '--config',
    #   type=str,
    #   action='store',
    #   dest='config',
    #   default=os.path.join(os.getcwd(), '../../config/'),
    #   help='Configuration file path.'
    # )

    # args = parser.parse_args(sys.argv)

    file = os.path.join(os.getcwd(), "../tests/sample/config/sample.yaml")
    generator = ProtonCGenerator(file)
    generator.generate(file.split("/")[-1].split(".")[0])
