#define USER_CS     0x0023
#define USER_DS     0x002B
#define USER_ESP	0x83FFFFC



.globl systemCall
.globl SC_halt, SC_write, SC_execute, SC_read, SC_write, SC_open, SC_close, SC_getargs, SC_vidmap, SC_set_handler, SC_sigreturn
.globl file_open, file_close, file_read, file_write
.globl dir_open, dir_close, dir_read, dir_write
.globl RTC_open, RTC_read, RTC_write, RTC_close
.globl terminal_open, terminal_read, terminal_write, terminal_close

sysCall_eax_save:	.long 0x0 				// save eax

sysCall_ret_addr: 	.long 0x0 				// The return address of the program that called int 0x80

sysCall_num: 		.long 0x0				// sysCall num for checking if currently calling halt

/** 
 * systemCall
 * 
 * Description: Sets up for system_call_handler
 * Inputs: none
 * Outputs: none
 * Side Effects: Pushes exception number to stack
 */
systemCall:			// use TSS when you want to return from another process or something
    // save return address before diddling stack

	sti
	pushal 
	pushfl									// save registers and flags

    cmpl $1, %eax							// if less than 1, jump to error
	jl systemCall_error
	cmpl $10, %eax 							// if greater than 10, jump to error
	jg systemCall_error

	 
	pushl %edx 									//caller save registerrs
	pushl %ecx 
	pushl %ebx 
	call *systemCall_jump_table-4(, %eax, 4)	// jump to correct system call
	popl %ebx 									//pop registers									
	popl %ecx 
	popl %edx 
	cmpl $-1, %eax    							// if return value is -1, jump to error
	je systemCall_error
	movl %eax, sysCall_eax_save


systemCall_finish:
	popfl 										// pop flags and registers 
    popal
	movl sysCall_eax_save, %eax 				// save return value from call 
	iret

systemCall_error:
	popfl
    popal
	movl $-1, %eax							// error, return -1
	iret 


/** 
 * call number placed in EAX
 * first arg in EBX
 * second arg in ECX
 * third arg in EDX
 * protect regs from modification (push all regs)
 * return value placed in EAX if call returns
 *      -1 = error
 */ 

systemCall_jump_table:
	.long	halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn

