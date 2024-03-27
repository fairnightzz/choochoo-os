<div align="center">

# CS452 - TC2: Train Control 2
### Names: Anish Aggarwal, Zhehai Zhang
### Student IDs: a59aggar, z2252zha
### Date: 03/27/24

</div>

# 1 Overview
This is **Train Control (Part 2)** of CS452 W24 @UWaterloo. We added the ability to control MULTIPLE TRAINS and make it stop at most sensor on the track (excluding some edge sensors). We hope you like it :)

# 2 Hash Commit & Testing
Run the following on a `linux.student.cs` environment:
```bash
git clone https://git.uwaterloo.ca/z2252zha/choochoo-os.git
cd choochoo-os
git checkout <fill out>
make
```

The above has been tested on machine `ubuntu2204-002`.

# 3 Testing
To test the new TC2 features, we added a few new commands in our UI:

```c
srp
erp
rvi <train_number>
```

- `train_number` can be any of the trains 2, 37, 55, 58, or 77

`srp` - starts random pathing of two trains we set up (Train 2 and 54).
`erp` - ends random pathing of two trains we set up (Train 2 and 54).
`rvi` - Reverses the train without changing sensor attribution (for initial setup)

Demo: Many trains on the essential speeds of 5, 8, 11, 14 due to presentation time constraints. We are demoing train 2 and 54. 

# 4 Kernel Features and Structure

## 4.1 Kernel Features

The choochoo-os kernel implements a new syscall:

```c
int AwaitTid(int tid);
```

This will wait until the task has exited. This is really useful to figure out when pathing is done to start the next random path.

A new AwaitEvent id called `EVENT_TASK_FINISHED` waits on this condition to happen.

# 5 Train Modeling
do we want to talk about sensor attribution with reverse and zoning here
- next sensor and next next sensor to prevent sensor errors

- zoning to prevent crashing
i will talk about this

# 6 Pathfinding & Routing (`user/pathfinder-server`)

Zhehai will talk about 
- track data (adding reverse edges and zones to data)

talk about all this. some new points:

- shortest path with reversing
- zoning with reservations on parts of the track
- deadlock condition (soft deadlock, hard deadlock to cancel pathing)
- changing of speeds to be more accurate


# 7 Additional Servers
We abstracted multiple features of A0's train control out into different servers to help with priority and context switches. These are the critical new servers that we have implemented (all with high priority) so they can be responsive to requests.

## 7.1 Train System Server (`user/trainsys-server`)
This server controls each train's states where other tasks can send this server the command to set lights / speeds. Furthermore, there is a get speed functionality for other tasks to get the speed of a current train. This is useful for the pathfinder-server.

## 7.2 Pathfinder Server (`user/pathfinder-server`)
This server does the execution and routing to go from A -> B at a specific path-finding speed. Details are explained in Section 6 Pathfinding & Routing.

## 7.3 Random dest Server (`user/pathfinder-server`)
This server does the execution and routing to go from A -> B at a specific path-finding speed. Details are explained in Section 6 Pathfinding & Routing.

## 7.4 Zone Server (`user/switch-server`)
The switch server controls the various switches on a track allowing the user to set and get different switch states. We initially initialize the switch state to track_a_init state. One important function is `WaitOnSwitchChange` which will continuously wait until a switch has been updated (switch could be specified).


## 8 UI Changes (`user/ui`)
The sensor server is used to notify subscribers of a sensor that was triggered. We have a notifier which requests for sensor data and tells the server when there are sensors that have been triggered. The sensor will then notify anyone who is listening and waiting for a sensor to be triggered. Some functions that use this service is our render sensor trigger, and pathfinding server.