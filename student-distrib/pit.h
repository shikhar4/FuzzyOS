#ifndef _PIT_H
#define _PIT_H

#include "lib.h"
#include "i8259.h"
#include "system_call.h"
#include "scheduler.h"

// enable the pit
void init_pit();

// process the interrupt from irq
void pit_irq();

#endif
