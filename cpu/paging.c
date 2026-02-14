#include "paging.h"
#include "../drivers/screen.h"

uint32_t page_directory[1024] __attribute__((aligned(4096)));
uint32_t first_page_table[1024] __attribute__((aligned(4096)));

void initialize_paging() {
    // Identity map the first 4MB
    for (int i = 0; i < 1024; i++) {
        // As the address is page aligned, it will always be 0xXXXXX000
        // 0x7 is (PRESENT | WRITABLE | USER)
        first_page_table[i] = (i * 4096) | 7;
    }

    // Set up the first entry in the page directory
    page_directory[0] = ((uint32_t)first_page_table) | 7;

    // Set the rest of the page directory to not present
    for (int i = 1; i < 1024; i++) {
        page_directory[i] = 0 | 2; // Not present, but writable
    }

    // Load the page directory address into CR3
    asm volatile("mov %0, %%cr3" : : "r"(page_directory));

    // Enable paging by setting the PG bit in CR0
    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    asm volatile("mov %0, %%cr0" : : "r"(cr0));
    
    kprint("Paging enabled.\n");
}
