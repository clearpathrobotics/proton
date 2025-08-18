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

# Redistribution and use in source and binary forms, with or without
# modification, is not permitted without the express permission
# of Clearpath Robotics.

from config import ProtonConfig
from typing import List
import os


class Variable:
    def __init__(self, name, type, length=0, capacity=0):
        self.name = name
        self.type = type
        self.length = length
        self.capacity = capacity

class Struct:
    def __init__(self, name, vars):
        self.name = name
        self.vars = vars

class Function:
    def __init__(self, name: str, parameters: List[Variable], ret: str):
        self.name = name
        self.parameters = parameters
        self.ret = ret

class CWriter:
    tab = "  "

    def __init__(self, path: str, name: str):
        self.file_path = path
        self.name = name
        self.header_guard = self.name.upper().replace('.', '_')
        self.file_name = os.path.join(self.file_path, name)
        self.file = open(self.file_name, "w")
        self.initialize_file()

    def write(self, string, indent_level=1):
        self.file.write("{0}{1}\n".format(self.tab * indent_level, string))

    def write_include(self, file: str, path=None):
        if ".h" not in file and ".hpp" not in file and ".hh" not in file:
            file = f"{file}.h"
        if path:
            self.write(f'#include "{os.path.join(path, file)}"', indent_level=0)
        else:
            self.write(f'#include "{file}"', indent_level=0)

    def write_comment(self, comment, indent_level=1):
        self.write("// {0}".format(comment), indent_level)

    def write_newline(self):
        self.write("", 0)

    def write_header_guard_open(self):
        self.write(f'#ifndef PROTONC__{self.header_guard}', indent_level=0)
        self.write(f'#define PROTONC__{self.header_guard}', indent_level=0)
        self.write_newline()

    def write_header_guard_close(self):
        self.write(f'#endif  // PROTONC__{self.header_guard}', indent_level=0)

    def write_variable(self, variable: Variable, indent_level=0):
        var_string = f"{variable.type} {variable.name}"

        if isinstance(variable.length, str) or variable.length > 0:
            var_string += f"[{variable.length}]"
        if isinstance(variable.capacity, str) or variable.capacity > 0:
            var_string += f"[{variable.capacity}]"
        var_string += ";"

        self.write(var_string, indent_level)

    def write_struct(self, struct: Struct, indent_level=1):
        self.write("struct {", indent_level)
        for v in struct.vars:
            if isinstance(v, Variable):
                self.write_variable(v, indent_level + 1)
            elif isinstance(v, Struct):
                self.write_struct(v, indent_level + 1)
        self.write(f"}} {struct.name};", indent_level)

    def write_typedef_struct(self, struct: Struct, indent_level=1):
        self.write(f"typedef struct {struct.name} {{", indent_level)
        for v in struct.vars:
            if isinstance(v, Variable):
                self.write_variable(v, indent_level + 1)
            elif isinstance(v, Struct):
                self.write_struct(v, indent_level + 1)
        self.write(f"}} {struct.name}_t;", indent_level)

    def write_extern_variable(self, variable: Variable, indent_level=0):
        self.write(f'extern {variable.type} {variable.name};', indent_level)

    def write_function_prototype(self, function: Function, indent_level=0):
        parameters = ''
        for p in function.parameters:
            if len(parameters) == 0:
                parameters += f'{p.type} {p.name}'
            else:
                parameters += f', {p.type} {p.name}'
        self.write(f'{function.ret} {function.name}({parameters});', indent_level)

    def write_function_start(self, function: Function, indent_level=0):
        parameters = ''
        for p in function.parameters:
            if len(parameters) == 0:
                parameters += f'{p.type} {p.name}'
            else:
                parameters += f', {p.type} {p.name}'
        self.write(f'{function.ret} {function.name}({parameters})', indent_level)
        self.write('{', indent_level)

    def write_function_end(self, indent_level=0):
        self.write('}', indent_level)
        self.write_newline()

    def write_switch_start(self, variable: str, indent_level=1):
        self.write(f'switch ({variable})', indent_level)
        self.write('{', indent_level)

    def write_switch_end(self, indent_level=1):
        self.write_function_end(indent_level)

    def write_case_start(self, case: str, indent_level=2):
        self.write(f'case {case}:', indent_level)
        self.write('{', indent_level)

    def write_case_default_start(self, indent_level=2):
        self.write('default:', indent_level)
        self.write('{', indent_level)

    def write_case_end(self, indent_level=2):
        self.write('}', indent_level)

    def write_enum(self, name: str, enum: List[str], indent_level=0):
        self.write(f'typedef enum {name} {{', indent_level)
        for e in enum:
            self.write(f'{name.upper() + '__' + e.upper()},', indent_level + 1)
        self.write(f'{name.upper()}_COUNT', indent_level + 1)
        self.write(f'}} {name}_e;', indent_level)

    def write_for_loop_start(self, count, iter_type='int', iter_name='i', start=0, incr=1, indent_level=1):
        self.write(f'for ({iter_type} {iter_name} = {start}; {iter_name} < {count}; {iter_name} += {incr})', indent_level)
        self.write('{', indent_level)

    def write_for_loop_end(self, indent_level=0):
        self.write('}', indent_level)

    def write_if_statement_start(self, condition: str, indent_level=1):
        self.write(f'if ({condition})', indent_level)
        self.write('{', indent_level)

    def write_if_statement_end(self, indent_level=1):
        self.write('}', indent_level)

    def write_define(self, content, indent_level=0):
        self.write(f'#define {content}', indent_level)

    def initialize_file(self):
        self.write(
            """
/**
 * Software License Agreement (proprietary)
 *
 * @copyright Copyright (c) 2025 Clearpath Robotics, Inc., All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, is not permitted without the
 * express permission of Clearpath Robotics.
 *
 * THIS FILE WAS AUTOMATICALLY GENERATED. DO NOT MODIFY.
 */
""",
            0,
        )

    def close_file(self):
        self.file.close()
