#include <xinu.h>

pt_t* get_page_table(pid32 pid) {
	intmask mask;
	mask = disable();
	pt_t *new_pt = NULL;
	frame_t *new_frame = NULL;
	new_frame = get_frame(PAGETABLE, pid);
	uint32 pt_entry_iter = 0;
	hook_ptable_create(new_frame->id);
	if(new_frame == NULL)
    {
        LOG("Failed to get new pagetable: no free frames available!");
        restore(mask);
        return NULL; 
    }
    new_pt = (pt_t *)FRAMEID_TO_VADDR(new_frame->id);
    for (; pt_entry_iter < NENTPG; ++pt_entry_iter) {
    	 bzero((char *)new_pt, sizeof(pt_t));
    	 new_pt++;
    }
    LOG("New page table: Frame id %d address 0x%08x", new_frame->id, (pt_t *)FRAMEID_TO_VADDR(new_frame->id));

	restore(mask);
	return (pt_t *) FRAMEID_TO_VADDR(new_frame->id);;
}

int32 initialize_global_pt(void) {
	int index = 0, entry_iter = 0;
	pt_t *new_pagetable = NULL;
	intmask mask;
	mask = disable();
	for (; index < NGLPT; index++) {
		frame_t *new_frame = NULL;
        new_frame = get_frame(GLOBAL, 0);
        if(new_frame == NULL) {
            LOG("Error: Failed to create new global page tables no free frames");
            restore(mask);
            return SYSERR;
        }
        new_pagetable = (pt_t *) FRAMEID_TO_VADDR(new_frame->id);
    	LOG("Global pagetable %d: Frame id %d address 0x%08x", index, new_frame->id, new_pagetable);
    	// LOG("BASE TO WRITE: %d", index * 1024);

        //initialize all pagetable entries in global pt
        bzero((pt_t *)new_pagetable, sizeof(pt_t)*NENTPG);
        for (entry_iter = 0; entry_iter < NENTPG; ++entry_iter) {
            new_pagetable[entry_iter].pt_base = ((index * 1024) + entry_iter);
            new_pagetable[entry_iter].pt_pres = 1;
            new_pagetable[entry_iter].pt_write = 1;
        }
        // LOG("PT full after pt_pres/write =============: 0x%08x", *(new_pagetable));
        // kprintf("(kprintf)PT full after pt_pres/write =============: 0x%08x\n", *(new_pagetable));
    }
	for (index = 0; index < NGLPT; index++) {
        //print_frame(index + 1);
    }

    restore(mask);
    return OK;
}


int32 initialize_device_pt() {
	intmask mask;
	mask = disable();

	frame_t * new_frame = get_frame(GLOBAL, 0);
	pt_t * new_pagetable = (pt_t *) FRAMEID_TO_VADDR(new_frame->id);
	pt_t * pt_copy = new_pagetable;
    int entry_iter = 0;
    //initialize all pagetable entries in device pt.
	for (; entry_iter < NENTPG; ++entry_iter) {
            bzero((char *)new_pagetable, sizeof(pt_t));
            new_pagetable->pt_base = (((DEVICE_LOC) * NENTPG) + entry_iter);
            new_pagetable->pt_pres = 1;
		    new_pagetable->pt_write = 1;
            new_pagetable->pt_global = 1;
            new_pagetable++;
	}
	LOG("Device pagetable: Frame id %d address 0x%08x", new_frame->id, pt_copy);

	restore(mask);
	return OK;

}

