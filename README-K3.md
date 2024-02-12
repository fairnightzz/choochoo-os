r<div align="center">

# CS452 - K3: Event Notification & Clock Server
### Names: Anish Aggarwal, Zhehai Zhang
### Student IDs: a59aggar, z2252zha
### Date: 02/15/24

</div>

# 1 Overview
This is **Kernel Part 3** of CS452 W24 @UWaterloo. We added to the implementation of the Kernel that allows for event notification, clockserver functionality, and an idle task. Hope you like it :)

# 2 Hash Commit & Testing
Run the following on a `linux.student.cs` environment:
```bash
git clone https://git.uwaterloo.ca/z2252zha/choochoo-os.git
cd choochoo-os
git checkout 3074f90affcd92cc131e7caa3838414ff73e664a
make
```

The above has been tested on machine `ubuntu2204-002`.

To run the client tasks, head to `user/k3.c` and make sure to have this structure for `startK3Task()`:

## 2.1 RPS Testing
```c
void startK3Task()
{
  PRINT("Starting K3 Task!");
  NameServerTaskInit();
  FirstUserTask();
};
```

# 3 Kernel Features and Structure
## 3.1 New Kernel Features

The choochoo-os kernel implements the additional syscall described on the assignment page with no modifications to their signatures:

```c
int AwaitEvent(int eventType);
```

The user side of the kernel also has a clockserver implementation for the three wrappers:

```c
int Time(int tid);
```

```c
int Delay(int tid, int ticks);
```

```c
int DelayUntil(int tid, int ticks);
```

The kernel creates a task that runs `startK3Task()` in `user/k3.h`, which starts the clockserver and runs the client tests. More on the expected behaviour of these tests is described in a later section.


## 3.2 Additional Kernel Structure (since K1)

### 3.21 Interrupt Handling (`lib/gic.h`) (`kern/kern.h`) (`kern/enter_modes.S`)

The library `gic.h` allows for the targeting and enabling of specific interrupt ids. Currently, this is done in kernel initialization to target and enable interrupt id `97` which is used for handling the clock interrupts needed. Furthermore, this library also gives functionality allowing for the reading and writing of the interrupt acknowledgement register which is used when handling the interrupt. 

`enter_kernelmode_irq` in `kern/enter_modes.S` performs the same as `enter_kernelmode`, except
we branch to a different function, that being `handle_irq()`. One noticeable difference is that we are also resetting the kernel stackpointer to avoid variables that are allocated on the stack from running kernel memory out.

`handle_irq()` in `kern.h` gets the current task and makes sure to stop the idle timer if we just left the idle task. We then read the iar to get the interrupt id, where if it's `97`, then we unblock tasks that are waiting on the clock tick event. We then write the iar back and then yield to a task.

### 3.22 Event Notification (`lib/task_descriptor.h`) (`lib/syscall.h`)

We added an `eventWaitType` so that tasks can be blocked on a certain type of event id. We also 
added the `AwaitEvent(int eventType)` syscall.

### 3.23 Clock Server (`user/clock-server/server.h`)
Used the linked list structure created previously to create a linked list of buffered clock requests (either `Delay()` or `DelayUntil()`). Then every time our clock server receives a `CLOCK_TICK` event which is a specialized event only sent to the clock server via the notifier task `awaitTick` (our clock counts time by counting the number of these specialized events). Then, we iterate through the buffered requests and reply to each one if the delay has been completed. Based on this implementation, a `DelayUntil()` call that delays to a past timestamp will be replied to on the very next tick.  

This implementation might be inefficient for keeping track of many long lasting events and if our events keep getting queued, so this could be a future optimization point. For now, it works well enough from our performance tests.

### 3.24 Idle Task (`user/init_tasks.h`) (`kern/idle-perh.h`)

We have an `idleTask()` function which is an infinite while loop that waits for an interrupt. This puts the kernel in a low power state. Since we also need to track performance (how much time the idle task has taken), we also have a `idlePerformanceTask()` function which prints the percentage of the idle task time in comparison to everything else.

To measure the time, `idle_timer_stop_logic` is called at the start of syscall and interrupt code which stops the idle task timer if we just came from the idle task. Similarily, `idle_timer_start_logic` will be called at the end to start the timer if we are entering the idle task.

# 4 User Features

## 4.1 Clock Server Client (`user/clock-server/interface.h`) (`user/clock-server/client.h`)

The clock server implements the following three methods as described on the assignment page, with no changes to their signatures:

```c
int Time(int tid)
int Delay(int tid, int ticks)
int DelayUntil(int tid, int ticks)
```

Each of these wrappers simply creates a `ClockRequest` struct which is then sent to the clockserver.
`DelayUntil` and `Delay`'s logic are handled by the server.


# 5 Client Task Testing (`user/k3.c`)

`startK3Task()` starts by initializing the nameserver. It then runs `FirstUserTask()`, 
in which it creates the clock server and the 4 client tasks as requested in the assignment webpage.

## 5.1 Output Explanation

We can see that for the first client task (delay interval 10, numDelays 20), we see that the task gets initially called at tick 3. This results in the ticks being additions of 10, that being 13, 23, 33, 43, ... 203.

We can also see the second task (delay interval 23, numDelays 9) which gets called at tick 3, resulting in additions of 23, that being 26, 49, 72, 95, ... 210.

We can also see the third task (delay interval 33, numDelays 6)
which gets called at tick 3. This results in the ticks being additions of 33, giving 36, 69, 102, 135, 168, 201.

We can also see the third task (delay interval 71, numDelays 3) which gets called at tick 3, resulting in additions of 71, giving:
74, 145, 216.

One interesting observation is that we don't lose ticks. This is because our server is set up to 
send the response to the syscall the moment the tick value is equal to the delay we want. Another 
observation is that all of our tasks start calling at tick 3. This can be attributed to the fact that after the clock server is created, the time to create each client task, have them send a request back to the server (parent tid task), and then have the server respond back with the correct parameters would account for a delay of 3 ticks. This is fine because this is an initial setup time as we can see that in our logging output, every tick afterwards is correct.


## 5.2 Output
```
Starting K3 Task!
Tid: 7, Delay Interval: 10, Loop Iteration: 1, Tick: 13
Tid: 7, Delay Interval: 10, Loop Iteration: 2, Tick: 23
Tid: 8, Delay Interval: 23, Loop Iteration: 1, Tick: 26
Tid: 7, Delay Interval: 10, Loop Iteration: 3, Tick: 33
Tid: 9, Delay Interval: 33, Loop Iteration: 1, Tick: 36
Tid: 7, Delay Interval: 10, Loop Iteration: 4, Tick: 43
Tid: 8, Delay Interval: 23, Loop Iteration: 2, Tick: 49
Tid: 7, Delay Interval: 10, Loop Iteration: 5, Tick: 53
Tid: 7, Delay Interval: 10, Loop Iteration: 6, Tick: 63
Tid: 9, Delay Interval: 33, Loop Iteration: 2, Tick: 69
Tid: 8, Delay Interval: 23, Loop Iteration: 3, Tick: 72
Tid: 7, Delay Interval: 10, Loop Iteration: 7, Tick: 73
Tid: 10, Delay Interval: 71, Loop Iteration: 1, Tick: 74
Tid: 7, Delay Interval: 10, Loop Iteration: 8, Tick: 83
Tid: 7, Delay Interval: 10, Loop Iteration: 9, Tick: 93
Tid: 8, Delay Interval: 23, Loop Iteration: 4, Tick: 95
Tid: 9, Delay Interval: 33, Loop Iteration: 3, Tick: 102
Tid: 7, Delay Interval: 10, Loop Iteration: 10, Tick: 103
Tid: 7, Delay Interval: 10, Loop Iteration: 11, Tick: 113
Tid: 8, Delay Interval: 23, Loop Iteration: 5, Tick: 118
Tid: 7, Delay Interval: 10, Loop Iteration: 12, Tick: 123
Tid: 7, Delay Interval: 10, Loop Iteration: 13, Tick: 133
Tid: 9, Delay Interval: 33, Loop Iteration: 4, Tick: 135
Tid: 8, Delay Interval: 23, Loop Iteration: 6, Tick: 141
Tid: 7, Delay Interval: 10, Loop Iteration: 14, Tick: 143
Tid: 10, Delay Interval: 71, Loop Iteration: 2, Tick: 145
Tid: 7, Delay Interval: 10, Loop Iteration: 15, Tick: 153
Tid: 7, Delay Interval: 10, Loop Iteration: 16, Tick: 163
Tid: 8, Delay Interval: 23, Loop Iteration: 7, Tick: 164
Tid: 9, Delay Interval: 33, Loop Iteration: 5, Tick: 168
Tid: 7, Delay Interval: 10, Loop Iteration: 17, Tick: 173
Tid: 7, Delay Interval: 10, Loop Iteration: 18, Tick: 183
Tid: 8, Delay Interval: 23, Loop Iteration: 8, Tick: 187
Tid: 7, Delay Interval: 10, Loop Iteration: 19, Tick: 193
Tid: 9, Delay Interval: 33, Loop Iteration: 6, Tick: 201
Tid: 7, Delay Interval: 10, Loop Iteration: 20, Tick: 203
Tid: 8, Delay Interval: 23, Loop Iteration: 9, Tick: 210
Tid: 10, Delay Interval: 71, Loop Iteration: 3, Tick: 216
Idle Task Execution: 93 percent
Idle Task Execution: 94 percent
Idle Task Execution: 94 percent
Idle Task Execution: 94 percent
Idle Task Execution: 95 percent
Idle Task Execution: 95 percent
Idle Task Execution: 95 percent
Idle Task Execution: 95 percent
Idle Task Execution: 95 percent
Idle Task Execution: 95 percent
Idle Task Execution: 95 percent
Idle Task Execution: 95 percent
```

# 6 Performance Measurement - Idle Task
We can see that the idle task has been executing for `93%` of the 
time. We can also see that when there are no more tasks left, 
the percentage increases as the idle task runs (in conjunction to the task that outputs the performance). As a result, we can see that the idle task runs about `93-95%` of the time.

## 6.1 Performance Implementation - Kernel Sided

The performance of the idle task had to be calculated on the kernel side. What happens is we initialize
the idle task and record its id. This is in `kern/idle-perf.c`. It records the start time since we have 
started the OS. To calculate the performance, we record the time it takes for the idle timer to run, so we can get a percentage as `total_idle_time / total_time`.

Another notable portion is that we stop the timer for the idle task at the last possible second 
(right when we enter the syscall/interrupt), and also start the timer right before we enter user mode (`kern.c`). 
This is because we want to minimize the time of the kernel methods as that does not include the idle task time.


