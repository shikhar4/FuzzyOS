#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "paging.h"
#include "rtc.h"
#include "Terminal.h"
#include "filesys.h"

#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

/** 
 * NMI_interrupt
 * 
 * Asserts NMI_exception
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Spins if test case passes
 * Coverage: IDT
 * Files: handler.S, create_handler.c/h
 */ 
int NMI_interrupt() {
	asm("INT $0x2");
	return FAIL;
}

/** 
 * divide_by_zero
 * 
 * Asserts divide by zero exception
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Spins if test case passes
 * Coverage: IDT
 * Files: handler.S, create_handler.c/h
 */ 
int divide_by_zero() {
	int x = 1;
	int y = 0;
	x = x/y;
	return FAIL;
}

/** 
 * overflow
 * 
 * Asserts overflow exception
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Spins if test case passes
 * Coverage: IDT
 * Files: handler.S, create_handler.c/h
 */ 
int overflow() {
	// uint8_t x = 2;
	// uint8_t y = -2;
	// x = x - y;
	asm("INT $0x04");
	return FAIL;
}

/** 
 * systemcall_test
 * 
 * Asserts system call
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Spins if test case passes
 * Coverage: IDT
 * Files: handler.S, create_handler.c/h
 */ 
int systemcall_test() {
	asm("INT $0x80");
	return FAIL;
}

/** 
 * rtc_test
 * 
 * Tests RTC
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Prints dynamic weird character screen
 * Coverage: IDT, RTC
 * Files: handler.S, create_handler.c/h, rtc.c/h
 */ 
// int rtc_test() {
// 	init_rtc();
// 	return FAIL;
// }

/* Paging tests */

/** 
 * deref_null_ptr
 * 
 * Asserts that dereferencing a null pointer results in page fault
 * Inputs: None
 * Outputs: FAIL
 * Side Effects: Spins if test case passes
 * Coverage: Paging correctly initializes unused memory as not present
 * Files: paging.h/c, set_paging_registers.S
 */ 
int deref_null_ptr() {
	int* null_ptr = (int*)0x0;
	int test = *null_ptr; /* Page fault should happen here */
	test += 1; // This gets rid of unused variable warning
	assertion_failure();
	return FAIL;
}

/** 
 * deref_random_bad_ptr
 * 
 * Asserts that dereferencing a non-null bad pointer results in page fault
 * Inputs: None
 * Outputs: FAIL
 * Side Effects: Spins if test case passes
 * Coverage: Paging correctly initializes unused memory as not present
 * Files: paging.h/c, set_paging_registers.S
 */ 
int deref_random_bad_ptr() {
	int* rand_ptr = (int*)0xFFF00000;
	int test = *rand_ptr; /* Page fault should happen here */
	test += 1; // This gets rid of unused variable warning
	assertion_failure();
	return FAIL;
}

/** 
 * deref_kernel_ptr
 * 
 * Asserts that dereferencing a valid kernel pointer does not page fault
 * Inputs: None
 * Outputs: PASS
 * Side Effects: Page faults and spins if fails
 * Coverage: Paging correctly initializes kernel memory
 * Files: paging.h/c, set_paging_registers.S
 */
int deref_kernel_ptr() {
	// Kernel memory starts at 0x400000, so 0x400010 is inside of the 4MB-8MB kernel memory range
	int* kernel_ptr = (int*)0x400010;
	int kernel_mem = *kernel_ptr; /* This should be fine, dereference valid kernel memory */
	kernel_mem += 1; // This gets rid of unused variable warning
	return PASS;
}

/** 
 * deref_kernel_ptr_on_edge
 * 
 * Asserts that dereferencing a valid kernel pointer on the edge of valid memory does not page fault
 * Inputs: None
 * Outputs: PASS
 * Side Effects: Page faults and spins if fails
 * Coverage: Paging correctly initializes kernel memory
 * Files: paging.h/c, set_paging_registers.S
 */
int deref_kernel_ptr_on_edge() {
	// Kernel memory starts at 0x400000, this is the first valid kernel memory value
	int* kernel_ptr = (int*)0x400000;
	int kernel_mem = *kernel_ptr; /* This should be fine, dereference valid kernel memory */
	kernel_mem += 1; // This gets rid of unused variable warning
	return PASS;
}

/** 
 * deref_video_mem
 * 
 * Asserts that dereferencing a valid video memory pointer does not page fault
 * Inputs: None
 * Outputs: PASS
 * Side Effects: Page faults and spins if fails
 * Coverage: Paging correctly initializes video memory
 * Files: paging.h/c, set_paging_registers.S
 */
int deref_video_mem() {
	// Video memory starts at 0xB8000, so 0xB8010 is inside of the 4KB video memory range
	int* video_ptr = (int*)0xB8010;
	int video_mem = *video_ptr; /* This should be fine, dereference valid video memory */
	video_mem += 1; // This gets rid of unused variable warning
	return PASS;
}

/** 
 * deref_video_mem_on_edge
 * 
 * Asserts that dereferencing a valid video memory pointer on the edge of valid memory does not page fault
 * Inputs: None
 * Outputs: PASS
 * Side Effects: Page faults and spins if fails
 * Coverage: Paging correctly initializes video memory
 * Files: paging.h/c, set_paging_registers.S
 */
int deref_video_mem_on_edge() {
	// Video memory starts at 0xB8000, so this is the first valid video memory
	int* video_ptr = (int*)0xB8000;
	int video_mem = *video_ptr; /* This should be fine, dereference valid video memory */
	video_mem += 1; // This gets rid of unused variable warning
	return PASS;
}

/* End of paging tests */


/* Checkpoint 2 tests */
/** 
 * rtc_test
 * Opens RTC and prints 1s to the screen at frequencies from 2 to 1024, increasing by power of 2 each time.
 * Inputs: none
 * Outputs: pass
 * Side Effects: prints to screen
 * Coverage: RTC open, read/write, close
 * Files: rtc.c/h
 */ 
int rtc_test() {
	// int32_t fd = 0;
	// const uint8_t filename = 1;
	// int32_t buf = 2;
	// int i;
	// // change_rtc_freq(&buf);
	// RTC_open(fd, &filename);
	// for (i = 0; i < 10; i++) {
	// 	count = 0;
	// 	RTC_write(fd, (void*) &buf, 4);				// write new RTC freq
	// 	printf("Changed rtc freq to %d: ", buf);
	// 	while (count < 30) {
	// 		RTC_read(fd, (void*) &buf, 4);			// read RTC (will wait until interrupt happens)
	// 		putc('1');
	// 	}
	// 	putc('\n');
	// 	buf *= 2;
	// }
	// RTC_close(fd);
	return PASS;
}

/** 
 * terminal_open_test
 * Runs terminal open
 * Inputs: none
 * Outputs: pass
 * Side Effects: clears screen 
 * Coverage: terminal_open
 * Files: terminal.c/h
 */ 
int terminal_open_test() {
	int32_t fd = 0;
	const uint8_t filename = 1;
	printf("terminal_open");					// should not see terminal_open message on screen
	terminal_open(fd, &filename);
	return PASS;
}

/** 
 * terminal_close_test
 * Runs terminal close
 * Inputs: none
 * Outputs: pass
 * Side Effects: clears screen
 * Coverage: terminal_close
 * Files: terminal.c/h
 */ 
int terminal_close_test() {
	int32_t fd = 0;
	printf("terminal_close\n");
	terminal_close(fd);							// should not see terminal_close message on screen
	return PASS;
}

/** 
 * terminal_test
 * Writes a buffer to screen, reads user input, then prints user input back 
 * Inputs: none
 * Outputs: pass
 * Side Effects: writes things to terminal
 * Coverage: terminal read/write
 * Files: terminal.c/h
 */ 
int terminal_test() {
	int32_t fd = 0;
	const int buf_size = 300;
	char buf[buf_size];
	int i;
	for (i = 0; i < buf_size; i++) {
		buf[i] = 'c';
		if (i%2) 
			buf[i] = '\0';
		if (i%3)
			buf[i] = ' ';
		
	}
	// buf[0] = 'c';
	buf[127] = '\n';
	terminal_write(fd, (void*) &buf, buf_size);		// write current buffer to screen
	terminal_read(fd, (void*) buf, buf_size);		// read user input
	terminal_write(fd, (void*) buf, buf_size);		// write keyboard buffer
	
	return PASS;
}

// /* Start file system tests */

// /** 
//  * file_read_frame0_test
//  * 
//  * Asserts that we are able to read a small file from memory
//  * Inputs: None
//  * Outputs: PASS/FAIL
//  * Side Effects: None
//  * Coverage: File system can open and read files
//  * Files: filesys.h/c
//  */
// int file_read_frame0_test() {
// 	int i;
// 	file_open("frame0.txt");

// 	int fileSize = getFileLen();

// 	if (fileSize < 0){
// 		printf("bad file \n");
// 		return FAIL;
// 	}

// 	char buf[fileSize];

// 	file_read(0x0, buf, fileSize);

// 	printf("file read\n");
// 	for (i=0; i < fileSize; i++) {
// 		if (buf[i] == 0x0) continue;
// 		putc(buf[i]);
// 	}
// 	putc('\n');

// 	if (strncmp("/\\/\\", buf, 4) != 0) {
// 		return FAIL;
// 	}
// 	return PASS;
// }

// /** 
//  * file_read_verylarge_test
//  * 
//  * Asserts that we are able to read a large file with long name from memory
//  * Inputs: None
//  * Outputs: PASS/FAIL
//  * Side Effects: None
//  * Coverage: File system can open and read files
//  * Files: filesys.h/c
//  */
// int file_read_verylarge_test() {
// 	int i;
// 	file_open("verylargetextwithverylongname.txt");

// 	int fileSize = getFileLen();

// 	if (fileSize < 0){
// 		printf("bad file \n");
// 		return FAIL;
// 	}

// 	char buf[fileSize];

// 	file_read(0x0, buf, fileSize);

// 	printf("file read \n");
// 	for (i=0; i < fileSize; i++) {
// 		if (buf[i] == 0x0) continue;
// 		putc(buf[i]);
// 	}
// 	putc('\n');

// 	if (strncmp("very", buf, 4) != 0) {
// 		return FAIL;
// 	}
// 	return PASS;
// }

// /** 
//  * file_read_executable_test
//  * 
//  * Asserts that we are able to read a large executable from memory
//  * Inputs: None
//  * Outputs: PASS/FAIL
//  * Side Effects: None
//  * Coverage: File system can open and read files
//  * Files: filesys.h/c
//  */
// int file_read_executable_test() {
// 	int i;
// 	file_open("grep");

// 	int fileSize = getFileLen();

// 	if (fileSize < 0){
// 		printf("bad file \n");
// 		return FAIL;
// 	}

// 	char buf[fileSize];

// 	file_read(0x0, buf, fileSize);

// 	printf("file read \n");
// 	for (i=0; i < 100; i++) {
// 		if (buf[i] == 0x0) continue;
// 		putc(buf[i]);
// 	}
// 	putc('\n');

// 	if (strncmp("ELF", buf, 4) != 0) {
// 		return FAIL;
// 	}
// 	return PASS;
// }

// /** 
//  * multiple_file_reads_test
//  * 
//  * Asserts that we are able to read an open file multiple times, with the
//  * second time starting where the first left off.
//  * Inputs: None
//  * Outputs: PASS
//  * Side Effects: None
//  * Coverage: File system can open and read files
//  * Files: filesys.h/c
//  */
// int multiple_file_reads_test() {
// 	int i;

// 	file_open("frame1.txt");
// 	int fileSize = getFileLen();

// 	if (fileSize < 0){
// 		printf("bad file \n");
// 		return FAIL;
// 	}

// 	char buf[fileSize];
// 	file_read(0x0, buf, 3*fileSize/4);

// 	clear();

// 	printf("Read first 3/4 \n");
// 	for (i=0; i < 3*fileSize/4; i++) {
// 		if (buf[i] == 0x0) continue;
// 		putc(buf[i]);
// 	}
// 	putc('\n');
// 	putc('\n');
	
// 	file_read(0x0, buf+ 3*fileSize/4, fileSize/4);

// 	printf("Read last quarter \n");
// 	for (i= 3*fileSize/4; i < fileSize; i++) {
// 		if (buf[i] == 0x0) continue;
// 		putc(buf[i]);
// 	}
// 	putc('\n');

// 	return PASS;
// }

// /** 
//  * dir_read_test
//  * 
//  * Asserts that we are able to read a directory, outputting files with
//  * filename, file type, and file size.
//  * Inputs: None
//  * Outputs: PASS
//  * Side Effects: None
//  * Coverage: File system can open and read directories
//  * Files: filesys.h/c
//  */
// int dir_read_test() {
// 	int i;
// 	int j;
// 	char buf[200];

// 	for (i=0; i < MAX_DENTRIES; i++) {
// 		if (dir_read(0x0, buf, 0) == -1) {
// 			break;
// 		}
// 		for (j=0; j < 200; j++) {
// 			if (buf[j] == 0x0) break;
// 			putc(buf[j]);
// 		}
// 		putc('\n');
// 	}
// 	return PASS;
// }
/* End filesystem tests */


/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	/* Checkpoint 1 Tests */

	// TEST_OUTPUT("idt_test", idt_test());
	// TEST_OUTPUT("NMI_interrupt", NMI_interrupt());
	// TEST_OUTPUT("divide_by_zero", divide_by_zero());
	// TEST_OUTPUT("overflow", overflow());
	TEST_OUTPUT("systemcall", systemcall_test());
	// TEST_OUTPUT("rtc_test", rtc_test());

	// TEST_OUTPUT("dereference null pointer", deref_null_ptr());
	// TEST_OUTPUT("dereference random bad pointer", deref_random_bad_ptr());
	// TEST_OUTPUT("dereference kernel pointer", deref_kernel_ptr());
	// TEST_OUTPUT("dereference kernel pointer on edge", deref_kernel_ptr_on_edge());
	// TEST_OUTPUT("dereference video memory pointer", deref_video_mem());
	// TEST_OUTPUT("dereference video memory on edge", deref_video_mem_on_edge());


	/* Checkpoint 2 Tests */
	// TEST_OUTPUT("rtc_test", rtc_test());
	// TEST_OUTPUT("terminal_open_test", terminal_open_test());
	// TEST_OUTPUT("terminal_close_test", terminal_close_test());
	// TEST_OUTPUT("terminal_test", terminal_test());

	// TEST_OUTPUT("read file", file_read_frame0_test());
	// TEST_OUTPUT("read file", file_read_verylarge_test());
	// TEST_OUTPUT("read file", file_read_executable_test());
	// TEST_OUTPUT("read directory", dir_read_test());
	// TEST_OUTPUT("read file multiple times", multiple_file_reads_test());

	
	// launch your tests here
}
