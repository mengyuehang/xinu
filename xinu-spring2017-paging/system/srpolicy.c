/* srpolicy.c - srpolicy */

#include <xinu.h>
// extern uint32 policy;
/*------------------------------------------------------------------------
 *  srpolicy  -  Set the page replacement policy.
 *------------------------------------------------------------------------
 */
syscall srpolicy(int new_policy)
{
	// LOG("setting policy to %d", new_policy);
	switch (new_policy) {
	case FIFO:
		// LOG("set to FIFO");
		policy = FIFO;
		return OK;

	case GCA:
		/* LAB4TODO - Bonus Problem */
		// LOG("set to GCA");
		policy = GCA;
		return OK;

	default:
		return SYSERR;
	}
}
