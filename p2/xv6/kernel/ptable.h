#ifndef _PTABLE_H_
#define _PTABLE_H_

#include "param.h"
#include "proc.h"
#include "spinlock.h"
struct {
  struct spinlock lock;
  struct proc proc[NPROC];

} ptable;
#endif // _PTABLE_H_
