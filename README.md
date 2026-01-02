# Proton

Proton communication protocol.

## Build steps

Build all C/C++ code with:

```
mkdir build
cd build
cmake ..
make
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
