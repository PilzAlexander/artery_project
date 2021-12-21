# artery/dut
Stand alone configuration for simulation scenarios with a real DUT(device under test).
Connects the simulation with the DUT via a interface component. Communication can go from the simulation to the DUT 
and back.

## Requirements for building

- libzmq3-dev (ZeroMQ)
- cppzmq
- c++17