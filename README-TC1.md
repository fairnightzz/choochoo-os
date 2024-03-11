<div align="center">

# CS452 - TC1: Train Control 1
### Names: Anish Aggarwal, Zhehai Zhang
### Student IDs: a59aggar, z2252zha
### Date: 03/11/24

</div>

# 1 Overview
This is **Train Control (Part 1)** of CS452 W24 @UWaterloo. We added the ability to control A SINGLE TRAIN and make it stop at any sensor on the track. We hope you like it :)

# 2 Hash Commit & Testing
Run the following on a `linux.student.cs` environment:
```bash
git clone https://git.uwaterloo.ca/z2252zha/choochoo-os.git
cd choochoo-os
git checkout f43e3bdedcd8323880ce8c92d99f6ca1114773b9
make
```

The above has been tested on machine `ubuntu2204-002`.

# 3 Testing
To test the new TC1 features, we added a new command in our A0 UI:

```c
path <train_number> <train_speed> <sensor_destination> <optional: offset>
```
- `train_number` can be any of the trains 2, 37, 55, 58, or 77
- `train_speed` any from 5-14.
- `sensor_destination` will be name of sensor node to direct train to. (i.e. `A10`, `C10`, etc.)
- Optional `offset` meant only for testing and debugging purposes (how many millimeters offset from node) -> defaults to 0.

# 4 Measurements & Calibration

# 5 Train Modeling

# 6 Pathfinding & Routing

# 7 Additional Servers
We abstracted multiple features of A0's train control out into different servers to help with priority and context switches. These are the critical new servers that we have implemented (all with high priority) so they can be responsive to requests.
## 7.1 Train System Server (`user/trainsys-server`)

## 7.2 Sensor Server (`user/sensor-server`)

## 7.3 Switch Server (`user/switch-server`)

## 7.4 Pathfinder Server (`user/pathfinder-server`)

# 8 Kernel Changes
Implemented a `Puts` call for the IO Server (`user/io-server/interface.h`) to help with printing strings and packing messages into one `Send` request. This is to prevent being interrupted in between bytes of a message which especially helps in Marklin when there are certain commands that require sending multiple bytes in a specific order.

# 9 Known Bugs