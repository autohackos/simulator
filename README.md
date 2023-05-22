# CANBus Simulator

By: Team AutoHackOS

# Credits
- [ICSim](https://github.com/zombieCraig/ICSim)

# Compiling
You will need
- can-utils
- cmake
- Common build utilities like make, g++ etc.

You can get can-utils from github or on Ubuntu you may run the follwoing

```
  sudo apt install can-utils  
```

To compile, you can clone the repository, and build using cmake

```
git clone https://github.com/autohackos/simulator
cd simulator
mkdir build
cd build
cmake ..
make
```

# Testing on a virtual CAN interface
You can run the following commands to setup a virtual can interface

```
  sudo modprobe can
  sudo modprobe vcan
  sudo ip link add dev vcan0 type vcan
  sudo ip link set up vcan0
```

# Contact Us

Please feel free to contact us with suggestions, feedbacks, or contributions.

- Primary Developer: Adhokshaj Mishra <me@adhokshajmishraonline.in>
- Maintenance Team: Security @ AutoHackOS <security@autohackos.com>
