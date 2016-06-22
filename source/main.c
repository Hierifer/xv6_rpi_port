/*****************************************************************
*       main.c
*       by Zhiyi Huang, hzy@cs.otago.ac.nz
*       University of Otago
*
********************************************************************/


#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "arm.h"
#include "mailbox.h"

extern char end[]; // first address after kernel loaded from ELF file
extern pde_t *kpgdir;
extern FBI fbinfo;
extern volatile uint *mailbuffer;

void OkLoop()
{
   setgpiofunc(47, 1); // gpio 16, set as an output
   while(1){
        setgpioval(47, 0);
        delay(1000000);
        setgpioval(47, 1);
        delay(1000000);
   }
}

void NotOkLoop()
{
   setgpiofunc(47, 1); // gpio 16, set as an output
   while(1){
        setgpioval(47, 0);
        delay(100000);
        setgpioval(47, 1);
        delay(100000);
   }
}

void machinit(void)
{
    memset(cpus, 0, sizeof(struct cpu)*NCPU);
}


void enableirqminiuart(void);

int cmain()
{

  mmuinit1();
  machinit();
  uartinit();
  dsb_barrier();
  consoleinit(); 
  cprintf("\nHello World from xv6\n");
  kinit1(end, P2V(8*1024*1024));  // reserve 8 pages for PGDIR
  cprintf("\nkinit1\n");
  kpgdir=p2v(K_PDX_BASE);
  cprintf("\nkpgdir\n");
  mailboxinit();
  cprintf("\nmailboxinit()\n");
  create_request(mailbuffer, MPI_TAG_GET_ARM_MEMORY, 8, 0, 0);
  cprintf("\ncreate_request\n");
  writemailbox((uint *)mailbuffer, 8);
  cprintf("\nwritemailbox\n");
  readmailbox(8);
  cprintf("\nreadmailbox\n");
  if(mailbuffer[1] != 0x80000000) cprintf("new error readmailbox\n");
  else 
cprintf("ARM memory is %x %x\n", mailbuffer[MB_HEADER_LENGTH + TAG_HEADER_LENGTH], mailbuffer[MB_HEADER_LENGTH + TAG_HEADER_LENGTH+1]);
  
  pinit();
  cprintf("\npinit()\n");
  tvinit();
  cprintf("it is ok after tvinit\n");
  binit();
cprintf("it is ok after binit\n");
  fileinit();
cprintf("it is ok after fileinit\n");
  iinit();
cprintf("it is ok after iinit\n");
  ideinit();
cprintf("it is ok after ideinit\n");
  timer3init();
  kinit2(P2V(8*1024*1024), P2V(PHYSTOP));
cprintf("it is ok after kinit2\n");
  userinit();
cprintf("it is ok after userinit\n");
  scheduler();

  NotOkLoop();

  return 0;
}
