# Proton

Proton communication protocol.

## Build steps

### Protonc Python library

This command will install the `protonc` python module to your local system.

```
pip install -e protonc --break-system-packages
```

You can then call the `protonc` generator from anywhere with

```
protonc_generator -c /path/to/config.yaml -d /path/to/generated/dir -t target_name
```

### CMake

Build all C/C++ code with:

```
mkdir build
cd build
cmake ..
make
```

### Run A300 test

#### Terminal 1

C Node:

```
cd build/protonc/tests/a300
./a300
```

#### Terminal 2

Python ROS bridge:

```
source ~/proton_ws/install/setup.bash
ros2 run proton a300_ros2_bridge
```