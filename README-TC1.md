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

# 4 Measurements & Calibration (`measurements/`) (`calibration-data/`)
We decided to measure and calibrate 5 trains. The values we decided to measure are the constant velocities at each speed (5-14) and the stopping times at each speed (5-14). 

Our method of taking measurements for velocity is by running the train in the inner loop of the track 20 times. Everytime it triggers a sensor, we figure out the time it took to trigger a sensor to find the velocity (`dist between sensors/time`). We leave this value as micrometers per second to avoid integer division until the very end. This value gets accumulated between each switch where after 20 iterations, we sum all the velocities together to find the average. This way, in the future if we want to be even more specific (velocities within between sensors), we can be varying.


Regardless, it is recommended to use certain trains that are fast (2, 47, 77), because the other trains at this speed are too slow.

For stopping time, we are calculating the time it takes once we trigger sensor C10 to call stop, to perfectly stop on E14. Since this is a straight line distance, it would generalize well to other points. We figure out the stopping time using binary search where we increase the time it takes to call stop if we are undershooting (not triggering sensor E14), and reducing the time it takes to call stop if we are overshooting (triggering sensor E14). This way, we are able to get an accurate stopping time. 

# 5 Train Modeling

Instead of using complicated equations in a track where we do not account for external factors like friction, we decided that a simple model will fit better with our stopping. Since we have velocity, we will want to calculate a stopping distance, which is the distance the train will travel once we hit the stop command. We can calculate this using our stopping time, which is simply `constant velocity/stopping time`. Now that we know our stopping distance, based on any sensor we want to stop at, once we find the optimal path, we can work through the path in reverse to find the sensor we want to stop at, and then calculate the time we need to call stop.

# 6 Pathfinding & Routing

# 7 Additional Servers
We abstracted multiple features of A0's train control out into different servers to help with priority and context switches. These are the critical new servers that we have implemented (all with high priority) so they can be responsive to requests.
## 7.1 Train System Server (`user/trainsys-server`)

## 7.2 Sensor Server (`user/sensor-server`)

The sensor server is used to notify subscribers of a sensor that was triggered. We have a notifier which requests for sensor data and tells the server when there are sensors that have been triggered. The sensor will then notify anyone who is listening and waiting for a sensor to be triggered. Some functions that use this service is our render sensor trigger, and pathfinding server.

## 7.3 Switch Server (`user/switch-server`)

## 7.4 Pathfinder Server (`user/pathfinder-server`)

# 8 Kernel Changes
Implemented a `Puts` call for the IO Server (`user/io-server/interface.h`) to help with printing strings and packing messages into one `Send` request. This is to prevent being interrupted in between bytes of a message which especially helps in Marklin when there are certain commands that require sending multiple bytes in a specific order.

# 9 Known Bugs