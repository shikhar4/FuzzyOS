#ifndef _RTC_H
#define _RTC_H

#include "lib.h"
#include "i8259.h"
#include "system_call.h"

// volatile int count;

//Initializes RTC
void init_rtc();

//Processes the interrupt request from irq
void rtc_irq();

// helper function that writes frequency to RTC
void change_rtc_freq(int * new_freq);

// initializes RTC frequency to 2
extern int32_t RTC_open(int fd, const uint8_t* filename);

// reads RTC by waiting for RTC interrupt to occur before returning
extern int32_t RTC_read(int32_t fd, void* buf, int32_t nbytes);

// writes frequency to RTC
extern int32_t RTC_write(int32_t fd, const void* buf, int32_t nbytes);

// just returns 0
extern int32_t RTC_close(int32_t fd);


#endif 
