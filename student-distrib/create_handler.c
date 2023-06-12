#include "create_handler.h"


/** 
 * init_idt
 * 
 * Description: Initializes IDT
 * Inputs: none
 * Outputs: none
 * Side Effects: fills IDT with function pointers to exceptions, interrupts, and system calls
 */
void init_idt() {
    void* func_pointers[EXCEPTION_SIZE]; // an array of function pointers to be called by the idt
    func_pointers[0] = &divideByZero;           //Exception number 0 - Divide by Zero
    func_pointers[1] = &debugException;         //Exception number 1 - Debug exception
    func_pointers[2] = &nonMaskableInterrupt;   //Exception number 2 - NMI interrupt
    func_pointers[3] = &breakpointException;    //Exception number 3 - Breakpoint exception
    func_pointers[4] = &overflowException;      //Exception number 4 - Overflow exception 
    func_pointers[5] = &boundRangeExceeded;     //Exception number 5 - Bound Range Exceeded Exception
    func_pointers[6] = &invalidOpcode;          //Exception number 6 - Invalid Opcode Exception
    func_pointers[7] = &deviceNotAvailable;     //Exception number 7 -  Device Not Available Exception
    func_pointers[8] = &doubleFault;            //Exception number 8 - Double Fault Exception
    func_pointers[9] = &coprocessorSegment;     //Exception number 9 - Coprocessor Segment Overrun Exception
    func_pointers[10] = &invalidTSS;            //Exception number 10 - Invalid TSS Exception
    func_pointers[11] = &segmentNotPresent;     //Exception number 11 - Segment Not Present Exception
    func_pointers[12] = &stackFault;            //Exception number 12 -  Stack Fault Exception
    func_pointers[13] = &generalProtection;     //Exception number 19 - SIMD Floating-Point Exception
    func_pointers[14] = &pageFault;             // Exception 14, page fault
    func_pointers[15] = 0;
    func_pointers[16] = &floatingPointError;    //Exception number 16 - x87 FPU Floating-Point Error
    func_pointers[17] = &alignmentCheck;        //Exception number 17 - Alignment Check Exception
    func_pointers[18] = &machineCheck;          //Exception number 18 - Machine-Check Exception
    func_pointers[19] = &SIMDfloatingPoint;     //Exception number 19 - SIMD Floating-Point Exception
    int i; 
    //populating the IDT setting the necessary bits
    for (i=0; i < NUM_VEC; i++) {
        // idt_desc_t vec;
        idt[i].present = 1;
        if (i == 0x80)
            idt[i].dpl = 3;
        else 
            idt[i].dpl = 0;
        idt[i].size = 1;
        idt[i].seg_selector = KERNEL_CS;
        idt[i].reserved0 = 0;
        idt[i].reserved1 = 1;
        idt[i].reserved2 = 1;
        idt[i].reserved3 = 0;
        idt[i].reserved4 = 0;
        // idt[i] = vec;
        SET_IDT_ENTRY(idt[i], func_pointers[i]);
    }
    
    // SET_IDT_ENTRY(idt[1], &debugException);
    SET_IDT_ENTRY(idt[0x20], pitInterrupt);
    SET_IDT_ENTRY(idt[0x21], keyboardInterrupt); //calling IDT for keyboard interrupt 
    SET_IDT_ENTRY(idt[0x28], rtcInterrupt); //calling IDT for RTC interrupt 
    SET_IDT_ENTRY(idt[0x80], systemCall); //calling IDT for system call

}

/** 
 * handler
 * 
 * Description: Prints corresponding exception message when exception is triggered
 * Inputs: except_num
 * Outputs: none
 * Side Effects: prints exception name to screen
 */
void handler(int except_num) {
    switch (except_num)
    {
    case 0x00: //Exception number 0 - Divide by Zero
        printf("Error: Divide by Zero");
        break;
    case 0x01: //Exception number 1 - Debug exception
        printf("Error: Debug Exception");
        break;
    case 0x02: //Exception number 2 - NMI interrupt
        printf("Error: NMI Interrupt");
        break;
    case 0x03: //Exception number 3 - Breakpoint exception
        printf("Error: Breakpoint Exception");
        break;
    case 0x04: //Exception number 4 - Overflow exception 
        printf("Error: Overflow Exception");
        break;
    case 0x05: //Exception number 5 - Bound Range Exceeded Exception
        printf("Error: Bound Range Exceeded Exception");
        break;
    case 0x06: //Exception number 6 - Invalid Opcode Exception
        printf("Error: Invalid Opcode Exception");
        break;
    case 0x07: //Exception number 7 -  Device Not Available Exception
        printf("Error: Device Not Available Exception");
        break;
    case 0x08: //Exception number 8 - Double Fault Exception
        printf("Error: Double Fault Exception");
        break;
    case 0x09: //Exception number 9 - Coprocessor Segment Overrun Exception
        printf("Error: Coprocessor Segment Overrun");
        break;
    case 0x0A: //Exception number 10 - Invalid TSS Exception
        printf("Error: Invalid TSS Exception");
        break;
    case 0x0B: //Exception number 11 - Segment Not Present Exception
        printf("Error: Segment Not Present");
        break;
    case 0x0C: //Exception number 12 -  Stack Fault Exception
        printf("Error: Stack Fault Exception");
        break;
    case 0x0D: //Exception number 13 - General Protection Exception
        printf("Error: General Protection Exception");
        break;
    case 0x0E: //Exception number 14 - Page Fault Exception
        // int CR2;
        // int ESP;

        // asm volatile(
        //     "movl %%cr2, %0;"
        //     "movl %%esp, %1;"

        //     : "=r"(CR2), "=r"(ESP)
        //     :
        //     :
        // );

        // printf("\n");
        // printf("");

        printf("Error: Page Fault Exception");
        break;
    case 0x10: //Exception number 16 - x87 FPU Floating-Point Error
        printf("Error: x87 FPU Floating-Point Error ");
        break;
    case 0x11: //Exception number 17 - Alignment Check Exception
        printf("Error: Alignment Check Exception");
        break;
    case 0x12: //Exception number 18 - Machine-Check Exception
        printf("Error: Machine-Check Exception");
        break;
    case 0x13: //Exception number 19 - SIMD Floating-Point Exception
        printf("Error: SIMD Floating-Point Exception");
        break;
    default: //Defualt for exception 
        printf("Default Error");
        break;
    }
    printf("\n");
    exception_flag = 1;
    halt(0);
}

/** 
 * page_fault_handler
 * 
 * Description: Shows additional information about the exit status at a page fault to assist in debugging
 * Inputs: except_num, error_code, eip
 * Outputs: none
 */
void page_fault_handler(int except_num, int error_code, int eip) {
    int CR2;
    int ESP;
    switch(except_num) {
        case 0x0E: //Exception number 14 - Page Fault Exception
            
            // get register values from assembly
            asm volatile(
                "movl %%cr2, %0;"
                "movl %%esp, %1;"

                : "=r"(CR2), "=r"(ESP)
            );

            printf("Error: Page Fault Exception \n");
            printf("Error code: %x", error_code);
            printf("\nEIP: %x ", eip);
            printf("\nCR2: %x ", CR2);
            printf("\nESP: %x ", ESP);
            
            break;
        default:
            printf("Default Error");
    }
    printf("\n");
    exception_flag = 1;
    halt(0);
}

/** 
 * irq_handler
 * 
 * Description: Prints corresponding interrupt message when interrupt triggered
 * Inputs: irq_num
 * Outputs: none
 * Side Effects: Spinsn after irq processedd
 */
void irq_handler(int irq_num) {
    switch (irq_num) {
        case 0x20: // PIT interrupt
            pit_irq();
            break;

        case 0x21: // Interrupt number 33 - Keyboard interrupt 
            get_keyboard_input(); //calls the function to get user input
            break;

        case 0x28: //Interrupt number 40 - RTC interrupt 
            rtc_irq(); //Calls the RTC function 
            break;
        
        default: //Default interrupt 
            printf("Default irq");
    }
}

