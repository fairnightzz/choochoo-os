#include "gic.h"

#define GIC_BASE 0xFF840000

static char* const GICD_BASE = (char*) (GIC_BASE + 0x1000);
static char* const GICC_BASE = (char*) (GIC_BASE + 0x2000);

static const uint32_t GICD_ISENABLE = 0x100;
static const uint32_t GICD_ITARGETS = 0x800;

static const uint32_t GICC_IAR = 0xC;
static const uint32_t GICC_EOIR = 0x10;

#define GICD_REG(offset) (*(volatile uint32_t*)(GICD_BASE + offset))
#define GICC_REG(offset) (*(volatile uint32_t*)(GICC_BASE + offset))

void gic_target_and_enable(uint32_t interruptId)
{
  // Route interrupts of id 97 to core 0 - Routes each interrupt to correct core
  uint32_t core = 0;
  char* target = GICD_BASE + GICD_ITARGETS + 4*(interruptId/4); // find target register (each of which handles four interrupt)
  *(target + interruptId%4) |= 0x1 << core; // set the target register byte for specified interrupt id to be processed by the current processor

  // Enable the Interrupt to register the hardware to start interrupting the system
  uint32_t* enable = (uint32_t*) (GICD_BASE + GICD_ISENABLE + 4*(interruptId/32));
  *enable |= 0x1 << (interruptId%32);
}

uint32_t gic_read_iar() {
    // Pointer to the Interrupt Acknowledge Register (IAR)
    uint32_t* interruptAcknowledgeReg = (uint32_t*) (GICC_BASE + GICC_IAR);
    
    // Read and return the value from the IAR. This value contains
    // the ID of the interrupt that is currently being acknowledged.
    return *interruptAcknowledgeReg;
}

void gic_write_eoir(uint32_t interruptId) {
    // Pointer to the End Of Interrupt Register (EOIR)
    uint32_t* endOfInterruptReg = (uint32_t*) (GICC_BASE + GICC_EOIR);
    
    // Write the interrupt ID to the EOIR. This signals the end of interrupt
    // processing to the GIC, indicating that the system is ready for new interrupts.
    *endOfInterruptReg = interruptId;
}