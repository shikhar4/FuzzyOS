/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

/* Initialize the 8259 PIC */

/** 
 * i8259_init
 * 
 * Description: initializes pic and masks after initialization
 * Inputs: none
 * Outputs: none
 * Side Effects: send proper control words to master and slave
 */
void i8259_init(void) {
    //unsigned long flags;
    // cli(_and_save(flags));
    

    outb(ICW1, MASTER_8259_PORT);                       // initialize master
    outb(ICW2_MASTER, MASTER_8259_PORT_DATA);
    outb(ICW3_MASTER, MASTER_8259_PORT_DATA);           // slave in port 2
    outb(ICW4, MASTER_8259_PORT_DATA);

    outb(ICW1, SLAVE_8259_PORT);                        // initialize slave
    outb(ICW2_SLAVE, SLAVE_8259_PORT_DATA);
    outb(ICW3_SLAVE, SLAVE_8259_PORT_DATA);             // in master's port 2
    outb(ICW4, SLAVE_8259_PORT_DATA);

    outb(0xff, MASTER_8259_PORT_DATA);                  // mask all of 8259A-1
    outb(0xff, SLAVE_8259_PORT_DATA);                   // mask all of 8259A-1

    //enable_irq(2);
    // restore_flags(flags);
    // sti();
}

/** 
 * enable_irq
 * 
 * Description: enables port on master or slave, depending on irq number
 * Inputs: irq_num
 * Outputs: none
 * Side Effects: none
 */
/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num) {
    uint16_t port;
    if(irq_num >= NUM_PIC_PORTS){                                                   // if writing to slave
        port = inb(SLAVE_8259_PORT_DATA);                               // write to slave
        outb(port & ~(1 << (irq_num - NUM_PIC_PORTS)), SLAVE_8259_PORT_DATA);       // notify master that slave is being written to
        // outb(inb(MASTER_8259_PORT_DATA) & ~(1 << 2), MASTER_8259_PORT_DATA);
    }
    else{
        port = inb(MASTER_8259_PORT_DATA);                              // write to master    
        outb(port & ~(1 << irq_num), MASTER_8259_PORT_DATA);
    }
}

/** 
 * disable_irq
 * 
 * Description: disables ports on master or slave, depending on irq number 
 * Inputs: irq_num
 * Outputs: none
 * Side Effects: none
 */
/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num) {
    uint16_t port;
    if(irq_num >= NUM_PIC_PORTS){
        port = inb(SLAVE_8259_PORT_DATA);                               // if slave, mask slave port and notify master
        outb(port | (1 << (irq_num - NUM_PIC_PORTS)), SLAVE_8259_PORT_DATA);
        // outb(inb(MASTER_8259_PORT_DATA) | (1 << 2), MASTER_8259_PORT_DATA);
    }
    else{
        port = inb(MASTER_8259_PORT_DATA);                              // if master, mask master port
        outb(port | (1 << irq_num), MASTER_8259_PORT_DATA);
    }
}

/** 
 * send_eoi
 * 
 * Description: sends eoi to master or slave, depending on irq number
 * Inputs: irq_num
 * Outputs: none
 * Side Effects: none
 */
/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num) {
    if(irq_num >= NUM_PIC_PORTS){
        outb(EOI|(irq_num-NUM_PIC_PORTS), SLAVE_8259_PORT);                     // if slave, send eoi to slave and notify master
        outb(EOI| 2, MASTER_8259_PORT);                             // slave is on port 2
    }
    else{
        outb(EOI|irq_num, MASTER_8259_PORT);                        // if master, send eoi
    }
}
