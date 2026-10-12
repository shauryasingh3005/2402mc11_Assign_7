#include "kernel/types.h"
#include "user/user.h"

#define NCHILD 4
#define WORK 200000000
#define READY_SEM 7
#define START_SEM 8

void
burn_cpu(int amount)
{
  volatile int x = 0;

  for (int i = 0; i < amount; i++)
    x += i;
}

int
main(void)
{
  int priorities[NCHILD] = {2, 5, 10, 20};
  int parent_pid = getpid();

  // Keep the parent at priority 1 while it prepares all children.
  set_priority(parent_pid, 1);

  sem_init(READY_SEM, 0);
  sem_init(START_SEM, 0);

  printf("Priority Scheduler + Aging Test\n");
  printf("Priority 1 = highest, 20 = lowest\n");

  for (int i = 0; i < NCHILD; i++) {
    int pid = fork();

    if (pid < 0) {
      printf("Fork failed\n");
      exit(1);
    }

    if (pid == 0) {
      int mypid = getpid();

      if (set_priority(mypid, priorities[i]) < 0) {
        printf("Priority setup failed for PID %d\n", mypid);
        exit(1);
      }

      printf("Child PID %d ready, initial priority %d\n",
             mypid, get_priority(mypid));

      // Tell parent that priority setup is complete.
      sem_signal(READY_SEM);

      // Wait until all children are ready.
      sem_wait(START_SEM);

      printf("Child PID %d starting CPU work\n", mypid);
      burn_cpu(WORK);

      printf("Child PID %d completed, final priority %d\n",
             mypid, get_priority(mypid));

      exit(0);
    }
  }

  // Wait until all four children are ready.
  for (int i = 0; i < NCHILD; i++)
    sem_wait(READY_SEM);

  printf("All children ready; releasing workload\n");

  // Release all children while the parent retains priority 1.
  for (int i = 0; i < NCHILD; i++)
    sem_signal(START_SEM);

  // Let the children compete according to their priorities.
  set_priority(parent_pid, 10);

  for (int i = 0; i < NCHILD; i++)
    wait(0);

  printf("All children completed successfully.\n");
  exit(0);
}