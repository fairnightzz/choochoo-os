.text
.align 2

.global Create
.type Create, %function
Create:
  svc 0x0
  ret

.global MyTid
.type MyTid, %function
MyTid:
  svc 0x1
  ret

.global MyParentTid
.type MyParentTid, %function
MyParentTid:
  svc 0x2
  ret

.global Yield
.type Yield, %function
Yield:
  svc 0x3
  ret

.global Exit
.type Exit, %function
Exit:
  svc 0x4
  ret
