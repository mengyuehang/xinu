#include <xinu.h>

extern frame_t frametab[NFRAMES];
extern frame_t *fifo_head;		/* head of mapped frame list: in time order */

extern 	uint32	policy;
extern 	uint32 	pagefaults;
local void remove_from_fifo_list(frame_t * frameptr);
local void print_fifo_list();

void initialize_frame(frame_t *frameptr) {
	intmask mask;
	mask = disable();
	frameptr->pid = -1;
	frameptr->type = FREE;
	frameptr->next = NULL;
	frameptr->vp = -1;
	frameptr->bs_id = -1;
	frameptr->bs_offset = -1;
	frameptr->actual_dirty_bit = 0;
	restore(mask);
	return;
}

void initialize_frames() {
	intmask mask;
	mask = disable();

	int iter = 0;
	for (; iter < NFRAMES; iter ++) {
		frame_t *frameptr = &frametab[iter];
		frameptr->id = iter;
		initialize_frame(frameptr);
	}
	fifo_head = NULL;

	restore(mask);
	return;
}


frame_t* get_frame(frame_type type, pid32 pid) {
	intmask mask;
	mask = disable();
	frame_t *frameptr = NULL;
	frame_t *result = NULL;

	// Search the inverted page table for an empty frame.
	int start = 0, end = NFRAMES;
	if(type == GLOBAL) {
		start = 1;
		end = 6;
	}

	int iter = start;
	for (; iter < end; iter++) {
		frameptr = &frametab[iter];
		//  If one exists, stop.
		if (frameptr->type == FREE) {
			result = frameptr;
			break;
		}
	}

	// Else, pick a page to replace (using the current replacement policy)
	if (result == NULL) {
		// LOG("No free frames in memory, evict one using %d!", policy);
		if (policy == FIFO) {
			result = replace_fifo();
		}
		if (policy == GCA)
			result = replace_gca();
		// LOG("Evict Frame %d", result->id);
	}

	if (result != NULL) {
		initialize_frame(result);
		result->type = type;
		result->pid = pid;
		if (policy == FIFO && type == PAGE) {
			frame_t *current = fifo_head;
			frame_t *prev = NULL;
			while (current) {
				prev = current;
				current = current->next;
			}
			if (prev == NULL) {
				fifo_head = result;
			}
			else {
				prev->next = result;
			}
		}
	}
	freeframes--;
	// LOG("get_frame done, frame id %d!", result->id);

	restore(mask);
	return result;

}

int free_frame(frame_t *frameptr) {
	intmask mask;
	mask = disable();
	if (frameptr== NULL || !IS_VALID_FRAMEID(frameptr->id) || frameptr->type == FREE) {
		restore(mask);
		return OK;
	}

	if (frameptr->id < 5) {
		LOG("Trying to free global table!!!");
		restore(mask);
		return OK;
	}

	if (frameptr->type == PAGE) {
		//3. Using the inverted page table, get vp, the virtual page number of the page to be replaced.
		uint32 vp = frameptr->vp;
		// LOG("freeing a PAGE, frame id: %d, vpage number: %d", frameptr->id, frameptr->vp);
		
		//4. Let a be vp*4096 (the first virtual address on page vp).
		uint32 a = vp * NBPG;
		virtual_addr * virtual = (virtual_addr *) &a;

		//5. Let p be the high 10 bits of a. Let q be bits [21:12] of a.
		uint32 p = virtual->pd_offset;
		uint32 q = virtual->pt_offset;

		//6. Let pid be the process id of the process owning vp.
		pid32 pid = frameptr->pid;

		//7. Let pd point to the page directory of process pid.
		struct	procent	*prptr;	
		prptr = &proctab[pid];
		pd_t *pd = prptr->prpagedir;

		if (pid == currpid) {
			// LOG("pid = currpid");
			asm volatile("invlpg (%0)" ::"r" (vp) : "memory");
		}

		remove_from_fifo_list(frameptr);

		if (pd == NULL) {
			LOG("pd doesn't exist!");
			restore(mask);
			return SYSERR;
		}
		if (pd[p].pd_pres) {
			//8. Let pt point to the pid's p_th page table.
			uint32 dirty = 0;
			pt_t *pt = (pt_t * ) ((pd[p].pd_base) * NBPG);
			if (pt[q].pt_pres) {
				//9. Mark the appropriate entry of pt as not present.
				pt[q].pt_pres = 0;
				dirty = pt[q].pt_dirty || frameptr->actual_dirty_bit;
				// LOG("frame %d is dirty? dirty: %d, dirty bit: %d, actual dirty bit: %d", frameptr->id, dirty, pt[q].pt_dirty, frameptr->actual_dirty_bit);
			}
			//11. decrement the reference count of the frame occupied by pt. If the reference count
			// has reached zero, mark the appropriate entry in pd as "not present".
			frame_t * pt_frame = &frametab[(pd[p].pd_base) - FRAME0];

			if (--pt_frame->ref_count <= 0) {
				pd[p].pd_pres = 0;
				LOG("page table containing this page is empty, free it!");
				free_frame(pt_frame);
			}
			if (dirty) {
				bsd_t bs_id;
				int bs_page_offset = 0;
				if (bs_map_lookup(pid, vp, &bs_id, &bs_page_offset) == SYSERR) {
					kill(pid);
					restore(mask);
					return SYSERR;
				}

				if (frameptr->bs_id == bs_id && frameptr->bs_offset == bs_page_offset) {
					open_bs(bs_id);
					while (SYSERR == write_bs( (char *)FRAMEID_TO_VADDR(frameptr->id), frameptr->bs_id, frameptr->bs_offset) ) {
						LOG('Error in write backing store');
					}					
					close_bs(bs_id);
				}
			}
		}
		initialize_frame(frameptr);
	}
	else if (frameptr->type == DIR) {
		// LOG("freeing a DIR. frame id: %d", frameptr->id);
		initialize_frame(frameptr);
	}
	else if (frameptr->type == PAGETABLE) {
		// LOG("freeing a PT. frame id: %d", frameptr->id);
		hook_ptable_delete(frameptr->id);
		initialize_frame(frameptr);
	}

	// LOG('free-frame ends');
	// print_all_frames();
	freeframes++;
	restore(mask);
	return OK;

}


frame_t* replace_fifo(){
	intmask mask;
	mask = disable();

	frame_t *frameptr = fifo_head;
	frame_t *prev = NULL;
	while (frameptr) {
		if (frameptr->type == PAGE) {
			if (prev == NULL) {
				fifo_head = frameptr->next;
				// frameptr = fifo_head;
			}
			else {
				prev->next = frameptr->next;
				// frameptr = prev->next;
			}
			break;
		}
		else {
			prev = frameptr;
			frameptr = frameptr->next;
		}
	}
	hook_pswap_out(frameptr->vp, frameptr->id);
	if (frameptr) {
		free_frame(frameptr);
	}
	restore(mask);
	return frameptr;
}


frame_t* replace_gca(){
	intmask mask;
	mask = disable();
	// LOG("evict using gca; head: %d", gca_head);
	uint32 curr = gca_head;
	frame_t *res = NULL;
	while (res == NULL) {
		frame_t *frameptr = &frametab[curr];
		if (frameptr->type == PAGE) {
			uint32 vp = frameptr->vp;
			uint32 a = vp * NBPG;
			virtual_addr * virtual = (virtual_addr *) &a;
			uint32 p = virtual->pd_offset;
			uint32 q = virtual->pt_offset;
			pid32 pid = frameptr->pid;
			struct	procent	*prptr;	
			prptr = &proctab[pid];
			pd_t *pd = prptr->prpagedir;
			uint32 dirty = 0;
			uint32	access = 0;
			if (pd == NULL) {
				LOG("pd doesn't exist in replacing_gca!");
				restore(mask);
				return (frame_t*)SYSERR;
			}
			if (!pd[p].pd_pres) {
				LOG("pt doesn't exist in replacing_gca!");
				restore(mask);
				return (frame_t*)SYSERR;
			}
			pt_t *pt = (pt_t *) ((pd[p].pd_base) * NBPG);
			if (pt[q].pt_pres) {
				access = pt[q].pt_acc;
				dirty = pt[q].pt_dirty;
			}
			// LOG("before: frame %d, access %d, dirty %d", curr, pt[q].pt_acc, pt[q].pt_dirty);
			if (access == 0 && dirty == 0) {
				gca_head = frameptr->id +1;
				hook_pswap_out(frameptr->vp, frameptr->id);
				free_frame(frameptr);
				return frameptr;
			}
			else if (access == 1 && dirty == 0) {
				pt[q].pt_acc = 0;
			}
			else if (access == 1 && dirty == 1) {
				pt[q].pt_dirty = 0;
				frameptr->actual_dirty_bit = 1;
			}
			// LOG("after: frame %d, access %d, dirty %d", curr, pt[q].pt_acc, pt[q].pt_dirty);
		}
		curr = (curr + 1) % NFRAMES;
	}
	restore(mask);
	return res;
}

void remove_from_fifo_list(frame_t * frameptr)
{
	intmask mask = disable();
	frame_t * previous;
	frame_t * current;
	frame_t * temp;
	previous = NULL;
	current = fifo_head;

	while (current != NULL)
	{
		if (current == frameptr) {
			temp = current;
			if (previous != NULL) {
				previous->next = current->next;
				// current = previous->next;
			} else {
				fifo_head = current->next;
				// current = fifo_head;
			}
			break;
		}
		else{
			previous = current;
			current = current->next;
		}
	}
	// LOG('removed frame %d from fifo list', frameptr->id);
	restore(mask);
	return;
}

void print_fifo_list() {
	frame_t *curr = fifo_head;
	while (curr != NULL) {
		LOG("fifo list --- frame id %d", curr->id);
		curr = curr->next;
	}
	return;
}


void remove_pagetable_for_pid(pid32 pid)
{
	intmask mask = disable();
	int i;
	frame_t * frame = NULL;
	for(i = 0; i < NFRAMES; i++)
	{
		frame = &frametab[i];
		if(frame->type == FREE)
			continue;
		else if(frame->pid == pid && frame->type == PAGETABLE)
			free_frame(frame);
	}
	restore(mask);
	return;

}

void remove_page_directory_for_pid(pid32 pid)
{
	intmask mask = disable();
	struct	procent	*prptr;		/* Ptr to process table entry	*/
	struct procent *prptrNull;
	prptr = &proctab[pid];
	prptrNull = &proctab[0];
	if (prptr->prpagedir != prptrNull->prpagedir) {
		pd_t * pd = prptr->prpagedir;
		int pdframe = ((uint32)pd)/NBPG - FRAME0;
		free_frame(&frametab[pdframe]);
		// kprintf(" REM MAPPING dir from %d: 0x%08x to %d: 0x%08x",pid, prptr->prpagedir, 0, prptrNull->prpagedir);
	}
	restore(mask);
	return;
}

void print_all_frames() {
	uint32 frame_id = 0;
	frame_t *frameptr;
	for (; frame_id < NFRAMES; frame_id++) {
		print_frame(frame_id);
	}
}

void print_frame(uint32 frame_id) {
		frame_t *frameptr = &frametab[frame_id];
		kprintf("frame id: %d", frameptr->id);
		if (frameptr->type == FREE) {
			LOG("frame type: FREE");
		}
		else if (frameptr->type == PAGETABLE) {
			LOG("frame type: VIRTUAL PAGETABLE; pid: %d", frameptr->pid);
			pt_t *ptptr = (pt_t *)(FRAMEID_TO_VADDR(frame_id));
			// uint32 entry = 0;
			// for (; entry < NENTPG; entry++) {
			// 	if (ptptr->pt_pres) {
			// 		LOG("entry: %d, page: %d, write? %d", entry, ptptr->pt_base, ptptr->pt_write);
			// 	}
			// 	ptptr++;
			// }
		}
		else if (frameptr->type == DIR) {
			LOG("frame type: DIR; pid: %d", frameptr->pid);
			// pd_t *ptptr = (pd_t *)(FRAMEID_TO_VADDR(frame_id));
			// uint32 entry = 0;
			// for (; entry < 50; entry++) {
			// 	if (ptptr->pd_pres) {
			// 		LOG("entry: %d, page: %d, write? %d", entry, ptptr->pd_base, ptptr->pd_write);
			// 	}
			// 	ptptr++;
			// }
		}
		else if (frameptr->type == PAGE) {
			LOG("frame %d, type: PAGE; pid: %d", frameptr->id, frameptr->pid);

		}
		else if (frameptr->type == GLOBAL){
			LOG("frame type: GLOBAL PAGETABLE; pid: %d", frameptr->pid);
			// pt_t *ptptr = (pt_t *)(FRAMEID_TO_VADDR(frame_id));
			pt_t *new_pagetable = (pt_t *) FRAMEID_TO_VADDR(frame_id);
			// uint32 entry = 0;
			// for (; entry < 50; entry++) {
			// 	if (ptptr->pt_pres) {
			// 		LOG("entry: %d, vpage: %d, write? %d", entry, ptptr->pt_base, ptptr->pt_write);
			// 	}
			// 	ptptr++;
			// }
			LOG("PT full in print_frame: 0x%08x", *new_pagetable);
		}
}







