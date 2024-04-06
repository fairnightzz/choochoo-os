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

.global Send
.type Send, %function
Send:
    svc 0x5
    ret

.global Receive
.type Receive, %function
Receive:
    svc 0x6
    ret

.global Reply
.type Reply, %function
Reply:
    svc 0x7
    ret

.global AwaitEvent
.type AwaitEvent, %function
AwaitEvent:
    svc 0x8
    ret
  
.global Kill
.type Kill, %function
Kill:
  svc 0x9
  ret


.global	reboot
.type reboot, %function
reboot:

mov x2, 36
mov x0, 28
movk  x2, 0xfe10, lsl 16
movk  x0, 0xfe10, lsl 16
mov w3, 1
mov w1, 32
movk  w3, 0x5a00, lsl 16
str w3, [x2]
movk  w1, 0x5a00, lsl 16
str w1, [x0]

exit:
    // shouldn't be reached
    wfi
    b  exit