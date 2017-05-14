/* hooks.c - hook_ptable_create, hook_ptable_delete, hook_pfault, hook_pswap_out */

#include <xinu.h>

// #define HOOK_LOG_ON

/*
 * Note that this file will be replaced in grading. So you may change them to
 * output what you are interested, but do not let your implementation depend on
 * this file.
 */

/*---------------------------------------------------------------------------
 *  hook_ptable_create  -  Called when your implementation is creating a page
 *  table.
 *---------------------------------------------------------------------------
 */
void hook_ptable_create(uint32 pagenum) {
#ifdef HOOK_LOG_ON
	kprintf("\n=== Created page table %d ===\n", pagenum);
#endif
}

/*---------------------------------------------------------------------------
 *  hook_ptable_delete  -  Called when your implementation is deleting a page
 *  table.
 *---------------------------------------------------------------------------
 */
void hook_ptable_delete(uint32 pagenum) {
#ifdef HOOK_LOG_ON
	kprintf("\n=== Deleted page table %d ===\n", pagenum);
#endif
}

/*---------------------------------------------------------------------------
 *  hook_pfault  -  Called whenever the page fault handler is called.
 *---------------------------------------------------------------------------
 */
void hook_pfault(void *addr) {
#ifdef HOOK_LOG_ON
	// kprintf("\n=== Page fault for address 0x%0X ===\n", addr);
	kprintf("\n=== Page fault for address 0x%0X, pid %d ===\n", addr, getpid());
#endif
}

/*---------------------------------------------------------------------------
 *  hook_pswap_out  -  Called when your implementation is replacing (swapping
 *  out) a frame.
 *---------------------------------------------------------------------------
 */
void hook_pswap_out(uint32 pagenum, uint32 framenum) {
#ifdef HOOK_LOG_ON
	kprintf("\n=== Replacing frame number %d, virtual page %d ===\n", framenum, pagenum);
#endif
}
