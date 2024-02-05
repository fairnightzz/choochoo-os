.text
.align 2

.global	get_esr_el1
.type get_esr_el1, %function
get_esr_el1:
    mrs x0, ESR_EL1
    ret

.global call_wfi
.type call_wfi, %function
call_wfi:
    wfi
    ret