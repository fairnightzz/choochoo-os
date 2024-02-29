<div align="center">

# CS452 - K4: I/O Server(s)
### Names: Anish Aggarwal, Zhehai Zhang
### Student IDs: a59aggar, z2252zha
### Date: 02/29/24

</div>

# 1 Overview
This is **Kernel Part 4** of CS452 W24 @UWaterloo. We added to the implementation of the Kernel that allows for an I/O server, and reimplemented A0. Hope you like it :)

# 2 Hash Commit & Testing
Run the following on a `linux.student.cs` environment:
```bash
git clone https://git.uwaterloo.ca/z2252zha/choochoo-os.git
cd choochoo-os
git checkout f43e3bdedcd8323880ce8c92d99f6ca1114773b9
make
```

The above has been tested on machine `ubuntu2204-002`.

To run the client tasks, head to `user/k3.c` and make sure to have this structure for `startK3Task()`:

## 3 RPS Testing
```c
void startK3Task()
{
  PRINT("Starting K3 Task!");
  NameServerTaskInit();
  FirstUserTask();
};
```

# 4 Kernel Features and Structure

## 4.1 Kernel Features

The choochoo-os kernel implements the following syscalls described on the assignment page with no modifications to their signatures:

```c
int Create(int priority, void (*function)());
```

```c
int MyTid();
```

```c
int MyParentTid();
```

```c
void Yield();
```

```c
void Exit();
```

```c
int Send(int tid, const char *msg, int msglen, char *reply, int replylen);
```

```c
int Receive(int *tid, char *msg, int msglen);
```

```c
int Reply( int tid, void *reply, int replylen);
```

```c
int AwaitEvent(int eventType);
```

## 4.2 Kernel Structure
Our kernel program core is written in `kern.c` (header file `kern.h`) where we initialize our kernel which intern initializes all our structures and modular components on the kernel side (i.e. heap memory, i/o, vector exception table). It also provides two other functions: 
```C
 int svc_create(uint32_t priority, void (*entrypoint)());
 ```
```C
void handle_svc();
```

The first of which is the function which our `kern/main.c` file uses to execute the first user task program. The second function is used by our assembly script in `enter_mode.S` when entering kernel mode. This handles the system call made by the user mode. The following are all the core concepts and explanations of the algorithms / data structures used for each:

### 4.21 Memory Management (`kern/addrspace.h`)

We allocate 128KB for the kernel stack, and each task gets an address space of 4KB (+ extra allocated for switch frame, task descriptor, and scheduler in kernel ~ 400B) page to use as a stack.
The maximum amount of tasks is 300 so our kernel does not exceed 128 KB in size.

Thus, we can only create 300 active tasks in the kernel.
If this limit is reached, the kernel prevents the creation of new tasks.
Tasks after exiting will have their space recycled.

At max memory usage of our address space we use 1.2 MB for user address spaces over all 300 tasks + 128 KB kernel space. We can increase this to a lot more, but it is important to note that we currently have a heap memory allocator which holds 30 KB of static memory used for switchframes, task descriptors, and scheduler nodes. This allocator currently limits us to onlu ~33 active tasks. However, for this assignment that is good enough since the request needs only 5 active tasks.

One last thing is based on our task descriptor we can only do tasks up to 128. Meaning if there have already been 128 created tasks up to this point, creating a new task will result in -2. 

### 4.22 Task Descriptor & Storage (`kern/task_descriptor.h`)

The task descriptor used to define each task contains the following fields:

- `tid`: uniquely indentify tasks
- `pTid`: id of parent task
- `switch_frame`: snapshot of registers on kernel side
- `status`: status of the task
- `pri`: the task priority
- `addrspace`: task's address space in usermode
See: `kern/task_descriptor.h` for details.

Task Descriptor metadata is stored in a hash table, which is implemented as an array of 128 task descriptor pointers. Note this is a caveat that does not allow us to make more than 128 tasks over the lifetime of this kernel. To fix this we can implement a hashmap like approach. To do in future assignments.


### 4.23 Task Scheduler (`kern/scheduler.h`)

The task scheduler is implemented using 32 priority levels, where 0 is the highest priority level and 31 is the lowest priority level. As per the kernel specification, since we can have more than one task at any priority, the scheduler is implemented as an array that is 32 long, and each value in the array has a linked list. Hence, it is a list of linked list nodes. It is implemented with round robin scheduling so the task at the head of the list will run first, and the 
task at the tail of the list will run last. When there are no tasks running, 0 as tid gets returned and the kernel stalls.

### 4.24 Heap Memory & Allocation (`kern/kalloc.h`)

Created an allocator using direct addressing and pointer arithmetic to assign blocks of data for switch_frames, task descriptors, and scheduling nodes. This uses the slab allocation algorithm to assign one block of memory for each type of struct of memory we are allocating. It is worth noting that currently we are using equal blocks of memory for each partition of structs with each block being 10 KB.

In future assignments, we might want to change this allocator to have more memory and to have different block sizes per struct allocated.

### 4.25 Context Switch (`kern/switchframe.h` / `kern/enter_modes.S` / `exception_table.S`)

Initialized vector exception table in the start.
When a syscall gets called, we call the function `enter_kernelmode` which first get the switchframe struct in the task descriptor and we save all the registers onto the switchframe.
We call `handle_svc()` to handle the syscall.

Once the svc has been handled, we call `enter_usermode`. This restores the registers back to what they were before and we return from the exception.

Switchframe contains the entrypoint (user task function), the stack pointer of the user task, and also the `USER_TASK_EXIT` which will get called when the function returns. This is useful because if the user forgets to add an `Exit()` syscall, we will call it.

### 4.21 Message Passing (`lib/syscall.S`) (`kern/svc_helpers.h`)

We added the new syscalls to `syscall.S` and added the expected behaviour of Send and Receive described in the kernel specification into `svc_helpers.c`. 

The logic for sending first versus receive first is implemented using states such as `ReceiveWait`, `SendWait`, `ReplyWait`.
It is implemented using this logic: `https://student.cs.uwaterloo.ca/~cs452/W24/lectures/lec06.html`. If the task tid does not exist, then 
the function returns with a negative return value.

### 4.22 User Allocation (`lib/alloc.h`)
We want users to also be able to allocate memory for structures; however, this cannot overlap with the kernel structures. Hence, we made a library (`lib/alloc.h`) that wraps `kern/kalloc.h` and still uses `slab allocation` but as specified user memmory structures. This is done by making the `UserAllocationType` enum a subset of `AllocationType` enum. Hence, allowing for reusability of our slab allocator but also for distinction between user program allocated memory and kernel allocated memory. One important note is that for our fixed size slabs, user programs still need to initialize the block size of each struct before calling `alloc` / `free`. 

### 4.23 Linked List (`lib/linked_list.h`)
We made a linked list structure because we decided to do hashing with chaining to ensure we do not run into any hash collisions and have to resize our hashmap. Hence, currently this linked list structure is only used to help implement the hashmap described in the next section; however, it was made in a way that allows it to be independent (with iterators as well). Thus, it could be used more in future assignments.

### 4.24 HashMap (`lib/hashmap.h`)
We made a hashmap for two reasons:
1. Nameserver needs to map names to tids
2. Want to map player tids to player objects for RPS. Could have used an array to do this, but went with hashmap approach for readability and to battle test our hash map implementation; since we know we will need this in future.

The HashMap structure allows users to create a hashmap with `char *` keys and `void *` values. The `char *` keys are string copied to ensure no user changes and for stability. Note due to our copy, keys should be of length <= 20. The `void *` values allow for generic values meaning anything can be stored. This is useful in our case as in some place we may want integers (1.) and in some places we want object pointers(2.).

The HashMap utilizes hashing with chaining with a 67 buckets as well as a simple hash function that uses prime 31 and the ascii values.

### 4.25 Return of Circular Buffer: Byte Queue (`lib/byte_queue.h`)
The byte queue is needed to help facillitate our `Send-Receive-Reply` functionality. When a sender sends first, they add their tid to a circular buffer queue on the receiver's task descriptor so when the receiver calls `Receive()` it can look if a sender is already in its queue. Through testing, we have verified that this buffer does not overflow.

### 4.26 Interrupt Handling (`lib/gic.h`) (`kern/kern.h`) (`kern/enter_modes.S`)

The library `gic.h` allows for the targeting and enabling of specific interrupt ids. Currently, this is done in kernel initialization to target and enable interrupt id `97` which is used for handling the clock interrupts needed. Furthermore, this library also gives functionality allowing for the reading and writing of the interrupt acknowledgement register which is used when handling the interrupt. 

`enter_kernelmode_irq` in `kern/enter_modes.S` performs the same as `enter_kernelmode`, except
we branch to a different function, that being `handle_irq()`. One noticeable difference is that we are also resetting the kernel stackpointer to avoid variables that are allocated on the stack from running kernel memory out.

`handle_irq()` in `kern.h` gets the current task and makes sure to stop the idle timer if we just left the idle task. We then read the iar to get the interrupt id, where if it's `97`, then we unblock tasks that are waiting on the clock tick event. We then write the iar back and then yield to a task.

We added a new interrupt id to handle, which is 153.
We handle an interrupt depending on their type.

For `MARKLIN`, we only check for CTS or RX interrupts and then unblock the respective event. We only check CTS because we noticed that CTS is only high after TX is high, so there's no point in setting up a state machine if we know that CTS will be high in the end and everything should be good to send. It's also important to note that we delaying the marklin send so that we will not be overloading the buffer. We also disabled all FIFOs.

For `CONSOLE`, we only check for TX and RX interrupts. The reason for not checking CTS is because the console is incredibly responsive and fast so checking using state machines will only delay console output. This change was decided after rigorous testing.

### 4.27 Event Notification (`lib/task_descriptor.h`) (`lib/syscall.h`)

We added an `eventWaitType` so that tasks can be blocked on a certain type of event id. We also 
added the `AwaitEvent(int eventType)` syscall.

We added 4 new events, that being:
1. `EVENT_MARKLIN_SEND`
2. `EVENT_MARKLIN_RECEIVE`
3. `EVENT_CONSOLE_SEND`
4. `EVENT_CONSOLE_RECEIVE`

# User Features

## 4.1 NameServer (`user/nameserver.h`)

Implemented using the specification in `https://student.cs.uwaterloo.ca/~cs452/W24/assignments/kernel.html`.

The nameserver is first initialized and the tid is saved in the static variable.

`RegisterAs` and `WhoIs` are implemented to wrap around Send by calling the nameserver. We defined two 
structs, `NameServerMessage` is to send requests to the nameserver (register as and who is), and also 
`NameServerResponse` which is the response of the message.

The `NameServerTask()` is an infinite loop that receives messages and parses them to determine what kind of request it is. `RegisterAs()` will add the name to the hashmap which links the tid. This way, 
when `WhoIs()` gets called, we are able to return the correct TID of whoever registered the name.

## 4.2 RPS Server and Client (`user/RPS/*.h`)

### 4.21 RPS Server (`user/RPS/server.h`)

The RPS Server can be started by running the function `RPSServer()` in `user/RPS/server.h`. This server holds a hashmap that maps player tids to games. Each game instance will have its pointer stored twice in the map (one under each player).

Then this server will constantly loop calling `Receive()` to receive a request from some client. These requests have one of three types: `RPS_SIGNUP`, `RPS_PLAY`, and `RPS_QUIT` which calls the appropriate handlers. These handlers then use and alter the RPSGameState and based on the game state, reply to the appropriate tasks. When a player plays, the corresponding game is updated with their move. If both players have moves recorded, the game is evaluated and the results are replied to each player. Games are removed from the hash map when they are quit.

The game logic follows normal logic and for more specifics you can see our implmentation in `user/RPS/server.c`.

### 4.22 RPS Client (`user/RPS/client.h`) (`user/RPS/interface.h`)

The client calls three functions in the `interface.h`:

```c
int Signup(int server);
```

```c
RPS Result Play(int server, RPSMove move);
```

```c
int Quit(int server);
```

Note that `server` is a tid of the RPS server, which is generally first located using the nameserver's `WhoIs` function. Then, 
users can signup to play a game, in which `-1` is returned if an error occurred. Users can then play, where they can pass in a `RPSMove` which consists of Rock, paper, scissors. The `RPSResult` returns a win, loss, draw, or if the opponent quit. If an opponent quits before the user is still done playing, they can still send moves, just that the moves won't do anything and they will always get back a message saying that the opponent quit.

## 4.1 Clock Server Client (`user/clock-server/interface.h`) (`user/clock-server/client.h`)

The clock server implements the following three methods as described on the assignment page, with no changes to their signatures:

```c
int Time(int tid)
int Delay(int tid, int ticks)
int DelayUntil(int tid, int ticks)
```

Each of these wrappers simply creates a `ClockRequest` struct which is then sent to the clockserver.
`DelayUntil` and `Delay`'s logic are handled by the server.

## 4.2 IO Server(s) - Marklin & Console

We have two IO Server(s): one for Marklin & one for Console. Each of these servers have two notifiers (two receive two event interrupts per server - one for clear to send event and one for a receive event) and each server handles an IORequest that is divided into one of 4 requests:
1. PUTC Request: a user task wants to put a character on this line. This request also holds data.
2. GETC Request: a user task wants to get a character on this line. This request's reply holds data.
3. RECEIVE IO Request: sent from notifier to server on receive event as result of interrupt.
4. SEND IO Request: sent from notifier on clear to send event as a result of interrupt.

Using these 4 requests, each server is able to properly write / read to the specified line.

## 4.3 IO Interface (`user/io-server/interface.h`)

The io server implements the following two methods as described on the assignment page, with no changes to their signatures:

```c
int Getc(int tid, int channel)
int Putc(int tid, int channel, unsigned char ch)
```

Each of these wrappers simply creates a `IORequest` struct which is then sent to the respective io server.

## 5 A0 Revisited

We do a bunch in the UI task. We have separated it out into 5 tasks:

### 5.1 idle performance task (`user/ui/interface.c`)

This task renders the idle task percentage onto the console every 500 ticks.

### 5.2 prompt task (`user/ui/helper_tasks.h`)

This task renders the input and handles all console commands between the UI and the train system module. It also executes the train commands entered sending them to the marklin system via interactions with the train system.

### 5.3 clock ui task (`user/ui/helper_tasks.h`)

The is task is just in charge of rendering the clock ui. It calls time and delays every 10 ticks to accurately get every 10th of a second without losing time.

### 5.4 trainsys task (`user/ui/helper_tasks.h`)

This task is one that handles reading from the sensors periodically. It then sends these sensors to the UI to update it and get all the banks in order.

### 5.5 rendering to console (`user/ui/render.h`)

This file is in charge of all the rendering semantics. In order to prevent `Putc` from getting put out of order, we place all the chars in a buffer to ensure the order is correct. The next putc will be the pop of the buffer.
An important thing to note is that the initial rendering is done with blocking uart print. This is because there are many characters to output in the start. Afterwards, everything is exclusively non blocking using putc.

# How our Kernel Runs
The kernel creates a task that runs `initTask()` in `user/initTasks.h`, which starts the nameserver, clockserver console io server and marklin ioserver. Then, it runs the UI Task which does all the specifications in A0.