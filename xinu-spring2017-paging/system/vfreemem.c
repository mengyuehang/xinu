/* vfreemem.c - vfreemem */

#include <xinu.h>

/*---------------------------------------------------------------------------
 *  vfreemem  -  Free a memory block (in virtual memory), returning the block
 *  to the free list.
 *---------------------------------------------------------------------------
 */
syscall	vfreemem(
	  char		*blkaddr,	/* Pointer to memory block	*/
	  uint32	nbytes		/* Size of block in bytes	*/
	)
{
	/* LAB4TODO */
	intmask mask;
	struct vmemblk *next, *prev, *block, *head;
	struct procent *prptr;
	uint32 top;
	mask = disable();

	prptr = &proctab[currpid];
	// LOG("Trying to freemem for pid %d, memory addr. 0x%08x, minheap addr. 0x%08x, maxheap addr. 0x%08x", currpid, blkaddr, prptr->vminaddr, prptr->vmaxaddr);

	if ((nbytes <= 0) || ((uint32) blkaddr < (uint32) prptr->vminaddr)
			  || ((uint32) blkaddr > (uint32) prptr->vmaxaddr)) {
		restore(mask);
		return SYSERR;
	}
	nbytes = (uint32) roundmb(nbytes);
	head = proctab[currpid].vlist;

	/* Walk along free list	*/
	for(prev = head, next = head->vmbnext; (next!= NULL) && (next->vbase < blkaddr); prev = next, next = next->vmbnext);
	
	if (prev == head) {		/* Compute top of previous blkaddr*/
		top = (uint32) NULL;
	} else {
		top = (uint32) (prev->vbase) + prev->vlength;
	}

	/* Ensure new blkaddr does not overlap previous or next blkaddrs	*/

	if (((prev != head) && (uint32) blkaddr < top)
			|| ((next != NULL) && (uint32)blkaddr + nbytes > (uint32)next->vbase)) {
		restore(mask);
		return SYSERR;
	}

	/* Update global variable to reflect that we shall certainly be freeing memory now */
	head->vlength += nbytes;

	/* Add to free list with appropriate coalescence */

	if (top == (uint32)blkaddr) { /* Coalesce on left */
		prev->vlength += nbytes;
		
		if(next != NULL && blkaddr + nbytes == next->vbase) {
			/* both left and right coalesce are happening */
			prev->vlength += next->vlength;
			prev->vmbnext = next->vmbnext; // Delete right block
			freemem((char*)next, sizeof(struct vmemblk));
		}
	} else if (next != NULL && blkaddr + nbytes == next->vbase) { /* coalesce on right */
		next->vbase = blkaddr;
		next->vlength += nbytes;
	} else {
		/* no coalescence */
		/* create a new node and have it point to blkadr */
		struct vmemblk* newblk = (struct vmemblk*)getmem(sizeof(struct vmemblk));
		newblk->vbase = blkaddr;
		newblk->vlength = nbytes;
		newblk->vmbnext = next;
		prev->vmbnext = newblk;
	}


	restore(mask);
	return OK;
}
