# ping_pong example

This example shows two nodes (`pinger` and `ponger`) communicating with each other over a UDP. Each node has a single bundle that they send (`send_ping` and `send_pong`), and each bundle has a single int32 signal (`ping` and `pong`). Effectively, they do the same thing.

`pinger` operates using the lower-level C API using the static signal registry generator, whereas `ponger` represents the C++ node_builder API.

## Building and Running

```sh
cd /path/to/directory/with/this/readme
cmake -B build_ping_pong
cmake --build build_ping_pong --parallel
./build_ping_pong/proton_examples_ping_pong
```
