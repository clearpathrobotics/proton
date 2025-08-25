# Proton

Cloning:

```
mkdir ~/proton_ws/src -p
cd ~/proton_ws/src
git clone git@gitlab.clearpathrobotics.com:research/proton.git
```

## Build steps

### Protonc

```
cd ~/proton_ws/src/proton/protonc
mkdir build
cd build
cmake ..
make
```

### ROS2 package

```
cd ~/proton_ws/
colcon build --symlink-install
```

### Run A300 test

#### Terminal 1

C Node:

```
cd protonc/build/tests/a300
./a300
```

#### Terminal 2

Python ROS bridge:

```
source ~/proton_ws/install/setup.bash
ros2 run proton a300_ros2_bridge
```