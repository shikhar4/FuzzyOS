.globl set_paging_registers

// OSDev: load CR3 with the address of the page directory and to set the paging (PG) and protection (PE) bits of CR0. 
//  Note: setting the paging flag when the protection flag is clear causes a general protection exception.
// CR4 code to enable PSE (4 MiB pages)


/** 
 * set_paging_registers
 * 
 * Description: Sets control registers to initialize paging 
 * Inputs: none
 * Outputs: none
 * Side Effects: Writes to CR0, CR3, CR4 (control registers) in order to initialize paging.
 */
set_paging_registers:
    pushl %ebp
    movl %esp, %ebp

    // Set cr3 to point to page_directory in memory
    movl $page_directory, %eax
    movl %eax, %cr3
    movl %cr4, %eax
    // Set PSE (page size extension, index 4) bit and PGE (page global enabled, index 7) bit of CR4
    orl  0x00000090, %eax
    movl %eax, %cr4
    movl %cr0, %eax
    // Set PG (paging, index 31) and PE (protection, index 0) bits of CR0
    orl  $0x80000001, %eax
    movl %eax, %cr0

    leave
    ret

