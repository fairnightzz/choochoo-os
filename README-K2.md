r<div align="center">

# CS452 - K2: Message Passing & Nameserver
### Names: Anish Aggarwal, Zhehai Zhang
### Student IDs: a59aggar, z2252zha
### Date: 02/06/24

</div>

# 1 Overview
This is **Kernel Part 2** of CS452 W24 @UWaterloo. We added to the implementation of the Kernel that allows for message passing, nameserver, and RPS. Hope you like it :)

# 2 Hash Commit & Testing
Run the following on a `linux.student.cs` environment:
```bash
git clone https://git.uwaterloo.ca/z2252zha/choochoo-os.git
cd choochoo-os
git checkout 07d9c4eaed8fe2d7ae8b7b47243c0dc4422746e5
make
```

The above has been tested on machine `ubuntu2204-002`.

To run the RPS Game Test, head to `user/k2.c` and make sure to have this structure for `startK2Task()`:

## 2.1 RPS Testing
```c
void startK2Task()
{
  PRINT("Starting K2 TAsk!");
  NameServerTaskInit();
  RPSTask();
  // k2_performance_measuring();
}
```

## 2.2 Performance Testing
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

### 3.21 Message Passing (`lib/syscall.S`) (`kern/svc_helpers.h`)

We added the new syscalls to `syscall.S` and added the expected behaviour of Send and Receive described in the kernel specification into `svc_helpers.c`. 

The logic for sending first versus receive first is implemented using states such as `ReceiveWait`, `SendWait`, `ReplyWait`.
It is implemented using this logic: `https://student.cs.uwaterloo.ca/~cs452/W24/lectures/lec06.html`. If the task tid does not exist, then 
the function returns with a negative return value.

### 3.22 User Allocation (`lib/alloc.h`)
We want users to also be able to allocate memory for structures; however, this cannot overlap with the kernel structures. Hence, we made a library (`lib/alloc.h`) that wraps `kern/kalloc.h` and still uses `slab allocation` but as specified user memmory structures. This is done by making the `UserAllocationType` enum a subset of `AllocationType` enum. Hence, allowing for reusability of our slab allocator but also for distinction between user program allocated memory and kernel allocated memory. One important note is that for our fixed size slabs, user programs still need to initialize the block size of each struct before calling `alloc` / `free`. 

### 3.23 Linked List (`lib/linked_list.h`)
We made a linked list structure because we decided to do hashing with chaining to ensure we do not run into any hash collisions and have to resize our hashmap. Hence, currently this linked list structure is only used to help implement the hashmap described in the next section; however, it was made in a way that allows it to be independent (with iterators as well). Thus, it could be used more in future assignments.

### 3.24 HashMap (`lib/hashmap.h`)
We made a hashmap for two reasons:
1. Nameserver needs to map names to tids
2. Want to map player tids to player objects for RPS. Could have used an array to do this, but went with hashmap approach for readability and to battle test our hash map implementation; since we know we will need this in future.

The HashMap structure allows users to create a hashmap with `char *` keys and `void *` values. The `char *` keys are string copied to ensure no user changes and for stability. Note due to our copy, keys should be of length <= 20. The `void *` values allow for generic values meaning anything can be stored. This is useful in our case as in some place we may want integers (1.) and in some places we want object pointers(2.).

The HashMap utilizes hashing with chaining with a 67 buckets as well as a simple hash function that uses prime 31 and the ascii values.

### 3.25 Return of Circular Buffer: Byte Queue (`lib/byte_queue.h`)
The byte queue is needed to help facillitate our `Send-Receive-Reply` functionality. When a sender sends first, they add their tid to a circular buffer queue on the receiver's task descriptor so when the receiver calls `Receive()` it can look if a sender is already in its queue. Through testing, we have verified that this buffer does not overflow.

# 4 User Features

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

`RPSClientTask1` signs up, plays rock three times, and quits.
`RPSClientTask2` signs up, rock, paper, scissors, rock, paper, scissors, and quits.
`RPSClientTask3` signs up, plays rock and quits. Signs up again, plays paper and then quits.

# 5 RPS Game Test (`user/k2.c`) (`user/RPS/client.c`)

The RPS Game test (`RPSTask()`) begins by starting the RPS Server.

Then, three tests are initialized. 

Test 1 starts by creating RPS client task 1 and client task 2, where the behaviour is expected to be a tie on the first round, task 2 winning the second round, and then task 1 winning the third round. Task 1 quits, while task 2 plays three more moves (that are invalid since task 1 quit already) and then quits.

Test 2 starts by calling task 1 twice. Since they are both the same, there should be three ties.
Then, it calls task 2 twice. Since they are both the same, there should be six ties.

Test 3 creates Task 3 three times. We will call the tasks 1, 2, and 3 respectively.

Task 1 will play rock and task 2 will also play rock, giving a tie. They both quit, and then signup again, resulting in them both playing paper, resulting in a tie. 

Task 3 gets created, and hangs on the initial sign up.

Output:
```
Starting K2 TAsk!
Test 1
[CLIENT] Player 4 requesting to sign up
[RPS SERVER Signup]: Player Tid 4 joined. Waiting for one more.
[CLIENT] Player 5 requesting to sign up
[RPS SERVER Signup]: Player Tid 5 joined. Player Tid 4 already here. Starting...
[CLIENT] Player 4 requesting to play move ROCK
[CLIENT] Player 5 requesting to play move ROCK
[RPS SERVER Play] Player 4 played move: ROCK
[RPS SERVER Play] Player 5 played move: ROCK
[RPS SERVER Play] player 4 and 5's game completed -> responding with results
[CLIENT] Player 4 got a TIE
[CLIENT] Player 5 got a TIE
[CLIENT] Player 4 requesting to play move ROCK
[RPS SERVER Play] Player 4 played move: ROCK
[CLIENT] Player 5 requesting to play move PAPER
[RPS SERVER Play] Player 5 played move: PAPER
[RPS SERVER Play] player 4 and 5's game completed -> responding with results
[CLIENT] Player 4 got a LOSE
[CLIENT] Player 5 got a WIN
[CLIENT] Player 4 requesting to play move ROCK
[RPS SERVER Play] Player 4 played move: ROCK
[CLIENT] Player 5 requesting to play move SCISSORS
[RPS SERVER Play] Player 5 played move: SCISSORS
[RPS SERVER Play] player 4 and 5's game completed -> responding with results
[CLIENT] Player 4 got a WIN
[CLIENT] Player 5 got a LOSE
[CLIENT] Player 4 requesting to quit
[RPS SERVER Quit] Player 4 has quit
[CLIENT] Player 5 requesting to play move ROCK
[RPS SERVER Play] Player 5 played move: ROCK
[RPS SERVER Play] Rejected player 5's move since game is already completed.
[CLIENT] Player 5 got a OPPONENT QUIT
[CLIENT] Player 5 requesting to play move PAPER
[RPS SERVER Play] Player 5 played move: PAPER
[RPS SERVER Play] Rejected player 5's move since game is already completed.
[CLIENT] Player 5 got a OPPONENT QUIT
[CLIENT] Player 5 requesting to play move SCISSORS
[RPS SERVER Play] Player 5 played move: SCISSORS
[RPS SERVER Play] Rejected player 5's move since game is already completed.
[CLIENT] Player 5 got a OPPONENT QUIT
[CLIENT] Player 5 requesting to quit
[RPS SERVER Quit] Player 5 has quit
Test 2
[CLIENT] Player 6 requesting to sign up
[RPS SERVER Signup]: Player Tid 6 joined. Waiting for one more.
[CLIENT] Player 7 requesting to sign up
[RPS SERVER Signup]: Player Tid 7 joined. Player Tid 6 already here. Starting...
[CLIENT] Player 6 requesting to play move ROCK
[CLIENT] Player 7 requesting to play move ROCK
[RPS SERVER Play] Player 6 played move: ROCK
[RPS SERVER Play] Player 7 played move: ROCK
[RPS SERVER Play] player 6 and 7's game completed -> responding with results
[CLIENT] Player 6 got a TIE
[CLIENT] Player 7 got a TIE
[CLIENT] Player 6 requesting to play move ROCK
[RPS SERVER Play] Player 6 played move: ROCK
[CLIENT] Player 7 requesting to play move ROCK
[RPS SERVER Play] Player 7 played move: ROCK
[RPS SERVER Play] player 6 and 7's game completed -> responding with results
[CLIENT] Player 6 got a TIE
[CLIENT] Player 7 got a TIE
[CLIENT] Player 6 requesting to play move ROCK
[RPS SERVER Play] Player 6 played move: ROCK
[CLIENT] Player 7 requesting to play move ROCK
[RPS SERVER Play] Player 7 played move: ROCK
[RPS SERVER Play] player 6 and 7's game completed -> responding with results
[CLIENT] Player 6 got a TIE
[CLIENT] Player 7 got a TIE
[CLIENT] Player 6 requesting to quit
[RPS SERVER Quit] Player 6 has quit
[CLIENT] Player 7 requesting to quit
[RPS SERVER Quit] Player 7 has quit
[CLIENT] Player 8 requesting to sign up
[RPS SERVER Signup]: Player Tid 8 joined. Waiting for one more.
[CLIENT] Player 9 requesting to sign up
[RPS SERVER Signup]: Player Tid 9 joined. Player Tid 8 already here. Starting...
[CLIENT] Player 8 requesting to play move ROCK
[CLIENT] Player 9 requesting to play move ROCK
[RPS SERVER Play] Player 8 played move: ROCK
[RPS SERVER Play] Player 9 played move: ROCK
[RPS SERVER Play] player 8 and 9's game completed -> responding with results
[CLIENT] Player 8 got a TIE
[CLIENT] Player 9 got a TIE
[CLIENT] Player 8 requesting to play move PAPER
[RPS SERVER Play] Player 8 played move: PAPER
[CLIENT] Player 9 requesting to play move PAPER
[RPS SERVER Play] Player 9 played move: PAPER
[RPS SERVER Play] player 8 and 9's game completed -> responding with results
[CLIENT] Player 8 got a TIE
[CLIENT] Player 9 got a TIE
[CLIENT] Player 8 requesting to play move SCISSORS
[RPS SERVER Play] Player 8 played move: SCISSORS
[CLIENT] Player 9 requesting to play move SCISSORS
[RPS SERVER Play] Player 9 played move: SCISSORS
[RPS SERVER Play] player 8 and 9's game completed -> responding with results
[CLIENT] Player 8 got a TIE
[CLIENT] Player 9 got a TIE
[CLIENT] Player 8 requesting to play move ROCK
[RPS SERVER Play] Player 8 played move: ROCK
[CLIENT] Player 9 requesting to play move ROCK
[RPS SERVER Play] Player 9 played move: ROCK
[RPS SERVER Play] player 8 and 9's game completed -> responding with results
[CLIENT] Player 8 got a TIE
[CLIENT] Player 9 got a TIE
[CLIENT] Player 8 requesting to play move PAPER
[RPS SERVER Play] Player 8 played move: PAPER
[CLIENT] Player 9 requesting to play move PAPER
[RPS SERVER Play] Player 9 played move: PAPER
[RPS SERVER Play] player 8 and 9's game completed -> responding with results
[CLIENT] Player 8 got a TIE
[CLIENT] Player 9 got a TIE
[CLIENT] Player 8 requesting to play move SCISSORS
[RPS SERVER Play] Player 8 played move: SCISSORS
[CLIENT] Player 9 requesting to play move SCISSORS
[RPS SERVER Play] Player 9 played move: SCISSORS
[RPS SERVER Play] player 8 and 9's game completed -> responding with results
[CLIENT] Player 8 got a TIE
[CLIENT] Player 9 got a TIE
[CLIENT] Player 8 requesting to quit
[RPS SERVER Quit] Player 8 has quit
[CLIENT] Player 9 requesting to quit
[RPS SERVER Quit] Player 9 has quit
Test 3
[CLIENT] Player 10 requesting to sign up
[RPS SERVER Signup]: Player Tid 10 joined. Waiting for one more.
[CLIENT] Player 11 requesting to sign up
[RPS SERVER Signup]: Player Tid 11 joined. Player Tid 10 already here. Starting...
[CLIENT] Player 10 requesting to play move ROCK
[CLIENT] Player 11 requesting to play move ROCK
[RPS SERVER Play] Player 10 played move: ROCK
[RPS SERVER Play] Player 11 played move: ROCK
[RPS SERVER Play] player 10 and 11's game completed -> responding with results
[CLIENT] Player 10 got a TIE
[CLIENT] Player 11 got a TIE
[CLIENT] Player 10 requesting to quit
[RPS SERVER Quit] Player 10 has quit
[CLIENT] Player 11 requesting to quit
[CLIENT] Player 10 requesting to sign up
[RPS SERVER Quit] Player 11 has quit
[CLIENT] Player 11 requesting to sign up
[RPS SERVER Signup]: Player Tid 10 joined. Waiting for one more.
[RPS SERVER Signup]: Player Tid 11 joined. Player Tid 10 already here. Starting...
[CLIENT] Player 10 requesting to play move PAPER
[CLIENT] Player 11 requesting to play move PAPER
[RPS SERVER Play] Player 10 played move: PAPER
[RPS SERVER Play] Player 11 played move: PAPER
[RPS SERVER Play] player 10 and 11's game completed -> responding with results
[CLIENT] Player 10 got a TIE
[CLIENT] Player 11 got a TIE
[CLIENT] Player 10 requesting to quit
[RPS SERVER Quit] Player 10 has quit
[CLIENT] Player 11 requesting to quit
[RPS SERVER Quit] Player 11 has quit
[CLIENT] Player 12 requesting to sign up
[RPS SERVER Signup]: Player Tid 12 joined. Waiting for one more.
[WARN] No next task
```

# 6 Performance Measurement

## 6.1 Performance Methodology
The performance methodology we used is measuring multiple times:

```C
loop N=20 times:
  set_start(&timer, SSR_TIME);
  Send();
  set_end(&timer, SSR_TIME);
endloop;
```
We use this methodology as it allows us to measure variance and single instances of time. From comparing, max, avg, and the iteration values, we see that variance between tests is very low. Furthermore, the overhead from the PerfTimingState is very low because we did not see a difference in tests when testing , which we know because when we tried the other methodology of `inflate impact of operation`:
```c
set_start(&timer, SSR_TIME);
loop N=20 times:
    operation();
endloop;
set_end(&timer, SSR_TIME);
```
By comparing these to, we realize that our overhead on the timer is not much as the results do not differ.

## 6.2 Performance Testing
For each variation of test, we create two tasks: one that sends, and one that receives. Each task will perform 20 iterations of their respective send/receive on either 4, 64, 256 bytes of a message size. 20 iterations are used to prevent a high variance and a consistent recording in measurements. The timer is created in Send and ends when a reply has been received to time a full SRR.

The following features were permuted and tested: Optimization/No Optimization, icache/dcache/both/none, ReceiverFirst/SenderFirst, 4/64/256 Bytes.

Currently, `-O3` flag is commented out in the Makefile. The `CACHE_ENABLE` variable in `boot.S` is also disabled. To test caching and optimization features, please uncomment those.

## 6.3 Performance Conclusions

We can see that with regardless of whether optimization is on or whether cache is on, the general behaviour when the message size is increased is that the SRR time in microseconds increases. This makes sense as it takes longer for `memcopy` to copy over the message, making the SRR time longer.

We can also make the observation that Sender first gives a faster SRR time than Receiver first. The reason for this behaviour is because when Receive gets called first, we need to allocate a receive state. When we send first, there is no need to store a receive state as we can process the sender's info immediately. This extra allocate on the receiver side and free on the sender side when receiver gets called first can be the explanation as to why our time is a bit slower.

Another observation is that when optimization is turned on, the time is twice as fast compared to its no optimization counterpart. This makes intuitive sense as compiler optimization speeds up instructions. 

When both the instruction and data caches are turned on, we see a slight improvement in SRR times. This makes sense as repetitive instructions or access to data can be optimized. 

We can see that if we compare solely the instruction cache being turned on versus only the data cache being turned on, instruction cache gives a slightly better improvement. This is because more instructions are being cached in comparison to data caching.