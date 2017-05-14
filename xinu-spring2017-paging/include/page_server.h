/* page_server.h */

#ifndef __PAGE_SERVER_H_
#define __PAGE_SERVER_H_

/* Definitions and data structures for backing store */

#ifndef MAX_BS_ENTRIES
#define MAX_BS_ENTRIES MAX_ID - MIN_ID + 1
#endif

#ifndef RD_PAGES_PER_BS
#define RD_PAGES_PER_BS 205
#endif

#ifndef MAX_PAGES_PER_BS
#define MAX_PAGES_PER_BS 200
#endif

//backing store entry
struct bs_entry{
	bsd_t   bs_id;
	byte    isopen;
	byte	isallocated;
	int32	usecount;
	unsigned int npages;
	pid32 	pid;
	int 	vp;   // virtual page number (first page in this bs)
};

#define PAGE_SERVER_ACTIVE      1
#define PAGE_SERVER_INACTIVE    2

// bstab: used for lookup
extern struct bs_entry bstab[MAX_BS_ENTRIES];
extern sid32  bs_sem;
extern bool8  PAGE_SERVER_STATUS;
extern sid32  bs_init_sem;

#endif // __PAGE_SERVER_H_
