#include "paging.h"

/** 
 * init_paging
 * 
 * Description: Initializes paging
 * Inputs: none
 * Outputs: none
 * Side Effects: Calls set_paging_ registers, which writes to CR0, CR3, CR4 (control registers).
 *               Allocates memory for page_directory and video_page_table.
 */
void init_paging() {
    int i; /* Loop index */

    // Set up first page table
    page_directory[0] = ((uint32_t)video_page_table) | RW | P;
    
    // Set kernel 4MB page to second elem of page_directory
    // Kernel memory at 4MB = 0x400000
    page_directory[1] = 0x400000 | PS | RW | P | G | PCD;       // need this have U/S = 0, so moved above this for loop

    for (i = 0; i < KB_SIZE; i++){
        if (i > 1) {
            page_directory[i] = 0x00000000;
            // R, U, W, D, S
        }
        video_page_table[i] = 0x00000000;
    }

    // Set page table to point to video memory
    // Bit shift by 12 because page_table index bits are bits 12-21 (10 bits)
    video_page_table[VIDEO >> 12] = VIDEO | RW | P;

    set_paging_registers();

    map_terminal_vidmem(TERMINAL_1_VIDMEM, TERMINAL_1_VIDMEM);
    map_terminal_vidmem(TERMINAL_2_VIDMEM, TERMINAL_2_VIDMEM);
    map_terminal_vidmem(TERMINAL_3_VIDMEM, TERMINAL_3_VIDMEM);

}

/** 
 * remove_page
 * 
 * Description: Remove a page
 * Inputs: virtual address
 * Outputs: none
 * Side Effects: removes the present bit for the page being closed
 */
void remove_page(uint32_t virtual_addr){
    uint32_t pd_index = (virtual_addr >> 22) & 0x3FF; // 22 bit shift as the pd_index is the top 10 bits of the virtual addr, 0x3ff masks 10 bits
    page_directory[pd_index] = 0x00000000;
    flush_tlb();
}

/** 
 * remove_4kb_page
 * 
 * Description: Remove a page
 * Inputs: virtual address
 * Outputs: none
 * Side Effects: removes the present bit for the page being closed
 */
void remove_4kb_page(uint32_t virtual_addr){
    uint32_t pd_index = (virtual_addr >> 22) & 0x3FF; // 22 bit shift as the pd_index is the top 10 bits of the virtual addr, 0x3ff masks 10 bits
    uint32_t pt_index = (virtual_addr >> 12) & 0x3FF; // 12 bit shift and mask to get middle 10 bits
    page_directory[pd_index] = ((uint32_t)video_page_table) | RW | P | US;
    video_page_table[pt_index] = 0x00000000;
    flush_tlb();
}

/** 
 * map_virtual_addr_to_physical
 * 
 * Description: Mark page for user program as present
 * Inputs: physical address, virtual address 
 * Outputs: none
 * Side Effects: Add page for user program, set physical address and flag bits 
 */
void map_virtual_addr_to_physical(uint32_t virtual_addr, uint32_t physical_addr) {
    uint32_t pd_index = (virtual_addr >> 22) & 0x3FF; // 22 bit shift as the pd_index is the top 10 bits of the virtual addr, 0x3ff masks 10 bits
    page_directory[pd_index] = physical_addr | PS | RW | P | PCD | US;
    flush_tlb();
}

/** 
 * map_virtual_addr_to_physical_4kb_page
 * 
 * Description: Mark page for user program as present
 * Inputs: physical address, virtual address 
 * Outputs: none
 * Side Effects: Add page for user program, set physical address and flag bits 
 */
void map_virtual_addr_to_physical_4kb_page(uint32_t virtual_addr, uint32_t physical_addr) {
    uint32_t pd_index = (virtual_addr >> 22) & 0x3FF; // 22 bit shift as the pd_index is the top 10 bits of the virtual addr, 0x3ff masks 10 bits
    uint32_t pt_index = (virtual_addr >> 12) & 0x3FF; // 12 bit shift and mask to get middle 10 bits
    page_directory[pd_index] = ((uint32_t)video_page_table) | RW | P | US;
    video_page_table[pt_index] = physical_addr | RW | P | US;
    flush_tlb();
}

/** 
 * map_terminal_vidmem
 * 
 * Description: Mark page for 4kb video page as present
 * Inputs: physical address, virtual address 
 * Outputs: none
 * Side Effects: Add page for video, set physical address and flag bits 
 */
void map_terminal_vidmem(uint32_t virtual_addr, uint32_t physical_addr) {
    uint32_t pd_index = (virtual_addr >> 22) & 0x3FF; // 22 bit shift as the pd_index is the top 10 bits of the virtual addr, 0x3ff masks 10 bits
    uint32_t pt_index = (virtual_addr >> 12) & 0x3FF; // 12 bit shift and mask to get middle 10 bits
    page_directory[pd_index] = ((uint32_t)video_page_table) | RW | P;
    video_page_table[pt_index] = physical_addr | RW | P;
    flush_tlb();
}
