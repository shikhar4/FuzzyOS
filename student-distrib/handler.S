
.globl common_handler
.globl divideByZero, debugException
.globl nonMaskableInterrupt, breakpointException, overflowException
.globl boundRangeExceeded, invalidOpcode, deviceNotAvailable, doubleFault
.globl coprocessorSegment, invalidTSS, segmentNotPresent, stackFault, generalProtection
.globl pageFault, floatingPointError, alignmentCheck, machineCheck, SIMDfloatingPoint
.globl keyboardInterrupt, rtcInterrupt, pitInterrupt

/** 
 * common_handler
 * 
 * Description: Assembly linkage to C function for exceptions
 * Inputs: none
 * Outputs: exception number
 * Side Effects:
 */
common_handler:
    CLI
    
    pushal
    movl 32(%esp), %ebx             # get the exception number (pushal pushes 8 registers, 8 * 4 = 32)
        
    pushl %ebx                      # push exception number
    call handler 
    add $4, %esp                    # pops ebx

    popal
    add $4, %esp                    # pops exception number

    STI
    iret

/** 
 * divideByZero
 * 
 * Description: Sets up for common_handler
 * Inputs: none
 * Outputs: none
 * Side Effects: Pushes exception number to stack
 */
divideByZero:
    pushl $0x0
    jmp common_handler

/** 
 * debugException
 * 
 * Description: Sets up for common_handler
 * Inputs: none
 * Outputs: none
 * Side Effects: Pushes exception number to stack
 */
debugException:
    pushl $0x1
    jmp common_handler

/** 
 * nonMaskableInterrupt
 * 
 * Description: Sets up for common_handler
 * Inputs: none
 * Outputs: none
 * Side Effects: Pushes exception number to stack
 */
nonMaskableInterrupt:
    pushl $0x2
    jmp common_handler

/** 
 * breakpointException
 * 
 * Description: Sets up for common_handler
 * Inputs: none
 * Outputs: none
 * Side Effects: Pushes exception number to stack
 */
breakpointException:
    pushl $0x3
    jmp common_handler

/** 
 * overflowException
 * 
 * Description: Sets up for common_handler
 * Inputs: none
 * Outputs: none
 * Side Effects: Pushes exception number to stack
 */
overflowException:
    pushl $0x4
    jmp common_handler

/** 
 * boundRangeExceeded
 * 
 * Description: Sets up for common_handler
 * Inputs: none
 * Outputs: none
 * Side Effects: Pushes exception number to stack
 */
boundRangeExceeded:
    pushl $0x5
    jmp common_handler

/** 
 * invalidOpcode
 * 
 * Description: Sets up for common_handler
 * Inputs: none
 * Outputs: none
 * Side Effects: Pushes exception number to stack
 */
invalidOpcode:
    pushl $0x6
    jmp common_handler

/** 
 * deviceNotAvailable
 * 
 * Description: Sets up for common_handler
 * Inputs: none
 * Outputs: none
 * Side Effects: Pushes exception number to stack
 */
deviceNotAvailable:
    pushl $0x7
    jmp common_handler

/** 
 * doubleFault
 * 
 * Description: Sets up for common_handler
 * Inputs: none
 * Outputs: none
 * Side Effects: Pushes exception number to stack
 */
doubleFault:
    pushl $0x8
    jmp common_handler

/** 
 * coprocessorSegment
 * 
 * Description: Sets up for common_handler
 * Inputs: none
 * Outputs: none
 * Side Effects: Pushes exception number to stack
 */
coprocessorSegment:
    pushl $0x9
    jmp common_handler

/** 
 * invalidTSS
 * 
 * Description: Sets up for common_handler
 * Inputs: none
 * Outputs: none
 * Side Effects: Pushes exception number to stack
 */
invalidTSS:
    pushl $0xA
    jmp common_handler

/** 
 * segmentNotPresent
 * 
 * Description: Sets up for common_handler
 * Inputs: none
 * Outputs: none
 * Side Effects: Pushes exception number to stack
 */
segmentNotPresent:
    pushl $0xB
    jmp common_handler

/** 
 * stackFault
 * 
 * Description: Sets up for common_handler
 * Inputs: none
 * Outputs: none
 * Side Effects: Pushes exception number to stack
 */
stackFault:
    pushl $0xC
    jmp common_handler

/** 
 * generalProtection
 * 
 * Description: Sets up for common_handler
 * Inputs: none
 * Outputs: none
 * Side Effects: Pushes exception number to stack
 */
generalProtection:
    pushl $0xD
    jmp common_handler

/** 
 * pageFault
 * 
 * Description: Sets up for common_handler
 * Inputs: none
 * Outputs: none
 * Side Effects: Pushes exception number to stack
 */
save_error_code:    .long 0x0
save_eip:           .long 0x0

pageFault:
    // pushl 4(%esp)           // stored eip value, vector number, error code
    pushl %ebx
	movl 8(%esp), %ebx			        # save eip
	movl %ebx, save_eip
	movl 4(%esp), %ebx				    # save error code
	movl %ebx, save_error_code
    
	popl %ebx
    pushl $0xE							# push error code
    
    CLI
    
    pushal
    movl 32(%esp), %ebx             # get the exception number (pushal pushes 8 registers, 8 * 4 = 32)
    
	pushl save_eip
	pushl save_error_code
    pushl %ebx                      # push exception number
    call page_fault_handler 
    add $12, %esp                    # pops ebx

    popal
    add $4, %esp                    # pops exception number

    STI
    iret

/** 
 * floatingPointError
 * 
 * Description: Sets up for common_handler
 * Inputs: none
 * Outputs: none
 * Side Effects: Pushes exception number to stack
 */
floatingPointError:
    pushl $0x10
    jmp common_handler

/** 
 * alignmentCheck
 * 
 * Description: Sets up for common_handler
 * Inputs: none
 * Outputs: none
 * Side Effects: Pushes exception number to stack
 */
alignmentCheck:
    pushl $0x11
    jmp common_handler

/** 
 * machineCheck
 * 
 * Description: Sets up for common_handler
 * Inputs: none
 * Outputs: none
 * Side Effects: Pushes exception number to stack
 */
machineCheck:
    pushl $0x12
    jmp common_handler

/** 
 * SIMDfloatingPoint
 * 
 * Description: Sets up for common_handler
 * Inputs: none
 * Outputs: none
 * Side Effects: Pushes exception number to stack
 */
SIMDfloatingPoint:
    pushl $0x13
    jmp common_handler

/** 
 * keyboardInterrupt
 * 
 * Description: Sets up for irq_handler
 * Inputs: none
 * Outputs: none
 * Side Effects: Pushes exception number to stack
 */
keyboardInterrupt:
    pushal 
    pushl $0x21
    call irq_handler
    add $4, %esp 
    popal 

    iret
    
/** 
 * rtcInterrupt
 * 
 * Description: Sets up for irq_handler
 * Inputs: none
 * Outputs: none
 * Side Effects: Pushes exception number to stack
 */
rtcInterrupt:
    pushal 
    pushl $0x28
    call irq_handler
    add $4, %esp 
    popal 

    iret 

/** 
 * pitInterrupt
 * 
 * Description: Sets up for irq_handler
 * Inputs: none
 * Outputs: none
 * Side Effects: Pushes exception number to stack
 */
pitInterrupt:
    pushal
    pushl $0x20
    call irq_handler
    add $4, %esp 
    popal 

    iret 
