# Proton

Proton communication protocol.

Documentation is available [here](https://docs.clearpathrobotics.com/docs_proton/)

## Requirements

Proton has several external requirements for both code generation and runtime usage

### General
- `cmake`: Build system used for proton
- `pkg-config`: used for finding info about dependencies in cmake
- `build-essential`: Compiler
- `pre-commit`: Used for styling and linting checks before committing
- `clang-format`: Used for styling and linting checks on source files

### C
- `nanopb` (included): lightweight protobuf library and code generator for protonc

### C++
- `libboost-all-dev`: Specifically for `boost/asio.hpp`: an asynchronous serial input/output executor
- `protobuf-compiler`: Used at both build-time (for generating code versions of .proto files), and as a general protobuf library for protoncpp
- `libyaml-cpp-dev`: For parsing bundle config files during the code generation step.
- `libgtest-dev`: For unit testing

### Python
- `protobuf`: pip package for `nanopb` code generator
- `pyyaml`: yaml file parser
- `jinja2`: code generator for signals and bundles

## Build steps

Build all C/C++ code with:

```
mkdir build
cd build
cmake ..
make
```

> **NOTE:** CMake does not check for Python packages. If you run into problems running the `nanopb` code generator, install the above Python dependencies in a `venv`

```
python3 -m venv venv
. venv/bin/activate
pip3 install protobuf pyyaml jinja2
```

## Run Examples

### A300

The A300 example consists of a C MCU node communicating with a C++ PC node over UDP.

#### Terminal 1 - MCU

```
./build/examples/a300/a300_mcu_c
```

#### Terminal 2 - PC

```
./build/examples/a300/a300_pc_cpp
```

### J100

The J100 example consists of a C MCU node communicating with a C++ PC node over Serial.

#### Terminal 1 - socat

To simulate serial transport, create virtual serial ports with `socat`:

```
socat PTY,link=/tmp/ttyPC,raw,echo=0 PTY,link=/tmp/ttyMCU,raw,echo=0
```

#### Terminal 2 - MCU

```
./build/examples/j100/j100_mcu_c
```

#### Terminal 3 - PC

```
./build/examples/j100/j100_pc_cpp
```

### Multi Node

This example consists of 3 nodes communicating with each other over both UDP and Serial. Each node is implemented in C and C++ and can be used interchangeably.

#### Terminal 1 - socat

To simulate serial transport, create virtual serial ports with `socat`:

```
socat PTY,link=/tmp/ttyPC,raw,echo=0 PTY,link=/tmp/ttyMCU,raw,echo=0
```

#### Terminal 2 - Node 1

Node 1 produces `log` bundles and sends them to both Node 2 and 3. It also receives `node_name` bundles from both Node 2 and 3. It communicates with Node 2 over UDP, and with Node 3 over serial.

##### C
```
./build/examples/multi_node/multi_node_1_c
```

##### C++
```
./build/examples/multi_node/multi_node_1_cpp
```

#### Terminal 3 - Node 2

Node 2 produces `node_name` bundles that it sends to Node 1, and also `time` bundles that it sends to Node 3. It consumes `log` bundles It communicates with both nodes over UDP.

##### C
```
./build/examples/multi_node/multi_node_2_c
```

##### C++
```
./build/examples/multi_node/multi_node_2_cpp
```

#### Terminal 4 - Node 3

Node 3 produces `node_name` bundles which it sends to Node 1. It consumes `log` bundles from Node 1, and `time` bundles from Node 2. It communicates with Node 1 over Serial, and Node 2 over UDP.

##### C
```
./build/examples/multi_node/multi_node_3_c
```

##### C++
```
./build/examples/multi_node/multi_node_3_cpp
```

## Contributing

A quick style note for prospective contributors: We use [pre-commit](https://pre-commit.com/) as a framework for style checking any committed code. While these checks run in CI, it's also possible to run them locally on your PC for fewer "lint" commits.

```bash
$ python3 -m venv venv
$ pip install pre-commit
$ pre-commit install # In the top-level dir of the repo
```
`pre-commit` runs checks on all staged files. There may need to be necessary additions to the `.cspell.json` file, make sure that it's staged after any additions for them to be included in a pre-commit spellcheck run.
