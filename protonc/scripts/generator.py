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
        return f'PROTON_MESSAGE_STRUCT__{name}'

    def generate_message_struct_typedefs(self):
        self.header_writer.write_comment('Message Structure Definitions', indent_level=0)
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
        self.header_writer.write_comment('External Message Structures', indent_level=0)
        self.header_writer.write_newline()
        for m in self.config.messages:
          self.header_writer.write_extern_variable(Variable(m.name, f'{self.get_message_struct_name(m.name)}_t'))
        self.header_writer.write_newline()

    def generate_signal_enums(self):
        self.header_writer.write_comment('Signal Enums', indent_level=0)
        self.header_writer.write_newline()
        for m in self.config.messages:
            e = [s.signal for s in m.signals]
            self.header_writer.write_enum(f'PROTON_SIGNAL_ENUM__{m.name}', e)
            self.header_writer.write_newline()
        self.header_writer.write_newline()

    def generate_message_structs(self):
        self.src_writer.write_comment('Message Structures', indent_level=0)
        self.src_writer.write_newline()
        for m in self.config.messages:
          self.src_writer.write_variable(Variable(m.name, f'{self.get_message_struct_name(m.name)}_t'))
        self.src_writer.write_newline()

    def generate_message_init_prototypes(self):
        self.header_writer.write_comment('Message Init Prototypes', indent_level=0)
        self.header_writer.write_newline()
        for m in self.config.messages:
            if m.needs_init:
                self.header_writer.write_function_prototype(Function(f'PROTON_MESSAGE_init_{m.name}', [], 'void'))

        self.header_writer.write_function_prototype(Function('PROTON_MESSAGE_init', [], 'void'))
        self.header_writer.write_newline()

    def generate_message_init_functions(self):
        self.src_writer.write_comment('Message Init Functions', indent_level=0)
        self.src_writer.write_newline()
        for m in self.config.messages:
            if m.needs_init:
                content: str = ''
                for s in m.signals:
                    if s.type == ProtonConfig.Signal.SignalTypes.LIST_STRING:
                        content += self.src_writer.get_for_loop(s.length,
                                                                f'{m.name}.{s.signal}.list[i] = {m.name}.{s.signal}.strings[i];')
                self.src_writer.write_function(Function(f'PROTON_MESSAGE_init_{m.name}', [], 'void'), content)
        self.src_writer.write_newline()

        init_contents = ''
        for m in self.config.messages:
            if m.needs_init:
                if len(init_contents) > 0:
                    init_contents += '\n  '
                init_contents += f'PROTON_MESSAGE_init_{m.name}();'

        self.src_writer.write_function(Function('PROTON_MESSAGE_init', [], 'void'), init_contents)

    def generate(self, name: str):
        generated_filename = f'proton__{name}'

        self.src_writer = CWriter(
            os.path.join(os.getcwd(), "../build/generated/"), f"{generated_filename}.c"
        )

        self.header_writer = CWriter(
            os.path.join(os.getcwd(), "../build/generated/"), f"{generated_filename}.h"
        )

        self.header_writer.write_header_guard_open()
        self.header_writer.write_include('stdint.h')
        self.header_writer.write_include('stdbool.h')
        self.header_writer.write_include('proton.h')
        self.header_writer.write_newline()

        self.src_writer.write_include(generated_filename)
        self.src_writer.write_newline()

        self.generate_signal_enums()
        self.generate_message_struct_typedefs()
        self.generate_extern_message_structs()
        self.generate_message_structs()

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

    generator = ProtonCGenerator(os.path.join(os.getcwd(), "../../config/test.yaml"))
    generator.generate('test')
