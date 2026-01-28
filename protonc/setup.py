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

from setuptools import setup

package_name = 'protonc'

setup(
    name=package_name,
    version="0.0.0",
    packages=[package_name],
    entry_points={
        "console_scripts": [
            "protonc_generator=protonc.protonc_generator:main",
        ],
    },
    python_requires=">=3.12",
)
