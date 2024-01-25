.global vector_table_initialize
.type vector_table_initialize, %function
vector_table_initialize:
  // update to our new vector table
  ldr x0, =vector_table_el1
  msr VBAR_EL1, x0
  ret

// Typical exception vector table code. We use EL1 for Operating system kernel level.
.balign 0x800
vector_table_el1:
curr_el_sp0_sync:        // The exception handler for a synchronous 
                         // exception from the current EL using SP0.
  nop

.balign 0x80
curr_el_sp0_irq:         // The exception handler for an IRQ exception
                         // from the current EL using SP0.
  nop

.balign 0x80
curr_el_sp0_fiq:         // The exception handler for an FIQ exception
                         // from the current EL using SP0.
  nop

.balign 0x80
curr_el_sp0_serror:      // The exception handler for a System Error 
                         // exception from the current EL using SP0.
  nop

.balign 0x80
curr_el_spx_sync:        // The exception handler for a synchrous 
                         // exception from the current EL using the
                         // current SP.
  nop

.balign 0x80
curr_el_spx_irq:         // The exception handler for an IRQ exception from 
                         // the current EL using the current SP.
  nop

.balign 0x80
curr_el_spx_fiq:         // The exception handler for an FIQ from 
                         // the current EL using the current SP.
  nop

.balign 0x80
curr_el_spx_serror:      // The exception handler for a System Error 
                         // exception from the current EL using the
                         // current SP.
  nop

 .balign 0x80
lower_el_aarch64_sync:   // The exception handler for a synchronous 
                         // exception from a lower EL (AArch64).
                         // Enter kernel mode here
  b enter_kernelmode

.balign 0x80
lower_el_aarch64_irq:    // The exception handler for an IRQ from a lower EL
                         // (AArch64).
  nop

.balign 0x80
lower_el_aarch64_fiq:    // The exception handler for an FIQ from a lower EL
                         // (AArch64).
  nop

.balign 0x80
lower_el_aarch64_serror: // The exception handler for a System Error 
                         // exception from a lower EL(AArch64).
  nop

.balign 0x80
lower_el_aarch32_sync:   // The exception handler for a synchronous 
                         // exception from a lower EL(AArch32).
  nop

.balign 0x80
lower_el_aarch32_irq:    // The exception handler for an IRQ exception 
                         // from a lower EL (AArch32).
  nop

.balign 0x80
lower_el_aarch32_fiq:    // The exception handler for an FIQ exception from 
                         // a lower EL (AArch32).
  nop

.balign 0x80
lower_el_aarch32_serror: // The exception handler for a System Error
                         // exception from a lower EL(AArch32).
  nop
