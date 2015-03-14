#ifndef _PSTAT_H_
#define _PSTAT_H_

#include "param.h"

 struct pstat {
  int inuse[NPROC];//whether the slot of the process process table is in use (1 for RUNNABLE or 0 for other)
  int pid[NPROC];  //the PID of each process
  int hticks[NPROC];// number of ticks each process has accumulated at HIGH priority
  int lticks[NPROC];// number of ticks each process has accumulated at LOW priority


   int choosen[NPROC];// number how many times each process has been choosen to run.
  int priority[NPROC];//queue priority, 1 or 0
  int htallot[NPROC]; //remaining time allotment in HIGH priority
  int ltallot[NPROC]; //remaining time allotment in LOW priority
  //gw: use inuse and priority to get list of jobs in high/low priority queue for lottery
  
} ;


#endif // _PSTAT_H_
