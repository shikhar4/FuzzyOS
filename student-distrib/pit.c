#include "pit.h"

// static volatile int timer_ticks = 0;


#define PIT_FREQ    1193180
#define PIT_BITMASK 0xFF
#define PIT_SHIFT   8
#define PIT_CONTROL 0x43
#define PIT_A       0x40
#define PIT_SET     0x36

/** 
 * init_pit
 * 
 * Description: Initializes PIT 
 * Inputs: none
 * Outputs: none
 * Side Effects: Enables PIT, from OSDEV
 */
void init_pit() {
    outb(PIT_SET, PIT_CONTROL);
    outb(PIT_FREQ & PIT_BITMASK, PIT_A);
    outb(PIT_FREQ >> PIT_SHIFT, PIT_A);
    enable_irq(0);              // enable master port 0
}

/** 
 * pit_irq
 * 
 * Description: send eoi and call scheduling, this is called each PIT interrupt
 * Inputs: none
 * Outputs: none
 * Side Effects: Calls scheduler to implement round robin scheduling
 */
void pit_irq() {
    send_eoi(0);
    scheduler();
    send_eoi(0);
}
