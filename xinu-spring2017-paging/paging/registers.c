#include <xinu.h>

unsigned long tmp;

/*-------------------------------------------------------------------------
 * read_cr0 - read CR0
 *-------------------------------------------------------------------------
 */
unsigned long read_cr0(void) {

  intmask ps;
  ps = disable();
  unsigned long local_tmp;

  asm("pushl %eax");    // push (store) value of eax
  asm("movl %cr0, %eax");   // move value in cr0 to register eax
  asm("movl %eax, tmp");  // move value in eax (cr0) to tmp.
  asm("popl %eax");  // restore eax value

  local_tmp = tmp;

  restore(ps);

  return local_tmp;
}

/*-------------------------------------------------------------------------
 * read_cr2 - read CR2
 *-------------------------------------------------------------------------
 */

unsigned long read_cr2(void) {

  intmask ps;
  ps = disable();

  unsigned long local_tmp;
  asm("pushl %eax");
  asm("movl %cr2, %eax");
  asm("movl %eax, tmp");
  asm("popl %eax");

  local_tmp = tmp;

  restore(ps);

  return local_tmp;
}


/*-------------------------------------------------------------------------
 * read_cr3 - read CR3
 *-------------------------------------------------------------------------
 */

unsigned long read_cr3(void) {

  intmask ps;
  ps = disable();

  unsigned long local_tmp;
  asm("pushl %eax");
  asm("movl %cr3, %eax");
  asm("movl %eax, tmp");
  asm("popl %eax");

  local_tmp = tmp;

  restore(ps);

  return local_tmp;
}


/*-------------------------------------------------------------------------
 * read_cr4 - read CR4
 *-------------------------------------------------------------------------
 */

unsigned long read_cr4(void) {

  intmask ps;

  ps = disable();
  unsigned long local_tmp;

  asm("pushl %eax");
  asm("movl %cr4, %eax");
  asm("movl %eax, tmp");
  asm("popl %eax");

  local_tmp = tmp;

  restore(ps);

  return local_tmp;
}


/*-------------------------------------------------------------------------
 * write_cr0 - write CR0
 *-------------------------------------------------------------------------
 */

void write_cr0(unsigned long n) {

  intmask ps;

  ps = disable();

  tmp = n;
  asm("pushl %eax");       
  asm("movl tmp, %eax");		// mov (move) value at tmp into %eax register. 
  asm("movl %eax, %cr0");
  asm("popl %eax");

  restore(ps);

}


/*-------------------------------------------------------------------------
 * write_cr3 - write CR3
 *-------------------------------------------------------------------------
 */

void write_cr3(unsigned long n) {


  intmask ps;

  ps = disable();

  tmp = n;
  asm("pushl %eax");
  asm("movl tmp, %eax");            
  asm("movl %eax, %cr3");
  asm("popl %eax");

  restore(ps);

}


/*-------------------------------------------------------------------------
 * write_cr4 - write CR4
 *-------------------------------------------------------------------------
 */

void write_cr4(unsigned long n) {


  intmask ps;

  ps = disable();

  tmp = n;
  asm("pushl %eax");
  asm("movl tmp, %eax");                /* mov (move) value at tmp into %eax register.
                                           "l" signifies long (see docs on gas assembler)       */
  asm("movl %eax, %cr4");
  asm("popl %eax");

  restore(ps);

}


/*-------------------------------------------------------------------------
 * enable_paging
 *-------------------------------------------------------------------------
 */
void enable_paging(){
  
  unsigned long temp =  read_cr0();
  temp = temp | 0x1 | ( 0x1 << 31 );    // set PE (0th bit) and PG (31th bit) 
  write_cr0(temp); 
}


void switch_page_directory(unsigned long dir) {
  write_cr3(dir);       //cr3 stores current page dirctory base address.
}

