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

Our calibration data is in `calibration-data/` folder in main directory.

Demo: Many trains on the essential speeds of 5, 8, 11, 14 due to presentation time constraints. Our train order is this:
B1, C10, E5, C3, RV, C13/E7, E1, D1

# 4 Measurements & Calibration (`measurements/`) (`calibration-data/`)
We decided to measure and calibrate 5 trains. The values we decided to measure are the constant velocities at each speed (5-14) and the stopping times at each speed (5-14). 

Our method of taking measurements for velocity is by running the train in the inner loop of the track 20 times. Everytime it triggers a sensor, we figure out the time it took to trigger a sensor to find the velocity (`dist between sensors/time`). We leave this value as micrometers per second to avoid integer division until the very end. This value gets accumulated between each switch where after 20 iterations, we sum all the velocities together to find the average. This way, in the future if we want to be even more specific (velocities within between sensors), we can be varying.


Regardless, it is recommended to use certain trains that are fast (2, 47, 77), because the other trains at this speed are too slow.

For stopping time, we are calculating the time it takes once we trigger sensor C10 to call stop, to perfectly stop on E14. Since this is a straight line distance, it would generalize well to other points. We figure out the stopping time using binary search where we increase the time it takes to call stop if we are undershooting (not triggering sensor E14), and reducing the time it takes to call stop if we are overshooting (triggering sensor E14). This way, we are able to get an accurate stopping time. 

# 5 Train Modeling
Instead of using complicated equations in a track where we do not account for external factors like friction, we decided that a simple model will fit better with our stopping. Since we have velocity, we will want to calculate a stopping distance, which is the distance the train will travel once we hit the stop command. We can calculate this using our stopping time, which is simply `constant velocity/stopping time`. Now that we know our stopping distance, based on any sensor we want to stop at, once we find the optimal path, we can work through the path in reverse to find the sensor we want to stop at, and then calculate the time we need to call stop.

# 6 Pathfinding & Routing (`user/pathfinder-server`)
Our pathfinding algorithm is implemented in `server.h` and requests are sent through the interface from the `promptTask`. It is important to note that our pathfinder-server directly communicates with the sensor, switch, and train control servers in order to find, plan, and execute the path. This means once a path is in execution, we must wait for it to complete before processing another path. 

Our pathfinder starts by waiting for a sensor trigger to determine where the train is on the track. We use this sensor as a source in djikstra's graph search algorithm to try to find a shortest path to the target destination. After finding this path, we switch all the switches correctly on this path and then calculate the waiting sensor, which is the sensor to wait to be triggered before starting a timer to stop the train at the specified location based on stopping distance.

This design is actually quite accurate, but currently there are two large flaws that we may consider fixing in the future:
1. **Off-road Path Errors**: In the time between the train triggers the start sensor and djikstra alg finds the path, it is possible that the train has already moved past a branch and off the path our djikstra found. In this case when we set the switches, it is a possibility that the train never comes back to the source switch and never reaches the destination. To fix this we would need to continue to get sensor information on a train and do rerouting when we realize it is off path.
2. **Short Stop Errors**: In the case where the path calculated has less distance then the stopping time of the current train at a current speed, our pathfinder will error. 

The good point is that none of these errors are crashing errors. Hence, we can retry or path from better locations once we get there.

# 7 Additional Servers
We abstracted multiple features of A0's train control out into different servers to help with priority and context switches. These are the critical new servers that we have implemented (all with high priority) so they can be responsive to requests.

## 7.1 Train System Server (`user/trainsys-server`)
This server controls each train's states where other tasks can send this server the command to set lights / speeds. Furthermore, there is a get speed functionality for other tasks to get the speed of a current train. This is useful for the pathfinder-server.

## 7.2 Sensor Server (`user/sensor-server`)
The sensor server is used to notify subscribers of a sensor that was triggered. We have a notifier which requests for sensor data and tells the server when there are sensors that have been triggered. The sensor will then notify anyone who is listening and waiting for a sensor to be triggered. Some functions that use this service is our render sensor trigger, and pathfinding server.

## 7.3 Switch Server (`user/switch-server`)
The switch server controls the various switches on a track allowing the user to set and get different switch states. We initially initialize the switch state to track_a_init state. One important function is `WaitOnSwitchChange` which will continuously wait until a switch has been updated (switch could be specified).

## 7.4 Pathfinder Server (`user/pathfinder-server`)
This server does the execution and routing to go from A -> B at a specific path-finding speed. Details are explained in Section 6 Pathfinding & Routing.

# 8 Kernel Changes
Implemented a `Puts` call for the IO Server (`user/io-server/interface.h`) to help with printing strings and packing messages into one `Send` request. This is to prevent being interrupted in between bytes of a message which especially helps in Marklin when there are certain commands that require sending multiple bytes in a specific order.

# 9 Known Bugs
- Off-road Path Errors & Short Stop Errors: described in Section 6
- Reverse is blocking. Cannot do other commands / type while train is reversing currently. Fixes error with multiple reverses.
- Our train does not handle dead-ends too well since we rely on sensor activation to tell where a train is and do not do reversing logic. Hence, before a dead-end, we reverse manually and then path again.
