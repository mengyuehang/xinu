/* paging.h */

#ifndef __PAGING_H_
#define __PAGING_H_


#define FRAMEID_TO_VP(frameid)		(FRAME0 + frameid)
#define FRAMEID_TO_VADDR(frameid)	((FRAME0 + frameid) * NBPG)
#define VP_TO_VADDR(vp)		(vp * NBPG)


#define NBPG		4096	/* number of bytes per page	*/
#define FRAME0		1024	/* zero-th frame		*/
#define NENTPG		1024	/* number of entries per page */
#define NGLPT		4		/* number of global pagetables	*/
#define NENTDIR		1024 	/* number of entries per directory */

#ifndef NFRAMES
#define NFRAMES		100	/* number of frames		*/
#endif

#define DEVICE_LOC 	576
#define NONE		-222


#define MAP_SHARED 1
#define MAP_PRIVATE 2

#define FIFO 3
#define GCA 4

#define MAX_ID		7		/* You get 8 mappings, 0 - 7 */
#define MIN_ID		0
#define IS_VALID_FRAMEID(frame_id)          ((frame_id >= 0) && (frame_id < NFRAMES))

#define INV_BS_ID 10
#define INV_BS_OFF 210


/* Structure for a page directory entry */

typedef struct {
	unsigned int pd_pres	: 1;		/* page table present?	(LSB)	*/
	unsigned int pd_write : 1;		/* page is writable?		*/
	unsigned int pd_user	: 1;		/* is use level protection?	*/
	unsigned int pd_pwt	: 1;		/* write through cachine for pt? */
	unsigned int pd_pcd	: 1;		/* cache disable for this pt?	*/
	unsigned int pd_acc	: 1;		/* page table was accessed?	*/
	unsigned int pd_mbz	: 1;		/* must be zero			*/
	unsigned int pd_fmb	: 1;		/* four MB pages?		*/
	unsigned int pd_global: 1;		/* global (ignored)		*/
	unsigned int pd_avail : 3;		/* for programmer's use		*/
	unsigned int pd_base	: 20;		/* location of page table?	(MSB) */
} pd_t;

/* Structure for a page table entry */

typedef struct {
	unsigned int pt_pres	: 1;		/* page is present?	 (LSB)	*/
	unsigned int pt_write : 1;		/* page is writable?		*/
	unsigned int pt_user	: 1;		/* is use level protection?	*/
	unsigned int pt_pwt	: 1;		/* write through for this page? */
	unsigned int pt_pcd	: 1;		/* cache disable for this page? */
	unsigned int pt_acc	: 1;		/* page was accessed?		*/
	unsigned int pt_dirty : 1;		/* page was written?		*/
	unsigned int pt_mbz	: 1;		/* must be zero			*/
	unsigned int pt_global: 1;		/* should be zero in 586	*/
	unsigned int pt_avail : 3;		/* for programmer's use		*/
	unsigned int pt_base	: 20;		/* location of page?  (MSB)	 */
} pt_t;

// #define LOG(STR, ...) do { kprintf("[ <DEBUG IN FUNCTION %s | PID %d> ]  : ", __func__,  currpid) ; kprintf(#STR, ##__VA_ARGS__) ; kprintf("\n"); } while (0)

#define LOG(STR, ...) ;

typedef enum {DIR, GLOBAL, PAGETABLE, PAGE, FREE} frame_type;

typedef struct _frame_t{
	int32 id;
	uint32 vp;   // virtual page number
	pid32 pid;		// process that owns this frame
	frame_type type;	// directory, global pagetable, virtual pagetable, page, free
	uint32 ref_count;
	bsd_t bs_id;	// backing store ID
	uint32 bs_offset;
	uint32	actual_dirty_bit;
	struct _frame_t *next;
} frame_t;

extern frame_t frametab[NFRAMES];
extern frame_t *fifo_head;		/* head of mapped frame list: in time order */
extern uint32	gca_head;

extern 	uint32	policy;
extern 	uint32 	pagefaults;
extern int freeframes;




// in pagefault_handler.c
typedef struct __virtu_addr{
  unsigned int pg_offset : 12;   // page offset (LSB : 12)
  unsigned int pt_offset : 10;   // page table offset (middle: 10)
  unsigned int pd_offset : 10;   // page directory offset (MSB : 10)
} virtual_addr;


// in paging/page_table.c
extern int32 initialize_global_pt(void);
extern int32 initialize_device_pt(void);
extern pt_t * get_page_table(pid32);

// in paging/page_frame.c
extern frame_t * get_frame(frame_type, pid32);
extern void initialize_frames(void);
extern frame_t * replace_fifo(void);
extern frame_t * replace_gca(void);
extern int free_frame(frame_t *);

extern pd_t *get_directory(pid32);
extern int free_directory(pid32);


extern int bs_map_lookup(int, uint32, int *, int * );
extern int add_bs_mapping(int, uint32, bsd_t, int);
extern int drop_bs_mapping_process(pid32);
extern int frame_map_lookup(int, bsd_t, int, int * );

// in paging/registers.c
extern unsigned long read_cr0(void);
extern unsigned long read_cr2(void);
extern unsigned long read_cr3(void);
extern unsigned long read_cr4(void);
extern void write_cr0(unsigned long);
extern void write_cr3(unsigned long);
extern void write_cr4(unsigned long);
extern void enable_paging(void);
extern void switch_page_directory(unsigned long);

extern void pagefault_handler(void);

extern void print_frame(uint32);
extern void print_all_frames();

#endif // __PAGING_H_
