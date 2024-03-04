#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  backtrace();
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

/*
* @brief: initializes the alarm handler and the number of ticks
*
* @param: ticks - the number of ticks to wait before calling the handler
* @param: handler - the address of the handler
*
* @return: 0
*
* @note: if ticks is 0, the alarm is disabled
*/
uint64
sys_sigalarm(void)
{
  struct proc *p = myproc();
  int ticks;
  uint64 handler;
  argint(0, &ticks);
  argaddr(1, &handler);
  p->alarmhandler = handler;
  p->alarmticks = ticks;
  return 0;
}


/*
* @brief: returns to the handler after the alarm has been called, 
*         resetting the alarm and the trapframe
*
* @return: 0
*/
uint64
sys_sigreturn(void)
{
  struct proc *p = myproc();
  p->passedticks = 0;
  for (uint64 i = 0; i < sizeof(struct trapframe); i+=8)
  {
    p->trapframe[i] = p->prevtrapframe[i];
  }
  usertrapret();
  return 0;
}
