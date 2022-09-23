#include "mmu.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


// byte addressable memory
unsigned char RAM[RAM_SIZE];  


// OS's memory starts at the beginning of RAM.
// Store the process related info, page tables or other data structures here.
// do not use more than (OS_MEM_SIZE: 72 MB).
unsigned char* OS_MEM = RAM;  

// memory that can be used by processes.   
// 128 MB size (RAM_SIZE - OS_MEM_SIZE)
unsigned char* PS_MEM = RAM + OS_MEM_SIZE; 


// This first frame has frame number 0 and is located at start of RAM(NOT PS_MEM).
// We also include the OS_MEM even though it is not paged. This is 
// because the RAM can only be accessed through physical RAM addresses.  
// The OS should ensure that it does not map any of the frames, that correspond
// to its memory, to any process's page. 
int NUM_FRAMES = ((RAM_SIZE) / PAGE_SIZE);

// Actual number of usable frames by the processes.
int NUM_USABLE_FRAMES = ((RAM_SIZE - OS_MEM_SIZE) / PAGE_SIZE);

// To be set in case of errors. 
int error_no; 



void os_init() {
    // TODO student 
    // initialize your data structures.

    memset(RAM, 0, RAM_SIZE * sizeof(unsigned char));
}

// MY FUNCTIONS

void setMem(int pid, int page, int frame, int p, int e, int w, int r) {
    int pos = ((pid << 10) + page) << 2;
    int data = (frame << 4) + (p << 3) + (e << 2) + (w << 1) + r;

    unsigned int* posInt = (unsigned int*) (OS_MEM + pos);
    posInt[0] = data;
}

int getPid() {
    int start = 1 << 22;
    for(int i = 0; i < 100; i++) {
        int add = start + i;
        if(OS_MEM[add] == 0)  {
            OS_MEM[add] = 1;
            return add;
        }
    }

    return -1;
}

int getPfn() {
    int start = 1 << 23;
    for(int i = 0; i < (1 << 15); i++) {
        int add = start + i;
        if(OS_MEM[add] == 0) {
            OS_MEM[add] = 1;
            return add;
        }
    }

    return -1;
}

int getFrameAdd(int pid, int vAdd) {
    return 0;
}

// ------------------------------
// ----------------------------------- Functions for managing memory --------------------------------- //

/**
 *  Process Virtual Memory layout: 
 *  ---------------------- (virt. memory start 0x00)
 *        code
 *  ----------------------  
 *     read only data 
 *  ----------------------
 *     read / write data
 *  ----------------------
 *        heap
 *  ----------------------
 *        stack  
 *  ----------------------  (virt. memory end 0x3fffff)
 * 
 * 
 *  code            : read + execute only
 *  ro_data         : read only
 *  rw_data         : read + write only
 *  stack           : read + write only
 *  heap            : (protection bits can be different for each heap page)
 * 
 *  assume:
 *  code_size, ro_data_size, rw_data_size, max_stack_size, are all in bytes
 *  code_size, ro_data_size, rw_data_size, max_stack_size, are all multiples of PAGE_SIZE
 *  code_size + ro_data_size + rw_data_size + max_stack_size < PS_VIRTUAL_MEM_SIZE
 *  
 * 
 *  The rest of memory will be used dynamically for the heap.
 * 
 *  This function should create a new process, 
 *  allocate code_size + ro_data_size + rw_data_size + max_stack_size amount of physical memory in PS_MEM,
 *  and create the page table for this process. Then it should copy the code and read only data from the
 *  given `unsigned char* code_and_ro_data` into processes' memory.
 *   
 *  It should return the pid of the new process.  
 *  
 */
int create_ps(int code_size, int ro_data_size, int rw_data_size,
                 int max_stack_size, unsigned char* code_and_ro_data) 
{   
    // TODO student
    int pid = getPid();
    int vpn = 0;
    for(int i = 0; i < code_size/PAGE_SIZE; i++, vpn++) {
        int pfn = getPfn();
        setMem(pid, vpn, pfn, 1, 1, 0, 1);
        memcpy(PS_MEM + pfn * PAGE_SIZE, code_and_ro_data + i*PAGE_SIZE, sizeof(unsigned char) * PAGE_SIZE);
    }

    for(int i = 0; i < ro_data_size/PAGE_SIZE; i++, vpn++) {
        int pfn = getPfn();
        setMem(pid, vpn, pfn, 1, 0, 0, 1);
        memcpy(PS_MEM + pfn * PAGE_SIZE, code_and_ro_data + code_size + i*(PAGE_SIZE), sizeof(unsigned char) * PAGE_SIZE);
    }

    for(int i = 0; i < rw_data_size/PAGE_SIZE; i++, vpn++) {
        int pfn = getPfn();
        setMem(pid, vpn, pfn, 1, 0, 1, 1);
    }

    vpn = PS_VIRTUAL_MEM_SIZE - max_stack_size;
    for(int i = 0; i < max_stack_size/PAGE_SIZE; i++, vpn++) {
        int pfn = getPfn();
        setMem(pid, vpn, pfn, 1, 0, 1, 1);
    }

    return pid;
}

/**
 * This function should deallocate all the resources for this process. 
 * 
 */
void exit_ps(int pid) 
{
   // TODO student
}



/**
 * Create a new process that is identical to the process with given pid. 
 * 
 */
int fork_ps(int pid) {

    // TODO student:
    return 0;
}



// dynamic heap allocation
//
// Allocate num_pages amount of pages for process pid, starting at vmem_addr.
// Assume vmem_addr points to a page boundary.  
// Assume 0 <= vmem_addr < PS_VIRTUAL_MEM_SIZE
//
//
// Use flags to set the protection bits of the pages.
// Ex: flags = O_READ | O_WRITE => page should be read & writeable.
//
// If any of the pages was already allocated then kill the process, deallocate all its resources(ps_exit) 
// and set error_no to ERR_SEG_FAULT.
void allocate_pages(int pid, int vmem_addr, int num_pages, int flags) 
{
   // TODO student
   /*
    for all pages
        if frame is present -- that is it is used -- error
        else assign memory
   */
}



// dynamic heap deallocation
//
// Deallocate num_pages amount of pages for process pid, starting at vmem_addr.
// Assume vmem_addr points to a page boundary
// Assume 0 <= vmem_addr < PS_VIRTUAL_MEM_SIZE

// If any of the pages was not already allocated then kill the process, deallocate all its resources(ps_exit) 
// and set error_no to ERR_SEG_FAULT.
void deallocate_pages(int pid, int vmem_addr, int num_pages) 
{
   // TODO student
}

// Read the byte at `vmem_addr` virtual address of the process
// In case of illegal memory access kill the process, deallocate all its resources(ps_exit) 
// and set error_no to ERR_SEG_FAULT.
// 
// assume 0 <= vmem_addr < PS_VIRTUAL_MEM_SIZE
unsigned char read_mem(int pid, int vmem_addr) 
{
    // TODO: student
    int vpn = vmem_addr/PAGE_SIZE;
    int offset = (vmem_addr & ((1 << 12) - 1));

    int key = ((pid << 10) + vpn) << 2;
    unsigned int value = OS_MEM[key];

    if(is_present(value) && is_readable(value)) {
        int pfn = value >> 4;
        int address = pfn * PAGE_SIZE + offset;
        return PS_MEM[address];
    } else {
        error_no = ERR_SEG_FAULT;
        exit_ps(pid);
    }

    return 0;
}

// Write the given `byte` at `vmem_addr` virtual address of the process
// In case of illegal memory access kill the process, deallocate all its resources(ps_exit) 
// and set error_no to ERR_SEG_FAULT.
// 
// assume 0 <= vmem_addr < PS_VIRTUAL_MEM_SIZE
void write_mem(int pid, int vmem_addr, unsigned char byte) 
{
    // TODO: student
    int vpn = vmem_addr/PAGE_SIZE;
    int offset = (vmem_addr & ((1 << 12) - 1));

    int key = ((pid << 10) + vpn) << 2;
    unsigned int value = OS_MEM[key];

    // printf("%d %d %d %d %d\n", value, is_present(value), is_executable(value), is_writeable(value), is_readable(value));

    if(is_present(value) && is_writeable(value)) {
        int pfn = value >> 4;
        int address = pfn * PAGE_SIZE + offset;
        PS_MEM[address] = byte;
    } else {
        error_no = ERR_SEG_FAULT;
        exit_ps(pid);
    }
}





// ---------------------- Helper functions for Page table entries ------------------ // 

// return the frame number from the pte
int pte_to_frame_num(page_table_entry pte) 
{
    // TODO: student
    return 0;
}


// return 1 if read bit is set in the pte
// 0 otherwise
int is_readable(page_table_entry pte) {
    // TODO: student
    if((pte & 1) > 0) 
        return 1;
    return 0;
}

// return 1 if write bit is set in the pte
// 0 otherwise
int is_writeable(page_table_entry pte) {
    // TODO: student
    if((pte & 2) > 0)
        return 1;
    return 0;
}

// return 1 if executable bit is set in the pte
// 0 otherwise
int is_executable(page_table_entry pte) {
    // TODO: student
    if((pte & 4) > 0)
        return 1;
    return 0;
}


// return 1 if present bit is set in the pte
// 0 otherwise
int is_present(page_table_entry pte) {
    // TODO: student
    if((pte & 8) > 0)
        return 1; 
    return 0;
}

// -------------------  functions to print the state  --------------------------------------------- //

void print_page_table(int pid) 
{
    
    page_table_entry* page_table_start = NULL; // TODO student: start of page table of process pid
    int num_page_table_entries = -1;           // TODO student: num of page table entries

    // Do not change anything below
    puts("------ Printing page table-------");
    for (int i = 0; i < num_page_table_entries; i++) 
    {
        page_table_entry pte = page_table_start[i];
        printf("Page num: %d, frame num: %d, R:%d, W:%d, X:%d, P%d\n", 
                i, 
                pte_to_frame_num(pte),
                is_readable(pte),
                is_writeable(pte),
                is_executable(pte),
                is_present(pte)
                );
    }

}



// -----------------------------------------------------------------------
// add this to the mmu.c file and run

#include <assert.h>

#define MB (1024 * 1024)

#define KB (1024)

// just a random array to be passed to ps_create
unsigned char code_ro_data[10 * MB];


int main() {

	os_init();
    
	code_ro_data[10 * PAGE_SIZE] = 'c';   // write 'c' at first byte in ro_mem
	code_ro_data[10 * PAGE_SIZE + 1] = 'd'; // write 'd' at second byte in ro_mem

	int p1 = create_ps(10 * PAGE_SIZE, 1 * PAGE_SIZE, 2 * PAGE_SIZE, 1 * MB, code_ro_data);

	error_no = -1; // no error


    
	unsigned char c = read_mem(p1, 10 * PAGE_SIZE);

	assert(c == 'c');

	unsigned char d = read_mem(p1, 10 * PAGE_SIZE + 1);
	assert(d == 'd');

	assert(error_no == -1); // no error


	write_mem(p1, 10 * PAGE_SIZE, 'd');   // write at ro_data

	assert(error_no == ERR_SEG_FAULT);  


	int p2 = create_ps(1 * MB, 0, 0, 1 * MB, code_ro_data);	// no ro_data, no rw_data

	error_no = -1; // no error


	int HEAP_BEGIN = 1 * MB;  // beginning of heap

	// allocate 250 pages
	allocate_pages(p2, HEAP_BEGIN, 250, O_READ | O_WRITE);

	write_mem(p2, HEAP_BEGIN + 1, 'c');

	write_mem(p2, HEAP_BEGIN + 2, 'd');

	assert(read_mem(p2, HEAP_BEGIN + 1) == 'c');

	assert(read_mem(p2, HEAP_BEGIN + 2) == 'd');

	deallocate_pages(p2, HEAP_BEGIN, 10);

	print_page_table(p2); // output should atleast indicate correct protection bits for the vmem of p2.

	write_mem(p2, HEAP_BEGIN + 1, 'd'); // we deallocated first 10 pages after heap_begin

	assert(error_no == ERR_SEG_FAULT);


	int ps_pids[100];

	// requesting 2 MB memory for 64 processes, should fill the complete 128 MB without complaining.   
	for (int i = 0; i < 64; i++) {
    	ps_pids[i] = create_ps(1 * MB, 0, 0, 1 * MB, code_ro_data);
    	print_page_table(ps_pids[i]);	// should print non overlapping mappings.  
	}


	exit_ps(ps_pids[0]);
    

	ps_pids[0] = create_ps(1 * MB, 0, 0, 500 * KB, code_ro_data);

	print_page_table(ps_pids[0]);   

	// allocate 500 KB more
	allocate_pages(ps_pids[0], 1 * MB, 125, O_READ | O_READ | O_EX);

	for (int i = 0; i < 64; i++) {
    	print_page_table(ps_pids[i]);	// should print non overlapping mappings.  
	}
}






