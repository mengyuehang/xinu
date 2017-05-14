#include <xinu.h>

void pagefault_handler(void) {
	intmask mask;
	mask = disable();

	uint32 fault_address = (unsigned long )read_cr2();
	hook_pfault((void *)fault_address);
	// LOG("handler with address 0x%08x pd 0x%08x", fault_address, read_cr3());
	// kprintf("Pagefault number: %d \n", pagefaults);
	virtual_addr *vir_add = (virtual_addr *)&fault_address;
	uint32 pd_diff = vir_add->pd_offset;
	uint32 pt_diff = vir_add->pt_offset;
	// uint32 pg_diff = vir_add->pg_offset;
	bsd_t bs_store;
	int bs_offset;
	if (bs_map_lookup(currpid, fault_address >>12, &bs_store, &bs_offset) == SYSERR) {
		kprintf("FATAL: Accessed an illegal memory address in process %d  0x%08x vp %d pd_off %d", currpid, fault_address, fault_address>>12, pd_diff);
		kill(currpid);
		restore(mask);
		return;
	}

	struct	procent	*prptr;
	prptr = &proctab[currpid];
	pd_t * pgdir = prptr->prpagedir;
	pt_t * pt = NULL;
	uint32 frame_id;
	frame_t *frameptr = NULL;
	if (pgdir[pd_diff].pd_pres) {
		frame_id = (pgdir[pd_diff].pd_base) - FRAME0;
		pt = FRAMEID_TO_VADDR(frame_id);
		frameptr = &frametab[frame_id];   // the pagetable which stores the page being fault.
		frameptr->ref_count++;
	}
	else {
		pt = get_page_table(currpid);  // or else, find a new pagetable to store this page
		frame_id = ((uint32)pt)/NBPG - FRAME0;
		frameptr = &frametab[frame_id];
		frameptr->vp = fault_address >>12;
	    frameptr->bs_id = bs_store;
	    frameptr->bs_offset = bs_offset;
	    frameptr->ref_count++;
		pgdir[pd_diff].pd_pres = 1;
		// pgdir[pd_diff].pd_write = 1;
		pgdir[pd_diff].pd_base = FRAME0 + frame_id;

	}
	int new_frame_id;
	int result = frame_map_lookup(currpid, bs_store, bs_offset, &new_frame_id);
	//try to find the frame which stores the page for pid's backing store page.
	// NONE: not exist
	frame_t *new_frame;
	if (result == SYSERR) {
		kprintf(" Error: Frame map check executed with fatal error. ");
		restore(mask);
		kill(currpid);
		return;
	}
	else if (result == OK) {
		frametab[new_frame_id].ref_count++;
		restore(mask);
		return;
	}
	else {
		new_frame = get_frame(PAGE, currpid);
		new_frame->bs_id = bs_store;
		new_frame->bs_offset = bs_offset;
		new_frame->vp = fault_address>>12;
		new_frame_id = new_frame->id;
		open_bs(bs_store);
		while (SYSERR == read_bs((char *)FRAMEID_TO_VADDR(new_frame_id), bs_store, bs_offset)) {
			LOG("Read backing store error!");
		}
		close_bs(bs_store);
		// LOG("get frame done in pagefault_handler, frame id %d", new_frame->id);

	}
	// print_frame(new_frame->id);
	pt[pt_diff].pt_pres = 1;
	pt[pt_diff].pt_write = 1;
	pt[pt_diff].pt_acc = 1;
	pt[pt_diff].pt_base = FRAME0 + new_frame_id;
	pagefaults++;
	restore(mask);
	return;
}




