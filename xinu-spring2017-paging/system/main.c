/*  main.c  - main */

#include <xinu.h>
#include <stdio.h>
#include <stdlib.h> 

#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define RESET   "\033[0m"



void print_stats(){
 kprintf("------------------------\n");
 //kprintf("Replacement policy: %d\n", curr_policy);
 //kprintf("Available physical frames: %d\n", free_frcount);
 kprintf("Page fault count: %d\n", get_faults());
 //kprintf("Page replacement count: %d\n", replace_count);
 kprintf("------------------------\n");
}



process test_bonus(int heap_size, int iterations, char c) {
 int i;
 char *buf = (char *)vgetmem(heap_size * NBPG);
 if (buf == (char *)SYSERR) {
  kprintf(RED "Test Failed (1)\n" RESET);
  return SYSERR;
 }
 int size = heap_size * NBPG;
 /*for(i = 0; i < iterations; i++) {
  int index = rand() % size;
  if (index < 0 || index >= size) {
   kprintf(RED "Test Failed (invalid index)\n" RESET);
   return SYSERR;
  }
  buf[index * NBPG] = c;
 }*/
 for(i = 0; i < size; i++) {
  buf[i] = c;
 }
 for(i = 0; i < size; i++) {
  if (buf[i] != c) {
   kprintf(RED "Test Failed (check)\n" RESET);
   return SYSERR;
  }
 }
 kprintf("Done: Process %d\n", currpid);
 return OK;
}

process main(void)
{
 srpolicy(GCA);

 /* Start the network */
 /* DO NOT REMOVE OR COMMENT THIS CALL */
 netstart();

 /* Initialize the page server */
 /* DO NOT REMOVE OR COMMENT THIS CALL */
 psinit();

 print_stats();

 
 kprintf("------Test Bonus----------\n");
 int orig_pr = prcount;
 pid32 p1 = vcreate(test_bonus, INITSTK, 200, 50, "p1", 2, 200, 500, 'A');
 pid32 p2 = vcreate(test_bonus, INITSTK, 200, 50, "p2", 2, 200, 500, 'B');
 pid32 p3 = vcreate(test_bonus, INITSTK, 200, 50, "p3", 2, 200, 500, 'C');
 resched_cntl(DEFER_START);
 resume(p1);
 resume(p2);
 resume(p3);
 resched_cntl(DEFER_STOP);
 while(prcount > orig_pr) sleep(5);
 
 print_stats();
 kprintf("------Test Bonus end----------\n\n\n");

 return OK;
}