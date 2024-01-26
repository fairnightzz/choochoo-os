<div align="center">

# CS452-A0-Polling_Loop
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

# 3 Kernel Structure

# 4 Program Output

Our kernel outputs the following on the user tasks defined in the assignment page (also defined in `user/userprog.c`):

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


