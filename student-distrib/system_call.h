#ifndef _SYSTEM_CALL_H
#define _SYSTEM_CALL_H

#include "types.h"
#include "x86_desc.h"
#include "lib.h"
#include "paging.h"
#include "filesys.h"
#include "rtc.h"
#include "Terminal.h"
#include "keyboard.h"

#define MAX_CMD_CHARS       128
#define USER_MEM_START              0x8000000
#define USER_PHYS_MEM_START                0x800000
#define USER_MEM_SIZE                0x400000
#define KERNEL_STACK_SIZE                0x2000
#define FDA_MIN_INDEX       2
#define FDA_MAX_INDEX       7
#define FDA_MAX_OF          8
#define MAX_PID             5
#define EXCEP_NUM           256
#define SYSCALL_FOPS_LEN    4
#define SPACE               0x20
#define NUM_TERMINALS       3
#define TERMINAL_1_VIDMEM   0xB9000
#define TERMINAL_2_VIDMEM   0xBA000
#define TERMINAL_3_VIDMEM   0xBB000

extern int32_t pid; // Current process ID
extern void * file_fops[SYSCALL_FOPS_LEN]; // 4 is the size of the fops table (read write open close for below) 
extern void * directory_fops[SYSCALL_FOPS_LEN];
extern void * RTC_fops[SYSCALL_FOPS_LEN];
extern void * terminal_fops[SYSCALL_FOPS_LEN];

extern volatile void * terminal_data[NUM_TERMINALS];
extern int32_t terminal_vidmem[NUM_TERMINALS];
extern int32_t colors[NUM_TERMINALS];

// extern volatile int cur_display_terminal;
// extern volatile int cur_execute_terminal;
extern int32_t terminal_process_num[NUM_TERMINALS];


typedef struct fda_entry_t {
    uint32_t* fops;
    uint32_t inode;
    uint32_t file_position;
    uint32_t flags;
} fda_entry_t;

typedef struct pcb_t {
    fda_entry_t fda[FDA_MAX_OF];

    int32_t pid;
    int32_t parent_pid;
    
    uint32_t parent_esp;
    uint32_t parent_ebp;

    int8_t* params;
    uint32_t status_flags;
    
    // add field for grep and rtc frequency
} pcb_t;

//End a process
int32_t halt(uint8_t status);

//Parse args, check file for executable, set up paging, load file to memory, create pcb/open fds, context switch to user program
int32_t execute(const uint8_t * command);

//System Call read, call read for the correct file type
int32_t read(int32_t fd, void * buf, int32_t nbytes);

//System Call  write for the correct file type
int32_t write(int32_t fd, const void * buf, int32_t nbytes);

//System Call open a file
int32_t open(const uint8_t * filename);

//System Call close a fd
int32_t close(int32_t fd);

// get args syscall, for programs with an argument
int32_t getargs(uint8_t * buf, int32_t nbytes);

// vidmap syscall, used in fish
int32_t vidmap(uint8_t ** screen_start);
int32_t set_handler(int32_t signum, void * handler_address);
int32_t sigreturn (void);

//clear cr3 using inline
void flush_tlb();

// flag to keep track of process exception
extern volatile int32_t exception_flag;

// flag to keep track of which terminal we are running
extern volatile int32_t terminal_num;

// handles system calls
void system_call_handler(int system_call_num);


// sets for Exception 0x80 (128)
extern void systemCall();

// change the currently displayed terminal
void change_display_terminal(int32_t new_terminal);

// boot 3 shells
void boot_terminals();

// cur execute term getter
int getExecuteTerm();

// cur execute term setter
void setExecuteTerm(int set);

// cur display term getter
int getDisplayTerm();

// cur display term setter
void setDisplayTerm(int set);

// cur execute term PCB getter
pcb_t * getPCB();

#endif 
