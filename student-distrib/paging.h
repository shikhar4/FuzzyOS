#ifndef _PAGING_H
#define _PAGING_H

#include "types.h"
#include "lib.h"
#include "system_call.h"

#define KB_SIZE 1024
#define BITMASK_10BIT 0x3FF
#define BITMASK_12BIT 0xFFF

#define VIDEO       0xB8000 // redefining from lib.c

#define P   0x00000001
#define RW  0x00000002
#define US  0x00000004
#define PWT 0x00000008
#define PCD 0x00000010
#define A   0x00000020
#define RES6    0x00000040
#define D       0x00000040
#define PS      0x00000080
#define PAT     0x00000080
#define G       0x00000100
#define AVAIL   0x00000E00


// Page directory, total memory 4KB aligned to 4KB to preserve 12 zeros on end
int32_t page_directory[KB_SIZE] __attribute__((aligned (4*KB_SIZE)));

// Video page table, total memory 4KB aligned to 4KB to preserve 12 zeros on end
int32_t video_page_table[KB_SIZE] __attribute__((aligned (4*KB_SIZE)));

// Initializes paging
void init_paging();

// Sets control registers to initialize paging 
void set_paging_registers();

// remove page of user process ending
void remove_page(uint32_t virtual_addr);

// remove 4kb page
void remove_4kb_page(uint32_t virtual_addr);

// Set page table to point to new physical memory
void map_virtual_addr_to_physical(uint32_t virtual_addr, uint32_t physical_addr);

// Mark page for user program as present
void map_virtual_addr_to_physical_4kb_page(uint32_t virtual_addr, uint32_t physical_addr);

// Mark page for 4kb video page as present
void map_terminal_vidmem(uint32_t virtual_addr, uint32_t physical_addr);

#endif /* _PAGING_H */


