#include "types.h"
#include "x86.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"
#include "sysfunc.h"
#include "pstat.h"
#include "ptable.h"
//struct pstat;
//struct pstat globalpstat; //gw: need to re-declare here. necessary
//or shall I decleare it in proc.c instead of sysproc.c? since scheduler is in proc.c where writing of globalpstat actually happen?
//extern struct ptable ptable; //assume somewhere this ptable is defined although I dont know 

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return proc->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = proc->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;
  
  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(proc->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since boot.
int
sys_uptime(void)
{
  uint xticks;
  
  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

//gw:
// sets the number of tickets of the calling process.
//By default, each process should get one ticket;
//calling this routine makes it such that a process can raise the number of tickets it receives, and thus receive a higher proportion of CPU cycles.
//This routine should return 0 if successful, and -1 otherwise (if, for example, the user passes in a number less than one).
//usage (guessed):
//question: how does one process call some system call? fork()?
// where is the ticket information stored? how to change them?

int
sys_settickets(void) {
  int ntickets;
  if(argint(0,&ntickets)<0)
    return -1;

  if(ntickets<=0)
    return -1;

  
  
  proc->tickets=ntickets;
  
  return 0;
  
  /* int num; */
  /* fetchint(p, */

  /* return *pid; */
}




//gw:
//returns some basic information about each running process, including how many times it has been chosen to run and its process ID, and which queue it is on (high or low).
//You can use this system call to build a variant of the command line program ps , which can then be called to see what is going on.
int
sys_getpinfo(void) {
  struct pstat* pst; //input. user will create a struct pstat variable and pass in its pointer. This pointer will also be used to output data.
  //below code act as allocating memeory for pst;

  if(  argptr(0,(char**)&pst,sizeof(struct pstat))<0)//(char **) or (void *) works for the mid
    return -1;
  
  
  //now we have the pointer. How do we fill information to it?
  int i=0;

  struct proc* p;// why no need malloc?
  acquire(&ptable.lock);
  for(i=0;i<NPROC;++i)
    {
      p=&ptable.proc[i];

      pst->pid[i]=p->pid;
      pst->inuse[i]=((p->state!=UNUSED)?1:0);
      pst->hticks[i]=p->hticks;
      pst->lticks[i]=p->lticks;  
    }

  release(&ptable.lock);

  return 0; //for now assume always success
  //usage of argpstat:
  //  argpstat(n for the nth argument passed to sys_getpinfo,
  //           &pst pass in the address of pointer pst)

  //declaration of argpstat:
  // int argpstat(int n, struct pstat** pst) //pstat** means pointer to pointer to pstat
  // {
  //           for(i=0;i<64;i++)
  //           {
  //                fetchint(ptable->proc[i], ptable.proc->tf->..., pst->pid[i];
  //           }

  

}



int
sys_halt(void) {
  char* p="Shutdown";
  for(;*p;p++) {
    outb(0x8900, *p);
  }
  return 0;
    
}


//initilize the struct pstat variable

//  for(i=0;i<NPROC;++i) {
    



   /* pst->inuse[i]=globalpstat.inuse[i]; */
   /* pst->pid[i]=globalpstat.pid[i]; */
   /* pst->hticks[i]=globalpstat.hticks[i]; */
   /* pst->lticks[i]=globalpstat.lticks[i]; */
   /* pst->choosen[i]=globalpstat.choosen[i]; */
   /* pst->priority[i]=globalpstat.priority[i]; */
   /* pst->htallot[i]=globalpstat.htallot[i];  */
   /* pst->ltallot[i]=globalpstat.ltallot[i];  */
    //  }
