#ifndef _HANDLER_H
#define _HANDLER_H

#include "types.h"
#include "x86_desc.h"
#include "lib.h"
#include "tests.h"
#include "keyboard.h"
#include "rtc.h"
#include "system_call.h"
#include "pit.h"


// initializes idt
void init_idt();

// handles exceptions
void handler(int except_num);

// handles interrupt requests
void irq_handler(int irq_num);

void page_fault_handler(int except_num, int error_code, int eip);

// x86 wrapper function (defined in handler.S) that calls handler
extern void common_handler();

// sets for Exception 0
extern void divideByZero();

// sets for Exception 1
extern void debugException();

// sets for Exception 2
extern void nonMaskableInterrupt();

// sets for Exception 3
extern void breakpointException();

// sets for Exception 4
extern void overflowException();

// sets for Exception 5
extern void boundRangeExceeded();

// sets for Exception 6
extern void invalidOpcode();

// sets for Exception 7
extern void deviceNotAvailable();

// sets for Exception 8
extern void doubleFault();

// sets for Exception 9
extern void coprocessorSegment();

// sets for Exception 10
extern void invalidTSS();

// sets for Exception 11
extern void segmentNotPresent();

// sets for Exception 12
extern void stackFault();

// sets for Exception 13
extern void generalProtection();

// sets for Exception 14
extern void pageFault();

// sets for Exception 16
extern void floatingPointError();

// sets for Exception 17
extern void alignmentCheck();

// sets for Exception 18
extern void machineCheck();

// sets for Exception 19
extern void SIMDfloatingPoint();

// sets for Interrupt 0x21(33)
extern void keyboardInterrupt();

// sets for Interrupt 0x28 (40)
extern void rtcInterrupt();

// sets for Interrupt 0x20
extern void pitInterrupt();


#endif /* _HANDLER_H */

