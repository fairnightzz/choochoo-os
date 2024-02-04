<div align="center">

# CS452 - Kernel Part 2: Message Passing & Nameserver
### Names: Anish Aggarwal, Zhehai Zhang
### Student IDs: a59aggar, z2252zha
### Date: 02/06/24

</div>

# 1 Overview

# 2 Hash Commit & Testing
Run the following on a `linux.student.cs` environment:
```bash
git clone https://git.uwaterloo.ca/z2252zha/choochoo-os.git
cd choochoo-os
git checkout <insert hash>
make
```

The above has been tested on machine `ubuntu2204-010`.

To run the RPS Game Test, head to `user/k2.c` and make sure to have this structure for `startK2Task()`:

```c
void startK2Task()
{
  PRINT("Starting K2 TAsk!");
  NameServerTaskInit();
  RPSTask();
  // k2_performance_measuring();
}
```

To run performance tests on ONE row, have this structure:
```c
void startK2Task()
{
  PRINT("Starting K2 TAsk!");
  // NameServerTaskInit();
  // RPSTask();
  k2_performance_measuring();
}
```

In `user/perf-testing/k2-perf.c`, make sure to have ONE sender receiver pair uncommented in the function `void k2_performance_measuring()`. For example:

```c
void k2_performance_measuring()
{
    // PRINT("Send-first, 4 bytes");
    // sender_tid = Create(11, &send4);
    // receive_tid = Create(12, &receive4);

    // PRINT("Send-first, 64 bytes");
    // sender_tid = Create(13, &send64);
    // receive_tid = Create(14, &receive64);

    // PRINT("Send-first, 256 bytes");
    // sender_tid = Create(15, &send256);
    // receive_tid = Create(16, &receive256);

    // PRINT("Receive-first, 4 bytes");
    // sender_tid = Create(18, &send4);
    // receive_tid = Create(17, &receive4);

    // PRINT("Receive-first, 64 bytes");
    // sender_tid = Create(20, &send64);
    // receive_tid = Create(19, &receive64);

    // PRINT("Receive-first, 256 bytes");
    // sender_tid = Create(22, &send256);
    // receive_tid = Create(21, &receive256);
}
```

The reason for this is due to how the yields work, in order for sender_tid and receive_tid to be synchronized, we need to create the two tasks first so that their tids are initialized, and then have the function quit so that the sender and receiver functions can run properly. (e.g if we try sending or receiving to a tid that is undefined, that is invalid!)

# 3 Kernel Features and Structure
## 3.1 New Kernel Features

The choochoo-os kernel implements the additional three syscalls described on the assignment page with no modifications to their signatures:

```c
int Send(int tid, const char *msg, int msglen, char *reply, int replylen);
```

```c
int Receive(int *tid, char *msg, int msglen);
```

```c
int Reply( int tid, void *reply, int replylen);
```

The user side of the kernel also has a nameserver implementation for the two wrappers:

```c
int RegisterAs(const char *name);
```

```c
int WhoIs(const char *name);
```

The kernel creates a task that runs `startK2Task()` in `user/k2.h`, which starts the nameserver and runs the RPS game test. More on the expected behaviour of RPS test is described in a later section.

## 3.2 Additional Kernel Structure (since K1)

### 3.21 Message Passing (`lib/syscall.S`) (`kern/svc_helpers.c`)

We added the new syscalls to `syscall.S` and added the expected behaviour of Send and Receive described in the kernel specification into `svc_helpers.c`. 

The logic for sending first versus receive first is implemented using states such as `ReceiveWait`, `SendWait`, `ReplyWait`.
It is implemented using this logic: `https://student.cs.uwaterloo.ca/~cs452/W24/lectures/lec06.html`. If the task tid does not exist, then 
the function returns with a negative return value.

### 3.22 User Allocation (`lib/alloc.c`)

### 3.23 Linked List, HashMap, Byte Queue, RPS_STATE (`lib/hashmap.c`) (`linked_list.c`) (`byte_queue.c`) (`kern/kalloc.c`)

# 4 User Features

## 4.1 NameServer (`user/nameserver.c`)

Implemented using the specification in `https://student.cs.uwaterloo.ca/~cs452/W24/assignments/kernel.html`.

The nameserver is first initialized and the tid is saved in the static variable.

`RegisterAs` and `WhoIs` are implemented to wrap around Send by calling the nameserver. We defined two 
structs, `NameServerMessage` is to send requests to the nameserver (register as and who is), and also 
`NameServerResponse` which is the response of the message.

The `NameServerTask()` is an infinite loop that receives messages and parses them to determine what kind of request it is. `RegisterAs()` will add the name to the hashmap which links the tid. This way, 
when `WhoIs()` gets called, we are able to return the correct TID of whoever registered the name.

## 4.2 RPS Server and Client (`user/RPS/*.c`)

### 4.21 RPS Server

### 4.22 RPS Client (`user/RPS/client.c`) (`user/RPS/interface.c`)

The client calls three functions in the `interface.c`:

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

`RPSClientTask1` signs up, plays rock three times, and quits.
`RPSClientTask2` signs up, rock, paper, scissors, rock, paper, scissors, and quits.
`RPSClientTask3` signs up, plays rock and quits. Signs up again, plays paper and then quits.

# 5 RPS Game Test (`user/k2.c`) (`user/RPS/client.c`)

The RPS Game test (`RPSTask()`) begins by starting the RPS Server.

Then, three tests are initialized. 

Test 1 starts by creating RPS client task 1 and client task 2, where the behaviour is expected to be a tie on the first round, task 2 winning the second round, and then task 1 winning the third round. Task 1 quits, while task 2 plays three more moves (that are invalid since task 1 quit already) and then quits.

Test 2 starts by calling task 1, task 2. This should have the same behaviour as test 1.
Then, it calls task 1 twice. Since they are both the same, there should be three ties.
Then, it calls task 2 twice. Since they are both the same, there should be six ties.

Test 3 creates Task 3 three times. We will call the tasks 1, 2, and 3 respectively.

Task 1 will play rock and task 2 will also play rock, giving a tie. They both quit, and then signup again, resulting in them both playing paper, resulting in a tie. 

Task 3 gets created, and hangs on the initial sign up.

# 6 Performance Measurement

For each variation of test, we create two tasks: one that sends, and one that receives. Each task will perform 20 iterations of their respective send/receive on either 4, 64, 256 bytes of a message size. 20 iterations are used to prevent a high variance and a consistent recording in measurements. The timer is created in Send and ends when a reply has been received to time a full SRR.

The following features were permuted and tested: Optimization/No Optimization, icache/dcache/both/none, ReceiverFirst/SenderFirst, 4/64/256 Bytes.

Currently, `-O3` flag is commented out in the Makefile. The `CACHE_ENABLE` variable in `boot.S` is also disabled. To test caching and optimization features, please uncomment those.

## Conclusions

We can see that with regardless of whether optimization is on or whether cache is on, the general behaviour when the message size is increased is that the SRR time in microseconds increases. This makes sense as it takes longer for `memcopy` to copy over the message, making the SRR time longer.

We can also make the observation that Sender first gives a faster SRR time than Receiver first. The reason for this behaviour is because when Receive gets called first, we need to allocate a receive state. When we send first, there is no need to store a receive state as we can process the sender's info immediately. This extra allocate on the receiver side and free on the sender side when receiver gets called first can be the explanation as to why our time is a bit slower.

Another observation is that when optimization is turned on, the time is twice as fast compared to its no optimization counterpart. This makes intuitive sense as compiler optimization speeds up instructions. 

When both the instruction and data caches are turned on, we see a slight improvement in SRR times. This makes sense as repetitive instructions or access to data can be optimized. 

We can see that if we compare solely the instruction cache being turned on versus only the data cache being turned on, instruction cache gives a slightly better improvement. This is because more instructions are being cached in comparison to data caching.