<div align="center">

# CS452 - Kernel Part 1: Context Switch & Scheduling
### Names: Anish Aggarwal, Zhehai Zhang
### Student IDs: a59aggar, z2252zha
### Date: 01/30/24

</div>

# 1 Overview
This is **Kernel 1** of CS452 W24 @UWaterloo. We have started creating the implementation of a Kernel that allows for system calls from user programs. Hope you like it :)

# 2 Hash Commit & Testing
Run the following on a `linux.student.cs` environment:
```bash
git clone https://git.uwaterloo.ca/z2252zha/choochoo-os.git
cd choochoo-os
git <insert>
make
```

The above works on machine <insert>.

# 3 Kernel Features & Structure
## 3.1 Kernel Features
Our choochoo-os will run the first user task, which operates as described on the assignment page.

The choochoo-os kernel implements following five syscalls described on the assignment page with no modifications to their signatures:

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

The first task created has a TID of 1.
No task can have a TID of 0.
A TID of 0 acts as an error value for functions that should return a TID but cannot.

When there are no more tasks in the scheduler, our kernel simply hangs.

## 3.2 Kernel Structure
Our kernel program core is written in `kern.c` (header file `kern.h`) where we initialize our kernel which intern initializes all our structures and modular components on the kernel side (i.e. heap memory, i/o, vector exception table). It also provides two other functions: 
```C
 int svc_create(uint32_t priority, void (*entrypoint)());
 ```
```C
void handle_svc();
```

The first of which is the function which our `kern/main.c` file uses to execute the first user task program. The second function is used by our assembly script in `enter_mode.S` when entering kernel mode. This handles the system call made by the user mode. The following are all the core concepts and explanations of the algorithms / data structures used for each:

### 3.21 Memory Management (`kern/addrspace.h`)

We allocate 2MB for the kernel stack, and each task gets a 1MB page to use as a stack.
The maximum amount of tasks is 1024 so that we take up at most 1GB (+ 2MB for the kernel) of memory.

Thus, we can only create 1024 tasks over the lifespan of the kernel.
If this limit is reached, the kernel prevents the creation of new tasks.

We currently have a fake heap allocator implemented, which is actually 2048 bytes of static memory.
This will be improved into an actual heap allocator at a later point in time.
Unfortunately, this means that we run out of memory on the sixth task created.
Fortunately, the assignment only requires five.

### 3.22 Task Descriptor & Storage (`kern/task_descriptor.h`)
### 3.23 Task Scheduler (`kern/scheduler.h`)
### 3.24 Heap Memory & Allocation (`kern/kalloc.h`)
### 3.25 Context Switch (`kern/switchframe.h` / `kern/enter_modes.S`)

# 4 Program Output

Our kernel program outputs the following on the user tasks defined in the assignment page (also defined in `user/userprog.c`):

```
Created: 2
Created: 3
MyTid = 4, MyParentTid = 1
MyTid = 4, MyParentTid = 1,
Created: 4
MyTid = 5, MyParentTid = 1
MyTid = 5, MyParentTid = 1
Created: 5
FirstUserTask: exiting
MyTid = 2, MyParentTid = 1
MyTid = 3, MyParentTid = 1
MyTid = 2, MyParentTid = 1
MyTid = 3, MyParentTid = 1
[WARN] no next task because scheduler is empty
```

Here is the detailed explanation for the above output:
1. The first task created is `firstUserTask` which has Tid **1** and priority **4** (created in `kern/main.c`)
2. After entering the usermode for the first task, the function `firstUserTask` (the entrypoint) makes a syscall to `Create(5, &otherTask)`. This schedules another task in the kernel to run at the entrypoint `otherTask` with priority 5. Since, thus priority is lower (note higher priority numbers mean lower priority, i.e. highest priority is priority 0), the `firstUserTask` continues to run.
3. Then, the program prints out `Created: 2`, since the Tid of the created task is 2. Note our Tid numberings start at 1 since we use 0 as an error Tid.
4. Again, the usermode (running `firstUserTask`) makes a syscall to `Create(5, &otherTask)`. Since the priority is lower, `firstUserTask` continues to run. The program logs `Created: 3`. Since 3 is the Tid of the created task.
5. The next syscall the program makes is `Create(3, &otherTask)`. Since this priority is higher, the `firstUserTask` gets preempted and the otherTask with Tid **4** starts to run.
6. This logs to console `MyTid = 4, MyParentTid = 1` before making the syscall `Yield()`. Since, it is still the task with strictly the largest priority, it will continue to run.
7. The `otherTask` with Tid **4** continues to run and logs `MyTid = 4, MyParentTid = 1` to console before exiting (calling syscall `Exit()`). 
8. After exiting, the kernel chooses the next task to run which is `firstUserTask` which logs `Created: 4` since it enters back into the function where it left off.
9. Now, the `firstUserTask` makes syscall `Create(3, &otherTask)` which again creates a higher priority task with entry point `otherTask`. Similar to before this will log two lines of `MyTid = 5, MyParentTid = 1` before exiting since there are no equal or higher priority tasks in queue.
10. After exiting, `firstUserTask` will start running again since it has the next highest priority and log `Created: 5`. Finally, `firstUserTask`, will log `firstUserTask: exiting` before actually calling syscall `Exit()`.
11. At this point, the two lower priority tasks will be able to run. We start with task with Tid **2** since that was entered in the queue first. It logs `MyTid = 2, MyParentTid = 1` before calling syscall `Yield()`. At this point, the task with Tid **2** will be put at the end of its priority queue allowing task with Tid **3** to run.
12. Task with Tid **3** will log `MyTid = 3, MyParentTid = 1` to console before yielding, allowing task 2 (the only other task) to run again.
13. Task with Tid **2** will log `MyTid = 2, MyParentTid = 1` before calling syscall `Exit()`.
14. Task with Tid **3** will log `MyTid = 2, MyParentTid = 1` before calling syscall `Exit()`.
15. All tasks have been completed, and kernel has nothing left in scheduler. Program will stall. 




