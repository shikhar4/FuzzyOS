
#include "rtc.h"

// initialize rtc interrupt flag
static volatile int32_t rtc_interrupt_flag = 0;

static volatile int32_t terminal_rtc[NUM_TERMINALS] = {0, 0, 0};// terminal_rtc[3]
static volatile int32_t rtc_counter[NUM_TERMINALS] = {0, 0, 0}; // rtc_counter[3]
static int32_t rtc_initialized = 0;

#define RTC_DATA    0x71
#define RTC_REG     0x70
#define RTC_REG_B   0x8B
#define RTC_REG_A   0x8A
#define ODD         0x1
#define MAX_FREQ    1024

// fops_t rtc = {&RTC_open, &RTC_read, &RTC_write, &RTC_close};

/** 
 * init_rtc
 * 
 * Description: Initializes RTC 
 * Inputs: none
 * Outputs: none
 * Side Effects: Enables port 2 on master (slave port) and port 0 on slave
 */
void init_rtc() {
    enable_irq(2);
    enable_irq(8);                      // enable slave port 0, master port 2 (slave port)
    outb(RTC_REG_B, RTC_REG);           // disable NMI (8) and select register B
    char prev = inb(RTC_DATA);          // read current val of reg B from rtc data port
    outb(RTC_REG_B, RTC_REG); 
    outb(prev | 0x40, RTC_DATA);        // write prev value ORed w/0x40, which sets bit 6 of register B
    rtc_interrupt_flag = 0;
}

/** 
 * rtc_irq
 * 
 * Description: Processes the interrupt request from irq
 * Inputs: none
 * Outputs: none
 * Side Effects: prints stuff from test_interrupts on screen, should by dynamic
 */
void rtc_irq() {
    // cli();
    
    // test_interrupts();
    // test_rtc_video();
    outb(0x0C, RTC_REG);	        // select register C
    inb(RTC_DATA);                  // read register C, even though we don't care about the value,
    // sti();                          so that another interrupt can occur. if you don't read, then no more rtc
                                    // interrupts can happen
    send_eoi(8);                    // send eoi of slave 0
    rtc_interrupt_flag = 1;
    
}

/** 
 * change_rtc_frequency
 * Description: Helper function that changes rtc frequency
 * Inputs: new_feq
 * Outputs: none
 * Side Effects: writes to the RTC and changes the frequency
 */
void change_rtc_freq(int32_t * new_freq) {
    int32_t freq = *new_freq;
	if (freq & ODD || freq < 0) {					    // if not power of 2 or negative, can't change the freq
		return;	
    }
	if (freq > MAX_FREQ) {					// limit max freq to 1024
		freq = MAX_FREQ;
    }
	
	terminal_rtc[getExecuteTerm()] = (int32_t)MAX_FREQ / freq; // terminal_rtc[terminal_number] = ...
	// printf("freq: %d	terminal_rtc: %d\n", freq, terminal_rtc);
	if (rtc_initialized)
		return;
    int32_t rate;
    switch(freq) {
        case 2:             // frequency = 32768 >> (rate - 1)
            rate = 0xF;     // set rate (bit shift) based on frequency (which must be power of 2)
            break;          // rate = 15 for 2 Hz
        case 4:
            rate = 0xE;     // rate = 14 for 4 Hz
            break;
        case 8:
            rate = 0xD;     // rate = 13 for 8 Hz
            break;
        case 16:
            rate = 0xC;     // rate = 12 for 16 Hz
            break;
        case 32:
            rate = 0xB;     // rate = 11 for 32 Hz
            break;
        case 64:
            rate = 0xA;     // rate = 10 for 64 Hz
            break;
        case 128:
            rate = 0x9;     // rate = 9 for 128 Hz
            break;
        case 256:
            rate = 0x8;     // rate = 8 for 256 Hz
            break;
        case 512:
            rate = 0x7;     // rate = 7 for 512 Hz
            break;
        case 1024:
            rate = 0x6;     // rate = 6 for 1024 Hz
            break;
        default:
            rate = 0xF;     // set default as 2 Hz (rate = 15)
    }
    
	cli();
	outb(RTC_REG_A, RTC_REG);					// set index to register A, disable NMI
	char prev = inb(RTC_DATA);				    // get initial value of register A
	outb(RTC_REG_A, RTC_REG);					// reset index to A
	outb((prev & 0xF0) | rate, RTC_DATA); 	    //write only our rate to A. Note, rate is the bottom 4 bits, which is why we & with 0xF0

	sti();

}

/** 
 * RTC_open
 * Description: opens the RTC
 * Inputs: fd, filename
 * Outputs: returns 0 on success
 * Side Effects: sets RTC frequency to 1024
 */
int32_t RTC_open(int fd, const uint8_t* filename){
    int32_t freq = 1024;
    change_rtc_freq(&freq);
	rtc_initialized = 1;
    pcb_t* pcb = getPCB();			// calculate current process pcb

    pcb->fda[fd].fops = (uint32_t *) RTC_fops;              // update fda entry 
    pcb->fda[fd].inode = -1;
    pcb->fda[fd].file_position = -1;
    pcb->fda[fd].flags = 1;
    return 0;                                               // return success
}

/** 
 * RTC_read
 * Description: Waits for an RTC interrupt to occur, then returns
 * Inputs: fd, buf, nbytes
 * Outputs: return 0 on success
 * Side Effects: Increments counter for test cases
 */
int32_t RTC_read(int32_t fd, void* buf, int32_t nbytes){
    rtc_interrupt_flag = 0;
	rtc_counter[getExecuteTerm()] = 0;
	while(rtc_counter[getExecuteTerm()] < terminal_rtc[getExecuteTerm()]/2) {			// wait for interrupt
		while (!rtc_interrupt_flag);				// terminal_rtc[terminal_num]
        rtc_interrupt_flag = 0;
		rtc_counter[getExecuteTerm()]++;
    	// count++;                                // increment count and reset interrupt flag

    }
	// printf("%x, %x, %x", rtc_counter, rt)
    
    return 0; 								// return 0 when an interrupt happens
}

/** 
 * RTC_write
 * Description: Writes a frequency to RTC
 * Inputs: fd, buf, nbytes
 * Outputs: number of bytes written (always 4)
 * Side Effects: Returns error if invalid buf or nbytes != 4
 */
int32_t RTC_write(int32_t fd, const void* buf, int32_t nbytes){

   	if (nbytes != 4)			            // if number of bytes is not 4, return failure
		return -1;
	if (!buf)                               // if buffer not valid, return failure
        return -1;

	change_rtc_freq((int32_t *) buf);		// update rtc frequency and return # of bytes written
   	return 4;
}

/** 
 * RTC_close
 * Description: Close RTC function
 * Inputs: fd
 * Outputs: return 0 always
 * Side Effects: none
 */
int32_t RTC_close(int32_t fd){
    return 0;                               // return success
}


