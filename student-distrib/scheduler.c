#include "scheduler.h"

int32_t terminal_esp[NUM_TERMINALS];
int32_t terminal_ebp[NUM_TERMINALS];

/** 
 * scheduler
 * 
 * Description: Implements round robin scheduling, switch to next execute terminal
 * Inputs: none
 * Outputs: none
 * Side Effects: Context switch to next program kernel stack
 */
void scheduler() {
    // Save user context (esp, ebp)
	// context switch (switching esp ebp to next kernel stack)

	//esential to save esp, ebp first
	asm volatile(
		"	movl %%esp, %0		\n	\
			movl %%ebp, %1		\n 	\
		"
		: "=rm"(terminal_esp[getExecuteTerm()]), "=rm"(terminal_ebp[getExecuteTerm()])
		: // no inputs
	);
	if (pid == -1) {
		change_display_terminal(2);		//execute first terminal, only done once
		ATTRIB = colors[2]; // pretty color number 2
		setExecuteTerm(2);
		setDisplayTerm(2);
		clear();
		reset_cursor();
		update_cursor();
		
		execute((uint8_t *)"shell");
		return;
	}
	if (pid == 0) {
		change_display_terminal(1);		// execute second terminal, only done once
		ATTRIB = colors[1];
		setExecuteTerm(1);
		setDisplayTerm(1);
		clear();
		reset_cursor();
		update_cursor();
		
		execute((uint8_t *)"shell");
		return;
	}
	if (pid == 1) {
		change_display_terminal(0);		// execute third terminal, only done once
		ATTRIB = colors[0];
		setExecuteTerm(0);
		setDisplayTerm(0);
		clear();
		reset_cursor();
		update_cursor();
		
		execute((uint8_t *)"shell");
		return;
	}
	

	//increment execute terminal
	setExecuteTerm((getExecuteTerm() + 1) % NUM_TERMINALS);

	// enable paging for current program 
	map_virtual_addr_to_physical(USER_MEM_START, USER_PHYS_MEM_START + (getExecuteTerm() * USER_MEM_SIZE) + (terminal_process_num[getExecuteTerm()] * NUM_TERMINALS * USER_MEM_SIZE));

	// remap paging for video mem based on display and executing terminal
	uint32_t video_phys_mem = VIDEO;
	if (getDisplayTerm() != getExecuteTerm()) {
		video_phys_mem = terminal_vidmem[getExecuteTerm()];	
	}
	
	// remap video memory to appropriate memory location
	map_virtual_addr_to_physical_4kb_page(USER_MEM_START+USER_MEM_SIZE, video_phys_mem);

	// set TSS for scheduled process
	tss.ss0 = KERNEL_DS;
    tss.esp0 = USER_PHYS_MEM_START - KERNEL_STACK_SIZE*(getExecuteTerm()+1) - (terminal_process_num[getExecuteTerm()]) * NUM_TERMINALS * KERNEL_STACK_SIZE - 4; // 4 to account for ebp

	// set esp ebp for next program, return to  it
	asm volatile(
	"	movl %0, %%esp		\n	\
		movl %1, %%ebp		\n 	\
	"
	: // no outputs
	: "rm"(terminal_esp[getExecuteTerm()]), "rm"(terminal_ebp[getExecuteTerm()])
	);
	return;
}

