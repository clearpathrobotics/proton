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
# @author Tom Wallis (thomas.wallis@rockwellautomation.com)


"""Mapping of protobuf Signal types to C internal types"""
INTERNAL_TYPE_MAP = {
    "double": "double",
    "float": "float",
    "int32": "int32_t",
    "int64": "int64_t",
    "uint32": "uint32_t",
    "uint64": "uint64_t",
    "bool": "bool",
    "string": "char",
    "bytes": "uint8_t",
    "list_double": "double",
    "list_float": "float",
    "list_int32": "int32_t",
    "list_int64": "int64_t",
    "list_uint32": "uint32_t",
    "list_uint64": "uint64_t",
    "list_bool": "bool",
    "list_string": "char",
    "list_bytes": "uint8_t",
}

"""Default values for particular types"""
DEFAULT_VALUE_MAP = {
    "double": "0.0f",
    "float": "0.0f",
    "int32": 0,
    "int64": 0,
    "uint32": 0,
    "uint64": 0,
    "bool": "false",
}