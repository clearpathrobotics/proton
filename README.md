# Proton

[![Proton CI](https://github.com/clearpathrobotics/proton/actions/workflows/ci.yml/badge.svg)](https://github.com/clearpathrobotics/proton/actions/workflows/ci.yml)

Proton communication protocol.

Documentation is available [here](https://docs.clearpathrobotics.com/docs_proton/)

## Requirements

Proton has several external requirements for building, code generation, and optional runtime features

### General
- `cmake`: Build system used for proton
- `pkg-config`: used for finding info about dependencies in cmake
- `build-essential`: Compiler
- `pre-commit`: Used for styling and linting checks before committing
- `clang-format`: Used for styling and linting checks on source files

### Protobuf message generation (PROTON_GENERATE_PROTOS)
- `git`: For downloading an external dependency (nanopb)
- `nanopb` (included): lightweight protobuf library and code generator for protobuf messages
- `python3-protobuf`: pip package for `nanopb` code generator
- `protobuf-compiler`: Used for generating code versions of .proto files
- `libabsl-dev`: May be required for `protobuf-compiler` depending on your distro's protobuf version. Required for protobuf >= v23

### proton_core static registry generator (generator_scripts/generator.py)
- `python3-yaml`: yaml file parser
- `python3-jinja2`: code generator for signals and bundles

> **NOTE:** CMake does not check for Python packages. If you run into problems running the `nanopb` code generator or static registry generator, install the following Python dependencies in a `venv`

```
python3 -m venv venv
. venv/bin/activate
pip3 install protobuf pyyaml jinja2
```

## C++ Optional Features

### PROTON_ENABLE_ALLOC
Enables internal usage of allocating STL types (`std::vector<T>`, `std::function`, etc) and RTTI.

### PROTON_NODE_BUILDER
Enables runtime creation of the proton node and registry, for using proton without the static registry generator

Requires:
  - `PROTON_ENABLE_ALLOC=ON`

### PROTON_NODE_BUILDER_YAML_PARSER
Enables parsing of yaml files and structs for `PROTON_NODE_BUILDER`
Requires:
  - `PROTON_NODE_BUILDER=ON`
  - `libyaml-cpp-dev`

### PROTON_NODE_BUILDER_JSON_PARSER
Enables parsing of json files and structs for `PROTON_NODE_BUILDER`
Requires:
  - `PROTON_NODE_BUILDER=ON`
  - `nlohmann-json3-dev`

### Optional Features Based on C++ STD >= 20
Compiling with `-DCMAKE_CXX_STANDARD=20` or higher will enable a `std::span` API in the C++ libraries.

## Unit Testing (PROTON_BUILD_TESTS)
Compile proton with the following feature flags:

```
cmake -B build_test \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_STANDARD=20 \
  -DCMAKE_CXX_STANDARD_REQUIRED=ON \
  -DPROTON_BUILD_TESTS=ON \
  -DPROTON_ENABLE_TEST_COVERAGE=ON \
  -DPROTON_ENABLE_ALLOC=ON \
  -DPROTON_NODE_BUILDER=ON \
  -DPROTON_NODE_BUILDER_YAML_PARSER=ON \
  -DPROTON_NODE_BUILDER_JSON_PARSER=ON

cmake --build build_test --parallel

ctest --test-dir build_test --output-on-failure
```

Requires:
 - `libgtest-dev`
 - all dependencies for enabled feature flags

### Optional code coverage (PROTON_ENABLE_TEST_COVERAGE)
Enables optional code coverage metrics

Requires:
  - `lcov`
  - `PROTON_BUILD_TESTS=ON`

## Build steps

Build all C/C++ code with:

```
cmake -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_STANDARD=20 \
  -DCMAKE_CXX_STANDARD_REQUIRED=ON \
  -DPROTON_BUILD_TESTS=OFF \
  -DPROTON_ENABLE_ALLOC=ON \
  -DPROTON_NODE_BUILDER=ON \
  -DPROTON_NODE_BUILDER_YAML_PARSER=ON \
  -DPROTON_NODE_BUILDER_JSON_PARSER=ON

  cmake --build build --parallel
```

## Installation

Proton can be installed with `-DPROTON_INSTALL=ON` with any set of feature flags. Installation is done through the normal cmake process:

```
cmake --install build --prefix /path/to/where/you/want/proton/installed
```

## Contributing

A quick style note for prospective contributors: We use [pre-commit](https://pre-commit.com/) as a framework for style checking any committed code. While these checks run in CI, it's also possible to run them locally on your PC for fewer "lint" commits.

```bash
$ python3 -m venv venv
$ pip install pre-commit
$ pre-commit install # In the top-level dir of the repo
```

`pre-commit` runs checks on all staged files. There may need to be necessary additions to the `.cspell.json` file, make sure that it's staged after any additions for them to be included in a pre-commit spellcheck run.
