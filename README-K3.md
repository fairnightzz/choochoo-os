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
git checkout <insert_hash>
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

<insert handle_irq description in kern.h>

<insert assemebly code description for switching to kernel mode from interrupt>

### 3.22 Event Notification (`lib/task_descriptor.h`) (`lib/syscall.h`)

### 3.23 Clock Server (`user/clock-server/server.h`)
Used the linked list structure created previously to create a linked list of buffered clock requests (either `Delay()` or `DelayUntil()`). Then every time our clock server receives a `CLOCK_TICK` event which is a specialized event only sent to the clock server via the notifier task `awaitTick` (our clock counts time by counting the number of these specialized events). Then, we iterate through the buffered requests and reply to each one if the delay has been completed. Based on this implementation, a `DelayUntil()` call that delays to a past timestamp will be replied to on the very next tick.  

This implementation might be inefficient for keeping track of many long lasting events and if our events keep getting queued, so this could be a future optimization point. For now, it works well enough from our performance tests.

### 3.24 Idle Task (`user/init_tasks.h`)

# 4 User Features

## 4.1 Clock Server Client (`user/clock-server/interface.h`) (`user/clock-server/client.h`)

The clock server implements the following three methods as described on the assignment page, with no changes to their signatures:

```c
int Time(int tid)
int Delay(int tid, int ticks)
int DelayUntil(int tid, int ticks)
```

<expand on the implementations a little>


# 5 Client Task Testing (`user/k3.c`)

## 5.1 Output Explanation

## 5.2 Output

# 6 Performance Measurement - Idle Task

## 6.1 Performance Implementation - Kernel Sided


## 6.2 Performance Testing - Statistics

93-95%s
