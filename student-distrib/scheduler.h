#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include "system_call.h"

// round robin to switch execute terminal each PIT interrupt
void scheduler();

#endif
