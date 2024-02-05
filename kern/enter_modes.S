.text
.align 2
.global	enter_kernelmode
.type enter_kernelmode, %function
enter_kernelmode:

# ESR_EL1 holds exception code and N
# ELR_EL1 holds return address (next PC after svc)
# SPSR_EL1 hods pstate before exception

# save registers
# call get task
# save registers onto switchframe
# save the special registers
# call handle svc

# save registers
sub sp, sp, #32
str x0, [sp, #0]
str x1, [sp, #8]
str x2, [sp, #16]
str x3, [sp, #24]
str x30, [sp, #32]

# returns Task struct
bl get_current_task

# x0 = task->switch_frame
ldr x0, [x0, #0]

# Start to save all registers into the switch frame.
# save original x0 to switchframe
ldr x1, [sp, #0]
str x1, [x0, #0]

# load the other register in
ldr x1, [sp, #8]
ldr x2, [sp, #16]
ldr x3, [sp, #24]
ldr x30, [sp, #32]
add sp, sp, #32

# add general purpose registers into switch frame
str x1, [x0, #8]
stp x2, x3, [x0, #16]
stp x4, x5, [x0, #32]
stp x6, x7, [x0, #48]
stp x8, x9, [x0, #64]
stp x10, x11, [x0, #80]
stp x12, x13, [x0, #96]
stp x14, x15, [x0, #112]
stp x16, x17, [x0, #128]
stp x18, x19, [x0, #144]
stp x20, x21, [x0, #160]
stp x22, x23, [x0, #176]
stp x24, x25, [x0, #192]
stp x26, x27, [x0, #208]
stp x28, x29, [x0, #224]
str x30, [x0, #240]

# save return address
mrs x1, ELR_EL1
str x1, [x0, #248]

# save pstate
mrs x1, SPSR_EL1
str x1, [x0, #256]

# save stack pointer of kernel
mrs x1, SP_EL0
str x1, [x0, #264]

b handle_svc

.global	enter_usermode
.type enter_usermode, %function
enter_usermode:

    # load return address
    ldr x1, [x0, #248]
    msr ELR_EL1, x1

    # load stack pointer of kernel
    ldr x1, [x0, #264]
    msr SP_EL0, x1

    # load state reg
    ldr x1, [x0, #256]
    msr SPSR_EL1, x1

    # load registers back from switch frame
    ldp x2, x3, [x0, #16]
    ldp x4, x5, [x0, #32]
    ldp x6, x7, [x0, #48]
    ldp x8, x9, [x0, #64]
    ldp x10, x11, [x0, #80]
    ldp x12, x13, [x0, #96]
    ldp x14, x15, [x0, #112]
    ldp x16, x17, [x0, #128]
    ldp x18, x19, [x0, #144]
    ldp x20, x21, [x0, #160]
    ldp x22, x23, [x0, #176]
    ldp x24, x25, [x0, #192]
    ldp x26, x27, [x0, #208]
    ldp x28, x29, [x0, #224]
    ldr x30, [x0, #240]

    # load 0 and 1 back in
    ldp x0, x1, [x0, #0]

    eret

.global	enter_kernelmode_irq
.type enter_kernelmode_irq, %function
enter_kernelmode_irq:

# ESR_EL1 holds exception code and N
# ELR_EL1 holds return address (next PC after svc)
# SPSR_EL1 hods pstate before exception

# save registers
# call get task
# save registers onto switchframe
# save the special registers
# call handle svc

# save registers
sub sp, sp, #32
str x0, [sp, #0]
str x1, [sp, #8]
str x2, [sp, #16]
str x3, [sp, #24]
str x30, [sp, #32]

# returns Task struct
bl get_current_task

# x0 = task->switch_frame
ldr x0, [x0, #0]

# Start to save all registers into the switch frame.
# save original x0 to switchframe
ldr x1, [sp, #0]
str x1, [x0, #0]

# load the other register in
ldr x1, [sp, #8]
ldr x2, [sp, #16]
ldr x3, [sp, #24]
ldr x30, [sp, #32]
add sp, sp, #32

# add general purpose registers into switch frame
str x1, [x0, #8]
stp x2, x3, [x0, #16]
stp x4, x5, [x0, #32]
stp x6, x7, [x0, #48]
stp x8, x9, [x0, #64]
stp x10, x11, [x0, #80]
stp x12, x13, [x0, #96]
stp x14, x15, [x0, #112]
stp x16, x17, [x0, #128]
stp x18, x19, [x0, #144]
stp x20, x21, [x0, #160]
stp x22, x23, [x0, #176]
stp x24, x25, [x0, #192]
stp x26, x27, [x0, #208]
stp x28, x29, [x0, #224]
str x30, [x0, #240]

# save return address
mrs x1, ELR_EL1
str x1, [x0, #248]

# save pstate
mrs x1, SPSR_EL1
str x1, [x0, #256]

# save stack pointer of kernel
mrs x1, SP_EL0
str x1, [x0, #264]

# reset kernel stack pointer so that extra variables get destructured
mov sp, #0x00200000

b handle_irq