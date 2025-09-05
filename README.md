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

MCU and PC nodes are implemented in both C and C++ and can be used interchangeably.

#### Terminal 1 - MCU

These nodes simulate the A300 MCU.

##### C

```
cd build/examples/a300
./a300_mcu_c
```

##### C++

```
cd build/examples/a300
./a300_mcu_cpp
```

#### Terminal 2 - PC

These nodes simulate the A300 PC.

##### C

```
cd build/examples/a300
./a300_pc_c
```

##### C++

```
cd build/examples/a300
./a300_pc_cpp
```