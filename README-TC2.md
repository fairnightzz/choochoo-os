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

- `train_number` can be any of the trains 2, 47, 55, 58, or 77

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

## 5.1 Sensor Attribution (`user/trainsys-server`)
For sensor attribution, we set up initial conditions where we know exactly what the first and second sensor a train should hit is at the start of the path. Reversing, and zone switching updates these predicted next sensors depending on the track state. 
Should a sensor outside of this range be detected, it is consider a spurious sensor. Either sensor, will cause an update on the train position allowing for single sensor failures. 

- zoning to prevent crashing
i will talk about this

# 6 Pathfinding & Routing (`user/pathfinder-server`)

Zhehai will talk about 
- track data (adding reverse edges and zones to data)

For finding the shortest path with reversing, we split the pathing into two different stages: `Complex Pathfinding` which handles paths with reversal edges in them, and `Simple Pathfinding` which paths the train through a simple path without any reversals following TC1 logic. Finally, to ensure zoning, we also have something called a `Partial Pathing` which splits up the entire end-to-end pathing that we get back by djikstra into sections that we have reserved before calling `Complex Pathfinding` on it which in turn splits the problem up into `Simple Paths`. To actually decide what path to take, we first use djikstra as to find the shortest distance (non-blocking / all unreserved segments) on the track. If this is not found, a blocking path is computed where the train will wait at the end of the partial segment (reserved zones) for the next zone to become available.


talk about all this. some new points:
- zoning with reservations on parts of the track

For deadlocks, we found two types of deadlocks in our testing which we refer to as `Soft Deadlocks` and `Hard Deadlocks`. A `soft deadlock` occurs when both trains when computing the path find a blocking path. When both trains compute a blocking path, they will both try to take the smallest distance path to the destination and wait if they cannot find a path. While moving through this partial path, an unblocked path for one train may become available although it will be longer. To solve this we have a time out on trains waiting on the next zone in a partial path. Every time this timeout happens, one train will recompute the unblocking path and give way. Since, we unblock one train at a time, we avoid the case where they both recompute the longer path.
A `hard deadlock` occurs when Train A is moving to Train B's position and Train B is not moving. In this case, we do not move the first one out of the way instead we try unblocking through our `soft deadlock` mechanism 3 times before aborting the path.

- changing of speeds to be more accurate


# 7 Additional Servers
We abstracted multiple features of A0's train control out into different servers to help with priority and context switches. These are the critical new servers that we have implemented (all with high priority) so they can be responsive to requests.

## 7.1 Random Destination Server (`user/random-dest-server`)
This server does the random routing to go from A -> B for each train. Once a train reaches its destination, a new path will be computed until the `erp` function to exit the pathfinding is called. We have blacklisted certain sensors in deadzones and too close to other switches. This server works by creating child tasks that run the `PlanPath` function on each train. Once each task responds to the parent task, the parent task sends them new destinations. Once `erp` is called, the trains finish the last tasks they are given, and then gracefully exit.

## 7.2 Zone Server (`user/switch-server`)
The switch server controls the various switches on a track allowing the user to set and get different switch states. We initially initialize the switch state to track_a_init state. One important function is `WaitOnSwitchChange` which will continuously wait until a switch has been updated (switch could be specified).


## 8 Known Bugs
- Derailing At Final Destination. When we stop at a final sensor, there is a small issue where the train could be on a switch depending on how close the sensor is to the switch. While we could try to inch the train a bit in reverse, this is not great in the multiple train case, as well as in the case where a train could be closely behind it depending on the length of the zone.