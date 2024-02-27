<div align="center">

# CS452 - K4: I/O Server(s)
### Names: Anish Aggarwal, Zhehai Zhang
### Student IDs: a59aggar, z2252zha
### Date: 02/29/24

</div>

# 1 Overview
This is **Kernel Part 4** of CS452 W24 @UWaterloo. We added to the implementation of the Kernel that allows for an I/O server, and reimplements A0. Hope you like it :)

# 2 Hash Commit & Testing
Run the following on a `linux.student.cs` environment:
```bash
git clone https://git.uwaterloo.ca/z2252zha/choochoo-os.git
cd choochoo-os
git checkout 7cb7d3364a0644ec2a8ade808f311442b632d1a8
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

The user side of the kernel also has a ioserver implementation for the two wrappers:

```c
int Getc(int tid, int channel)
```

```c
int Putc(int tid, int channel, unsigned char ch)
```

The kernel creates a task that runs `initTask()` in `user/initTasks.h`, which starts the nameserver, clockserver console io server and marklin ioserver. Then, it runs the UI Task which does all the specifications in A0. More detail on this in a later section.

## 3.2 Additional Kernel Structure (since K3)

### 3.21 Interrupt Handling (`lib/gic.h`) (`kern/kern.h`) (`kern/enter_modes.S`)

We added a new interrupt id to handle, which is 153.

### 3.22 Event Notification (`lib/task_descriptor.h`) (`lib/syscall.h`)

Two new events.


### 3.23 IO Server (`user/io-server/server.h`)

We have two io servers.

# 4 User Features

## 4.1 IO Server Client (`user/clock-server/interface.h`) (`user/clock-server/client.h`)

The clock server implements the following three methods as described on the assignment page, with no changes to their signatures:

```c
int Getc(int tid, int channel)
int Putc(int tid, int channel, unsigned char ch)
```

Each of these wrappers simply creates a `IORequest` struct which is then sent to the respective io server.

## 5.1 A0 Revisited

We do a bunch in the UI task. We have separated it out into 5 tasks:

* idle performance task
* prompt task
* clock ui task
* trainsys task
* trainsys slave task

