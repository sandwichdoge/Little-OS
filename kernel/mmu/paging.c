#include "kinfo.h"
#include "kheap.h"
#include "utils/debug.h"
extern void loadPageDirectory(void* page_directory);
extern void enablePaging();

#define PAGE_SIZE 0x1000 // 4096
#define PDE_SIZE 0x400000

// Page index 123 in table index 456 will be mapped to (456 * 1024) + 123 = 467067. 467067 * 4 = 1868268 KiB.

unsigned int kernel_page_directory[1024] __attribute__((aligned(4096))); // 1024 page tables in page directory

static unsigned int virtual_addr_to_pde(unsigned int virtual_addr) {
	return virtual_addr >> 22;
}

// Map 1 page table (4MiB) from virtual address to phys_addr.
static void paging_map(unsigned int virtual_addr, unsigned int phys_addr, unsigned int *page_dir, unsigned int *page_table) {
	// Populate the page table.
    for (unsigned int i = 0; i < 1024; i++) {
		// We can fit all addreses of 4GB physical memory into a PTE.
		// Since the page must be 4kB aligned, last 12 bits are always zeroes, so x86 uses them as access bits cleverly.
		page_table[i] = (phys_addr + (i * 0x1000)) | 3;
	}
	
	unsigned int pde = virtual_addr_to_pde(virtual_addr);
	page_dir[pde] = ((unsigned int)page_table - 0xc0000000) | 3;
}

void paging_init() {
	struct kinfo *kinfo = get_kernel_info();

	//unsigned int phys_mem_size_kb = (kinfo->phys_mem_upper - kinfo->phys_mem_lower); // e.g 31680
	unsigned int pages_total = kinfo->phys_mem_upper / 4;
	unsigned int page_tables_total = (pages_total + (1024 - 1)) / 1024; // Round up int division

	unsigned int *page_tables = kmalloc_align(1024 * page_tables_total, 4096);

	for (unsigned int i = 0; i < page_tables_total; i++) {
        paging_map(0xc0000000 + i * PDE_SIZE, i * PDE_SIZE, kernel_page_directory, page_tables + 1024 * i);
    }

	unsigned int kernel_page_directory_phys = (unsigned int)kernel_page_directory - 0xc0000000;

	// loadPageDirectory() only accepts physical addresses.
    loadPageDirectory((void*)kernel_page_directory_phys);
    enablePaging();
}
