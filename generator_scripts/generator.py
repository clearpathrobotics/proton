#!/usr/bin/env python3

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


"""Generator for protonc C code."""

import argparse
from pathlib import Path

from config import validate_ids
from jinja2 import Template
from normalize import (
    set_node_endpoint_address,
    set_signal_properties,
)
import yaml


def load_config(config_path: str) -> dict:
    """
    Read and parse proton configs.

    Args:
        config_path: string path to proton config file
    Raises:
        RuntimeError: if there are issues with the yaml construction or syntax
    Returns:
        yaml config as dict

    """
    try:
        with Path(config_path).open('r', encoding='utf-8') as f:
            config = yaml.safe_load(f)
    except yaml.scanner.ScannerError as s:
        raise RuntimeError(f'YAML file {config_path} is not well formed') from s
    except yaml.constructor.ConstructorError as c:
        raise RuntimeError(
            f'YAML file {config_path} is attempting to create unsafe objects'
        ) from c
    # Check contents are a Dictionary
    if not isinstance(config, dict):
        raise RuntimeError(f'YAML file {config_path} is not a dictionary, is {type(config)}')
    return config


def generate(
    dest_path: Path, dest_file: str, template_file: str, config: dict, name: str, target: str
):
    """
    Generate proton file according to template.

    Args:
        dest_path: path to destination folder
        dest_file: output filename
        template_file: the template file name
        config: proton config
        name: name of proton "project" (for lack of a better term)
        target: name of peer on the proton network

    """
    template_path = Path(__file__).parent / 'resources' / template_file

    with Path(template_path.resolve()).open('r', encoding='utf-8') as f:
        template_content = f.read()

    template = Template(template_content)

    output = template.render(
        name=name, target=target, nodes=config['nodes'], bundles=config['bundles']
    )

    dest_path.mkdir(parents=True, exist_ok=True)
    output_file = dest_path / dest_file

    with Path(output_file).open('w', encoding='utf-8') as f:
        f.write(output)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '-c',
        '--config',
        type=str,
        help='Configuration file path',
    )

    parser.add_argument(
        '-d',
        '--destination',
        type=str,
        help='Destination folder path for generated files',
    )

    parser.add_argument(
        '-t',
        '--target',
        type=str,
        help='Target node for generation',
    )

    args = parser.parse_args()

    config_path = args.config
    dest_path = Path(args.destination)
    target = args.target
    name = Path(config_path).stem

    config = load_config(config_path)

    # validate node config here
    validate_ids(config['bundles'])
    set_node_endpoint_address(config['nodes'])
    set_signal_properties(config['bundles'])

    generate(
        dest_path,
        f'proton__{name}_{target}.h',
        'proton__header_node.h.jinja',
        config,
        name,
        target,
    )
    generate(
        dest_path,
        f'proton__{name}_{target}.c',
        'proton__source_node.c.jinja',
        config,
        name,
        target,
    )


if __name__ == '__main__':
    main()
