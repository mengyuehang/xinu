/* vgetmem.c - vgetmem */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  vgetmem  -  Allocate heap storage from virtual memory, returning lowest
 *  word address
 *------------------------------------------------------------------------
 */
char  	*vgetmem(
	  uint32	nbytes		/* Size of memory requested	*/
	)
{
	/* LAB4TODO */
	intmask mask;
	struct vmemblk *curr, *prev, *leftover, *head;
	struct procent *prptr;
	mask = disable();
	prptr = &proctab[currpid];
	nbytes = (uint32)roundmb(nbytes);
	int npages = (nbytes + NBPG - 1) / NBPG;
	if(nbytes <= 0 || npages > prptr->hsize) {
		restore(mask);
		return (char*) SYSERR;
	}

	head = prptr->vlist;
	prev = head;
	curr = prev->vmbnext;
	while (curr != (struct vmemblk*) NULL) {

		if (curr->vlength == nbytes) {
			prev->vmbnext = curr->vmbnext;
			head->vlength -= nbytes;
			LOG("GOT memory addr. 0x%08x, size %d", curr->vbase, nbytes);
			restore(mask);
			return (char *)curr->vbase;
		}
		else if (curr->vlength > nbytes) {
			char *new = curr->vbase;
			curr->vbase = curr->vbase + nbytes; // Compute new begin address
			curr->vlength -= nbytes; // Update available free block length
			head->vlength -= nbytes; // Update global available length
			LOG("GOT memory addr. 0x%08x, size %d", new, nbytes);
			restore(mask);
			return new;
		}
		else {
			prev = curr;
			curr = curr->vmbnext;
		}
	}
	restore(mask);
	return (char *)SYSERR;
}
