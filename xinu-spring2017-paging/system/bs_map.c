#include <xinu.h>

int bs_map_lookup(int pid, uint32 vpage, bsd_t * store, int * page_offset) {
	intmask mask;
	mask = disable();
	// LOG("bs_map_lookup! pid: %d, vpage: %d...\n", pid, vpage);
	if (isbadpid(pid) || (pid == NULLPROC)) {

		LOG(" Bad pid requested in bs_map_lookup: %d .", pid);
		restore(mask);
		return SYSERR;
	}
	if(store == NULL || page_offset == NULL)
	{
		LOG( " Invalid BS id / bs offset ");
		restore(mask);
		return SYSERR;
	}

	if(vpage < 4096)
	{
		LOG(" Bad virtual address in bs_map_lookup %d", vpage);
		restore(mask);
		return SYSERR;
	}

	int bs_id;
	for (bs_id = MIN_ID; bs_id <= MAX_ID; ++bs_id) {
	  if (bstab[bs_id].isallocated == TRUE) {
		  if(bstab[bs_id].pid == pid && vpage >= bstab[bs_id].vp  && vpage <= (bstab[bs_id].vp + bstab[bs_id].npages))
		  {
			  *store = bs_id;
			  //LOG(" vpage is %d ",vpage);
			  //LOG(" bstab vp_no %d", bstab[bs_id].vp);
	      	  *page_offset = vpage - bstab[bs_id].vp;
	      	  //LOG(" page offset in store %d ", *page_offset);
	      	  restore(mask);
	      	  return OK;
		  }
	    }
	}

	restore(mask);
	return SYSERR;

}

int add_bs_mapping(int pid, uint32 vpage, bsd_t bs_id, int npages) {
	intmask mask;
	mask = disable();
	if (isbadpid(pid) || (pid == NULLPROC)) {

		LOG(" Bad pid requested in add_bs_mapping: %d .", pid);
		restore(mask);
		return SYSERR;
	}
	if(bs_id < MIN_ID || bs_id > MAX_ID )
	{
		LOG( " Invalid BS id ");
		restore(mask);
		return SYSERR;
	}

	if(vpage < 4096)
	{
		LOG(" Bad virtual address in add_bs_mapping %d", vpage);
		restore(mask);
		return SYSERR;
	}

	if (npages < 0 || npages > MAX_PAGES_PER_BS) {
		LOG(" Wrong number of pages (%d) for bs mapping", npages);
		restore(mask);
		return SYSERR;
	}
	bstab[bs_id].isallocated = TRUE;
	bstab[bs_id].npages = npages;
	bstab[bs_id].pid = pid;
	bstab[bs_id].vp = vpage; 
	// LOG("Added bs-mapping! bs_id: %d, npages: %d, pid: %d, vpage:%d \n", bs_id, npages, pid, vpage);

	restore(mask);
	return OK;
}

int drop_bs_mapping_process(pid32 pid) {
	int bs_id;
	intmask mask = disable();
	if (isbadpid(pid) || (pid == NULLPROC)) {

		LOG(" Bad pid requested in drop_bs_mapping_process: %d .", pid);
		restore(mask);
		return SYSERR;
	}
	for (bs_id = MIN_ID; bs_id <= MAX_ID; ++bs_id) {
		if (bstab[bs_id].pid == pid) {
			deallocate_bs(bs_id);
			// LOG("dropped bs map for process %d, bs_id %d",pid, bs_id);
		}
	}
	restore(mask);
	return OK;
}


int frame_map_lookup(int pid, bsd_t store, int bs_offset, int * frame_id ){
	// search for a frame with certain pid, bs_id and bs_offset
	intmask mask;
	mask = disable();
	if (isbadpid(pid) || (pid == NULLPROC)) {

		LOG(" Bad pid requested in frame_map_lookup: %d .", pid);
		restore(mask);
		return SYSERR;
	}
	if(store < MIN_ID || store > MAX_ID)
	{
		LOG(" Bad store id in frame_map_lookup. Received : %d", store );
		restore(mask);
		return SYSERR;
	}
	if(frame_id == NULL)
	{
		LOG(" Bad pageframe_id. Not initialized. Must be set so it can be returned. ");
		restore(mask);
		return SYSERR;
	}
	if(bs_offset < 0 || bs_offset >= MAX_PAGES_PER_BS)
	{
		LOG(" Bad offset in bs store with id %d offset %d", store, bs_offset);
		restore(mask);
		return SYSERR;
	}
	int fr_id;
	for (fr_id = 0; fr_id < NFRAMES; ++fr_id) {
		if (frametab[fr_id].type == PAGE) {
			frame_t *frameptr = &frametab[fr_id];
			if(frameptr->pid == pid && frameptr->bs_id == store && frameptr->bs_offset == bs_offset) {
				//kprintf(" Was a  match %d, %d, %d, %d, %d, %d", frametab[fr_id].pid, currpid, frametab[fr_id].bs_id, store, frames[fr_id].backstore_offset, bs_offset;
				*frame_id = fr_id;
				restore(mask);
				return OK;
			}
		}
	}
	restore(mask);
	return NONE;
}






