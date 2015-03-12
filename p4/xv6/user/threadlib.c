#include "types.h"
#include "stat.h"
#include "fcntl.h"
#include "user.h"
#include "x86.h"
#include "param.h"
//#include "mmu.h"


int thread_create(void (*start_routine)(void*), void *arg) {
  //  void* ptr=malloc(PGSIZE);
  void* ptr=malloc(4096);
  return clone(start_routine, arg, ptr);
}

int thread_join() {

  void** pp=malloc(sizeof(*pp));
  int pid=join(pp);

  free(*pp);
  free(pp);
  return pid;

}

void lock_acquire(lock_t* lk) {
  while(xchg(&lk->locked,1)==1)
    ;
}

void lock_release(lock_t* lk) {
  lk->locked=0;
}

void lock_init(lock_t* lk){
  lk->locked=0;
}


/* int thread_create(void (*start_routine)(void*), void *arg) { */
/*   //  void* ptr=malloc(PGSIZE); */
/*   void* ptr=malloc(4096); */
/*   return clone(start_routine, arg, ptr); */
/* } */

/* int thread_join(void) { */
/*   void** pp=malloc(sizeof(*pp)); */
/*   int pid=join(pp); */

/*   free(pp[0]); */
/*   free(pp);  */
/*   return pid; */

/* } */

/* void lock_acquire(lock_t* lk) { */
/*   while(xchg(&lk->locked,1)==1) */
/*     ; */
/* } */

/* void lock_release(lock_t* lk) { */
/*   lk->locked=0; */
/* } */

/* void lock_init(lock_t* lk){ */
/*   lk->locked=0; */
/* } */
