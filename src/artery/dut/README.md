# artery/dut
Stand alone configuration for simulation scenarios with a real DUT(device under test).
Connects the simulation with the DUT via a interface component. Communication can go from the simulation to the DUT 
and back. The code for the interface component is realised in the following repository:

https://github.com/LukasW352435/INFM_HIL_Interface

## Requirements for building

- libzmq3-dev (ZeroMQ)
- cppzmq
- c++17

## Configuration

- scenarios/dut/connectionConfig.xml (for setting the ports, etc.)
- scenarios/dut/connectorsConfig.xml (for configuring the connectors on the interface component)