#include "system_call.h"


// keep track of current process
int32_t pid = -1;

// flag to keep track of process exception
volatile int32_t exception_flag = 0;

// file function pointer table, 4 is the size of each table
void * file_fops[4] = {&file_open, &file_close, &file_read, &file_write};

// directory function pointer table
void * directory_fops[4] = {&dir_open, &dir_close, &dir_read, &dir_write};

// RTC function pointer table
void * RTC_fops[4] = {&RTC_open, &RTC_close, &RTC_read, &RTC_write};

// terminal function pointer table
void * terminal_fops[4] = {&terminal_open, &terminal_close, &terminal_read, &terminal_write};

// create terminal array to store data of terminals
terminal_t terminal1, terminal2, terminal3;
volatile void * terminal_data[NUM_TERMINALS] = {&terminal1, &terminal2, &terminal3};
int32_t colors[NUM_TERMINALS] = {0x17, 0x4E, 0x9F};		// pretty colors we like

int32_t terminal_vidmem[NUM_TERMINALS] = {TERMINAL_1_VIDMEM, TERMINAL_2_VIDMEM, TERMINAL_3_VIDMEM};
// globals to keep track of which terminal is displayed/executed
int volatile cur_display_terminal = 0;
int volatile cur_execute_terminal = 0;

// keep track of pid for each terminal
int32_t terminal_process_num[NUM_TERMINALS] = {-1, -1, -1};


/** 
 * system_call_handler
 * 
 * Description: Prints system call received when system call triggered
 * Inputs: system_call_num
 * Outputs: none
 * Side Effects: Spins after receiving system call
 */
void system_call_handler(int system_call_num) {
    switch (system_call_num) {
        case 0x80: // System call; 0x80 = 128
            printf("System call received");
            break;

        default:
            printf("Default system call message");
            break;
    }
    while (1);
}

/** 
 * halt
 * 
 * Description: End a process
 * Inputs: Status of the process halt
 * Outputs: none
 * Side Effects: Restore Parent Data, Restore Parent Paging, Close FDs
 */
int32_t halt(uint8_t status) {
	int32_t new_status = (int32_t) status;			// cast status to 32 bits
    
    // Instead of pid this should probably be terminal_process_num[cur_execute_terminal]
	if (terminal_process_num[cur_execute_terminal] == 0) {									// if closing last process, restart shell
		pid--;
		// decrement terminal pid or something
        terminal_process_num[cur_execute_terminal]--;
        execute((uint8_t *) "shell");
		return 0;
	}

	if (exception_flag) 							// if exception flag set, then return user exception
		new_status = EXCEP_NUM;

    exception_flag = 0;

	pcb_t* pcb = getPCB();		// get current pcb
    
	int32_t i;
	pid--;
    terminal_process_num[cur_execute_terminal]--;

    
	for (i = FDA_MIN_INDEX; i < FDA_MAX_OF; i++) {	// Tear down current pcb
        // Close all files associated with pcb
        close(i);
        pcb->fda[i].fops = NULL;
        pcb->fda[i].inode = -1;
        pcb->fda[i].file_position = -1;
        pcb->fda[i].flags = 0;	
	}

	// 128 MB is the virtual address of the current user program
	remove_page(USER_MEM_START);						// disable current process page

    // Unmap current video memory
    remove_4kb_page(USER_MEM_START + USER_MEM_SIZE);
	
	// add flag to check for status (i.e. if exception was generated, want to squash user program)
	
	flush_tlb();
    // Restore parent page, pid has already been decremented
    map_virtual_addr_to_physical(USER_MEM_START, USER_PHYS_MEM_START + (cur_execute_terminal * USER_MEM_SIZE) + terminal_process_num[cur_execute_terminal] * NUM_TERMINALS * USER_MEM_SIZE);

	tss.ss0 = KERNEL_DS;
    tss.esp0 = USER_PHYS_MEM_START - KERNEL_STACK_SIZE*(cur_execute_terminal+1) - (terminal_process_num[cur_execute_terminal]) * NUM_TERMINALS * KERNEL_STACK_SIZE - 4;					// update tss, -4 for ebp

	uint32_t parent_ebp = pcb->parent_ebp;
	uint32_t parent_esp = pcb->parent_esp;

    // reset pcb
    pcb->pid = -1;
	pcb->parent_pid = -1;
	pcb->parent_esp = 0;
	pcb->parent_ebp = 0;

    // ((terminal_t *)terminal_data[cur_execute_terminal])->_buffer_loc = 0;

    // set parent ebp and esp. Set new status as the return value by loading into eax
	asm volatile (
		"	movl %2, %%eax			\n	\
			movl %1, %%esp			\n	\
			movl %0, %%ebp			\n	\
			jmp execute_return		\n	\
		"
		: /* outputs */
		: "rm"(parent_ebp), "rm"(parent_esp), "rm"(new_status) /* inputs */
		: "memory"
	);
	// 0 is fine, != 0 is broken 
    return -1;				// should not return, so return error if reaches this line
}

/** 
 * execute 
 * 
 * Description: Parse args, check file for executable, set up paging, load file to memory, create pcb/open fds, context switch to user program
 * Inputs: Command to run
 * Outputs: return value from executed program
 */
int32_t execute(const uint8_t * command) {
    if (command == NULL)
        return -1;
	pid++; // increment pid because we want to make pcd for the child process
    // Increment current pid
    if (pid > MAX_PID) { // don't allow a 7th process, allow pid 0-5
        pid--;
        printf("\nMax processes reached. Type exit, idiot, if not then have fun diddling your bits\n");
        return 0;
    }
    terminal_process_num[cur_execute_terminal]++;       // increment pid of current terminal

    // Set max args to 128
    int8_t parsed_command[MAX_CMD_CHARS];
    int8_t parsed_args[MAX_CMD_CHARS];
    
    int8_t* curChar = (int8_t *) command;
    uint32_t cmd_idx = 0;

	// Parse args
    while (1) {
        // Get rid of spaces
        while (*curChar == SPACE) { // 0x20 is ascii space
            curChar++;
        }

        // At character or null
        // This char is the first letter of a new arg, so add it to parsed_command array
        while(*curChar != NULL && *curChar != SPACE) { // 0x20 is ascii space and 0x0 is ascii null
            parsed_command[cmd_idx++] = *curChar;
            curChar++;
        }

        // Now we are either at space or null. 0x0 is ascii null
        if (*curChar == NULL) {
            parsed_command[cmd_idx++] = NULL;
            parsed_args[0] = NULL;
            break;
		} else {
            parsed_command[cmd_idx++] = NULL;

            // Get rid of spaces
            while(*curChar == SPACE) { // 0x20 is ascii space and 0x0 is ascii null
                curChar++;
            }
            strcpy(parsed_args, curChar);
        }

        break; // change this to parse multiple args

        // SHOULD NEVER GET HERE
    }

    // Check for executable
    dentry_t dentry; 
    if (read_dentry_by_name((uint8_t*)parsed_command, &dentry) != 0) {
        pid--;
        terminal_process_num[cur_execute_terminal]--;
        return -1;
    }

    uint32_t file_size = 0;
    file_size = get_file_len_by_dentry(dentry);
    uint8_t buf[4]; // buf to read file into, 4 Bytes to contain the 4 magic numbers to be read
    read_data(dentry.inodeNum, 0, buf, 4); // read 4 bytes, those contain the magic num

    // compare the magic 4 bytes to expected values from first 4 bytes of executable
    if (buf[0] != 0x7f || buf[1] != 0x45 || buf[2] != 0x4C || buf[3] != 0x46) {
		pid--;
        terminal_process_num[cur_execute_terminal]--;
        return -1;
    }

    // Get correct eip from executable
    // bytes 24-27 is 4 bytes
    uint8_t eip_bytes[4];
    uint32_t entry_point_ip;
    read_data(dentry.inodeNum, 24, eip_bytes, 4); // want the 4 bytes in the file at 24-27 for EIP
    entry_point_ip = *((uint32_t*)eip_bytes);


    // Set up paging
    // Virtual address is 128MB
    map_virtual_addr_to_physical(USER_MEM_START, USER_PHYS_MEM_START + (cur_execute_terminal * USER_MEM_SIZE) + (terminal_process_num[cur_execute_terminal] * NUM_TERMINALS * USER_MEM_SIZE));

    // flush TLB
    // flush_tlb();

    // The program image itself is linked to execute at virtual address 0x08048000
    // This accounts for the program offset we need to execute
    read_data(dentry.inodeNum, 0, (uint8_t*)0x08048000, file_size);

    // Create PCB
    pcb_t* pcb_start = getPCB();

    //set stdin and stdout in the fda at index 0 and 1 respectively
    pcb_start->fda[0].fops = (uint32_t*)terminal_fops;
    pcb_start->fda[0].flags = 1;
    pcb_start->fda[0].inode = 0;
    pcb_start->fda[0].file_position = 0;

    pcb_start->fda[1].fops = (uint32_t*)terminal_fops;
    pcb_start->fda[1].flags = 1;
    pcb_start->fda[1].inode = 0;
    pcb_start->fda[1].file_position = 0;

    // store args in pcb
    pcb_start->params = parsed_args;

    //set pid in the pcb
    pcb_start->pid = pid;
    pcb_start->parent_pid = pid - 1;


    // Prepare for Context Switch
    uint32_t temp_esp_val;
    uint32_t temp_ebp_val;
    // save kernel stack pointer
    asm volatile (
        "   movl %%esp, %0      \n  \
            movl %%ebp, %1      \n  \
        "
        : "=rm"(temp_esp_val), "=rm"(temp_ebp_val)/* no outputs */
        : /* inputs */
    );
    pcb_start->parent_esp = temp_esp_val; // temp esp val
    pcb_start->parent_ebp = temp_ebp_val;

    // set SS0 and ESP0 fields of TSS
	tss.ss0 = KERNEL_DS;
    tss.esp0 = USER_PHYS_MEM_START - KERNEL_STACK_SIZE*(cur_execute_terminal+1) - (terminal_process_num[cur_execute_terminal]) * NUM_TERMINALS * KERNEL_STACK_SIZE - 4; // 4 to account for ebp

    // Push IRET context to kernel stack
    uint32_t ret_val;
    uint32_t user_ds = USER_DS;
    uint32_t user_cs = USER_CS;
    int32_t user_esp = USER_MEM_START + USER_MEM_SIZE - 4; // 132 MB - 4, 4 to account for ebp
    asm volatile (
        "   movw %1, %%ds		\n  \
			pushl %1        	\n  \
			pushl %4        	\n  \
			pushfl          	\n  \
			popl %%eax			\n  \
			orl $0x200, %%eax 	\n  \
			pushl %%eax			\n  \
			pushl %2        	\n  \
			pushl %3        	\n  \
            iret            	\n  \
		execute_return:			\n	\
            leave               \n  \
			ret					\n	\
        "
        : "=rm"(ret_val) /* output */
        : "rm"(user_ds), "rm"(user_cs), "rm"(entry_point_ip), "rm" (user_esp) /* Inputs */
    );

    return ret_val;
}

/** 
 * read 
 * 
 * Description: System Call read, call read for the correct file type
 * Inputs: fd number, buffer to read to, number of bytes to read
 * Outputs: The number of bytes read, -1 is error
 */
int32_t read(int32_t fd, void * buf, int32_t nbytes) {
    if (!buf)               								// error checking for NULL buffer
		return -1;

    // can have up to 8 fds
	if (fd < 0 || fd > FDA_MAX_INDEX || fd == 1)				// check for negative fd and check if fd is stdout which is in fd index 1
		return -1;
	pcb_t* pcb_ptr = getPCB();	        // calculate address of current pcb
	if (pcb_ptr->fda[fd].flags == 0)
		return -1;

	
	uint32_t function = (uint32_t)pcb_ptr->fda[fd].fops[2]; // call read function (fops table index 2) for fops
	
	int32_t num_bytes_read = ((int32_t(*)())function)(fd, buf, nbytes);
    return num_bytes_read;
}

/** 
 * write 
 * 
 * Description: System Call  write for the correct file type
 * Inputs: fd number, buffer to read to, number of bytes to read
 * Outputs: The number of bytes read, -1 is error
 */
int32_t write(int32_t fd, const void * buf, int32_t nbytes) {
	if (!buf)												// error checking for NULL buffer
		return -1;

    // fds acceptable = 0-7
	if (fd < 1 || fd > FDA_MAX_INDEX)				// check for negative fd or check for stdin which is in fd index 0
		return -1;
	pcb_t* pcb_ptr = getPCB();			// calculate address of current pcb
	if (pcb_ptr->fda[fd].flags == 0)
		return -1;
	
	
	uint32_t function = (uint32_t)pcb_ptr->fda[fd].fops[3];	// call write function (index 3 of fops array) for fops
	
	int32_t num_bytes_read = ((int32_t(*)())function)(fd, buf, nbytes);
    return num_bytes_read;		
}

/** 
 * open 
 * 
 * Description: System Call open a file
 * Inputs: filename
 * Outputs: The index of the fd that holds the now open file, -1 is error
 */
int32_t open(const uint8_t * filename) {
	if (!filename)											// if invalid filename, return error
		return -1;
	
	dentry_t file;
	if (read_dentry_by_name(filename, &file))				// check if file exists
		return -1;

	pcb_t* pcb = getPCB();			// calculate current process pcb

    // index 0,1 are terminal and already loaded, check the other 6 locations to see if theres a spot for an fd
	int32_t i, fda_index = FDA_MIN_INDEX;
	for (i = FDA_MIN_INDEX; i < FDA_MAX_OF; i++) {								// open file and place in first empty slot of fda
		if (pcb->fda[i].flags == 0) {
			break;
		}
		fda_index++;
	}
	if (i == FDA_MAX_OF)
		return -1;											// no open spots in fda array, return error


	if (file.fileType == 0) {					// 0 - device
		if (RTC_open(fda_index, filename))
			return -1;
	} else if (file.fileType == 1){
        if (dir_open(fda_index, filename))		// 1 - directory
			return -1;
    } else if (file.fileType == 2) {
        if (file_open(fda_index, filename))		// 2 - normal file
			return -1;
    }

    return fda_index;
}

/** 
 * close
 * 
 * Description: System Call close a fd
 * Inputs: fd number
 * Outputs: 0, -1 is error
 */
int32_t close(int32_t fd) {
    // check for non terminal fd so 2-7
	if (fd < FDA_MIN_INDEX || fd > FDA_MAX_INDEX)				// check for negative fd
		return -1;
	
	pcb_t* pcb_ptr = getPCB();				// calculate address of current pcb
	
	if (pcb_ptr->fda[fd].flags == 0)
		return -1;
	
	uint32_t function = (uint32_t)pcb_ptr->fda[fd].fops[1];		// call close for correct file type

	pcb_ptr->fda[fd].fops = NULL;
    pcb_ptr->fda[fd].inode = -1;
    pcb_ptr->fda[fd].file_position = -1;
    pcb_ptr->fda[fd].flags = 0;									// reset flags in fda
	
    return ((int32_t(*)())function)(fd); // return 0 on success, -1 on failure
}

/** 
 * getargs
 * 
 * Description: Copies the arguments from the command line into the buffer
 * Inputs: buf, nbytes
 * Outputs: 0 on success, -1 on failure
 */
int32_t getargs(uint8_t * buf, int32_t nbytes) {
    pcb_t* pcb_ptr = getPCB();				// calculate address of current pcb

    if (buf == NULL || strlen(pcb_ptr->params) == 0 || strlen(pcb_ptr->params) > nbytes) {
        return -1;
    }

    strncpy((int8_t*)buf, pcb_ptr->params, nbytes);
    return 0;
}

/** 
 * vidmap
 * 
 * Description: Returns virtual address mapping to physical video memory with user permissions
 * Inputs: screen_start
 * Outputs: Video virtual memory on success, -1 on failure
 */
int32_t vidmap(uint8_t ** screen_start) {

    if ((int32_t) screen_start < USER_MEM_START || (int32_t) screen_start > USER_MEM_START+USER_MEM_SIZE) {
        return -1;
    }

    *screen_start = (uint8_t *) USER_MEM_START + USER_MEM_SIZE;

    // uint32_t vid_mem = VIDEO;

    // if (cur_display_terminal != cur_execute_terminal) {
    //     vid_mem = terminal_vidmem[cur_execute_terminal];
    // }

    // This will currently not show up immediately in video memory
    map_virtual_addr_to_physical_4kb_page((uint32_t)*screen_start, VIDEO);
    return USER_MEM_START+USER_MEM_SIZE;
}

/** 
 * set_handler
 * 
 * Description: OPTIONAL (for signals)
 * Inputs: signum, handler_address
 * Outputs: 0
 */
int32_t set_handler(int32_t signum, void * handler_address) {
    return 0;
}

/** 
 * sigreturn
 * 
 * Description: OPTIONAL (for signals)
 * Inputs: none
 * Outputs: 0
 */
int32_t sigreturn (void) {
    return 0;
}

/** 
 * flush_tlb
 * 
 * Description: clear cr3 using inline
 * Inputs: none
 * Outputs: none
 * Side Effects: TLB is cleared
 */
void flush_tlb() {
    asm volatile (
        "   movl %%cr3, %%eax      \n    \
            movl %%eax, %%cr3      \n    \
        "
        : /* no outputs */
        : /* no inputs */
        : "eax" /* clobbers eax */
    );
}

/** 
 * change_display_terminal
 * 
 * Description: switch currently displayed terminal
 * Inputs: none
 * Outputs: none
 * Side Effects: copy old vid mem to unique location, set physical VIDEO mem to new terminals
 */
void change_display_terminal(int32_t new_terminal) {
	terminal_t* old_terminal = ((terminal_t *)terminal_data[cur_display_terminal]);
	memcpy(old_terminal->_keyboard_buf, keyboard_buf, 127);
    old_terminal->_caps_flag = caps_flag;
	terminal_t* cur_terminal = ((terminal_t *)terminal_data[new_terminal]);
	ATTRIB = colors[new_terminal];
	memcpy(keyboard_buf, cur_terminal->_keyboard_buf, 127);
    caps_flag = cur_terminal->_caps_flag;

    cli();
	// save video memory into current terminal memory
	memcpy((uint32_t*)terminal_vidmem[cur_display_terminal], (uint32_t*) VIDEO, KERNEL_STACK_SIZE/2);

	// copy new terminal memory into video memory
	memcpy((uint32_t*) VIDEO, (uint32_t*) terminal_vidmem[new_terminal], KERNEL_STACK_SIZE/2);
    sti();
	// printf("old terminal: %d, new terminal: %d", old_terminal, new_terminal);
}

/** 
 * getExecuteTerm
 * 
 * Description: cur execute terminal getter
 * Inputs: none
 * Outputs: cur execute terminal
 */
int getExecuteTerm() {
    return cur_execute_terminal;
}

/** 
 * setExecuteTerm
 * 
 * Description: cur execute terminal setter
 * Inputs: none
 * Outputs: none
 */
void setExecuteTerm(int set) {
    cur_execute_terminal = set;
}

/** 
 * getDisplayTerm
 * 
 * Description: cur display terminal getter
 * Inputs: none
 * Outputs: display term
 */
int getDisplayTerm() {
    return cur_display_terminal;
}

/** 
 * setDisplayTerm
 * 
 * Description: cur display terminal getter
 * Inputs: none
 * Outputs: none
 */
void setDisplayTerm(int set) {
    cur_display_terminal = set;
}

/** 
 * getPCB
 * 
 * Description: getPCB using cur execute term
 * Inputs: none
 * Outputs: PCB
 */
pcb_t * getPCB() {
    return (pcb_t*)(USER_PHYS_MEM_START - KERNEL_STACK_SIZE*(getExecuteTerm()+1) - (terminal_process_num[getExecuteTerm()]) * NUM_TERMINALS * KERNEL_STACK_SIZE);
}
