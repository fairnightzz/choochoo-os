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

We allocate 128KB for the kernel stack, and each task gets an address space of 4KB (+ extra allocated for switch frame, task descriptor, and scheduler in kernel ~ 400B) page to use as a stack.
The maximum amount of tasks is 300 so our kernel does not exceed 128 KB in size.

Thus, we can only create 300 active tasks in the kernel.
If this limit is reached, the kernel prevents the creation of new tasks.
Tasks after exiting will have their space recycled.

At max memory usage of our address space we use 1.2 MB for user address spaces over all 300 tasks + 128 KB kernel space. We can increase this to a lot more, but it is important to note that we currently have a heap memory allocator which holds 30 KB of static memory used for switchframes, task descriptors, and scheduler nodes. This allocator currently limits us to onlu ~33 active tasks. However, for this assignment that is good enough since the request needs only 5 active tasks.

One last thing is based on our task descriptor we can only do tasks up to 128. Meaning if there have already been 128 created tasks up to this point, creating a new task will result in -2. 

### 3.22 Task Descriptor & Storage (`kern/task_descriptor.h`)

The task descriptor used to define each task contains the following fields:

- `tid`: uniquely indentify tasks
- `pTid`: id of parent task
- `switch_frame`: snapshot of registers on kernel side
- `status`: status of the task
- `pri`: the task priority
- `addrspace`: task's address space in usermode
See: `kern/task_descriptor.h` for details.

Task Descriptor metadata is stored in a hash table, which is implemented as an array of 128 task descriptor pointers. Note this is a caveat that does not allow us to make more than 128 tasks over the lifetime of this kernel. To fix this we can implement a hashmap like approach. To do in future assignments.


### 3.23 Task Scheduler (`kern/scheduler.h`)

The task scheduler is implemented using 32 priority levels, where 0 is the highest priority level and 31 is the lowest priority level. As per the kernel specification, since we can have more than one task at any priority, the scheduler is implemented as an array that is 32 long, and each value in the array has a linked list. Hence, it is a list of linked list nodes. It is implemented with round robin scheduling so the task at the head of the list will run first, and the 
task at the tail of the list will run last. When there are no tasks running, 0 as tid gets returned and the kernel stalls.

### 3.24 Heap Memory & Allocation (`kern/kalloc.h`)

Created an allocator using direct addressing and pointer arithmetic to assign blocks of data for switch_frames, task descriptors, and scheduling nodes. This uses the slab allocation algorithm to assign one block of memory for each type of struct of memory we are allocating. It is worth noting that currently we are using equal blocks of memory for each partition of structs with each block being 10 KB.

In future assignments, we might want to change this allocator to have more memory and to have different block sizes per struct allocated.

### 3.25 Context Switch (`kern/switchframe.h` / `kern/enter_modes.S` / `exception_table.S`)

Initialized vector exception table in the start.
When a syscall gets called, we call the function `enter_kernelmode` which first get the switchframe struct in the task descriptor and we save all the registers onto the switchframe.
We call `handle_svc()` to handle the syscall.

Once the svc has been handled, we call `enter_usermode`. This restores the registers back to what they were before and we return from the exception.

todo: figure out how to optimize more (save less registers)

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




