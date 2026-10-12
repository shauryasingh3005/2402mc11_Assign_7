
#include "kernel/types.h"
#include "user/user.h"

#define NTHREADS 3
#define ITERATIONS 100000

volatile int counter = 0;


void
worker(void *arg)
{
  for (int i = 0; i < ITERATIONS; i++) {
    int temp = counter;

    for (volatile int j = 0; j < 5000; j++) {
    }

    counter = temp + 1;
  }

  exit(0);
}

int
main(void)
{
  void *stacks[NTHREADS];

  for (int i = 0; i < NTHREADS; i++) {
    stacks[i] = malloc(4096);
    if (stacks[i] == 0) {
      printf("malloc failed\n");
      exit(1);
    }

    int pid = clone(worker, 0, (char *)stacks[i] + 4096);
    if (pid < 0) {
      printf("clone failed\n");
      exit(1);
    }
  }

  for (int i = 0; i < NTHREADS; i++) {
    if (join() < 0) {
      printf("join failed\n");
      exit(1);
    }
  }

  printf("Final counter: %d\n", counter);
  printf("Expected counter: 300000\n");
  exit(0);
}
