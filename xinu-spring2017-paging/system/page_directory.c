#include <xinu.h>

pd_t *get_directory(pid32 pid) {
	intmask mask;
	mask = disable();
	pd_t *new_dir = NULL;
	frame_t *new_frame = NULL;
	new_frame = get_frame(DIR, pid);
	if (new_frame == NULL) {
		LOG("cannot find a new frame for DIR!");
		restore(mask);
		return NULL;
	}

	new_dir = (pd_t *)FRAMEID_TO_VADDR(new_frame->id);
	pd_t *dir_copy = new_dir;
	// LOG("Frame id %d, directory address 0x%8x", new_frame->id, new_dir);

	// Initialize all entries (refer to page table) in this directory.
	unsigned int entry = 0;
	for (entry = 0; entry < NENTDIR; entry++) {
		bzero(new_dir, sizeof(new_dir));
		if (entry < NGLPT) {
			//initialize global tables
			new_dir->pd_pres = 1;
			new_dir->pd_write = 1;
			new_dir->pd_base = FRAME0 + 1 + entry;  //virtual page number.
		}
		else if (entry == DEVICE_LOC) {
            new_dir->pd_pres = 1;
            new_dir->pd_write = 1;
            new_dir->pd_base = FRAME0 + 1 + NGLPT;
        }
        new_dir++;
	}
	// print_frame(new_frame->id);
	// LOG("New page directory: Frame id %d, pid %d", new_frame->id, new_frame->pid);
	restore(mask);
	return dir_copy;
}

int free_directory(pid32 pid) {
	intmask mask;
	mask = disable();

	struct procent * prptr;
    prptr = &proctab[pid]; 
    pd_t *dir = prptr->prpagedir;
    pd_t *tmp_dir = dir;
    uint32 entry = 0;
  	for (entry = 0; entry < NFRAMES; entry++) {
  		frame_t *curr_frame = &frametab[entry];
  		if (curr_frame->pid == pid && curr_frame->type == PAGE) {
  			free_frame(curr_frame);
  			sleepms(10);
  		}
  	}
  	for (entry = 0; entry < NFRAMES; entry++) {
  		frame_t *curr_frame = &frametab[entry];
  		if (curr_frame->pid == pid && curr_frame->type == PAGETABLE) {
  			free_frame(curr_frame);
  			sleepms(10);
  		}
  	}
  	for (entry = 0; entry < NFRAMES; entry++) {
  		frame_t *curr_frame = &frametab[entry];
  		if (curr_frame->pid == pid && curr_frame->type == DIR) {
  			free_frame(curr_frame);
  			sleepms(10);
  		}
  	}

  	// LOG("free dir ends");

	restore(mask);
	return OK;
}






