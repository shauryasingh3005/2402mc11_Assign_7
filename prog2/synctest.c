
#include "kernel/types.h"
#include "user/user.h"

#define NTHREADS 3
#define ITERATIONS 100000
#define NITEMS 10
#define BUFSIZE 5

volatile int counter = 0;

int buffer[BUFSIZE];
int in = 0;
int out = 0;

void
counter_worker(void *arg)
{
  for (int i = 0; i < ITERATIONS; i++) {
    sem_wait(0);
    counter++;
    sem_signal(0);
  }
  exit(0);
}

void
producer(void *arg)
{
  for (int i = 1; i <= NITEMS; i++) {
    sem_wait(1);  // empty slots
    sem_wait(3);  // mutex

    buffer[in] = i;
    in = (in + 1) % BUFSIZE;
    printf("Produced: %d\n", i);

    sem_signal(3);
    sem_signal(2); // full slots
  }
  exit(0);
}

void
consumer(void *arg)
{
  for (int i = 0; i < NITEMS; i++) {
    sem_wait(2);  // full slots
    sem_wait(3);  // mutex

    int item = buffer[out];
    out = (out + 1) % BUFSIZE;
    printf("Consumed: %d\n", item);

    sem_signal(3);
    sem_signal(1); // empty slots
  }
  exit(0);
}

int
main(void)
{
    printf("synctest started\n");
  void *stacks[3];

  // Counter synchronization
  sem_init(0, 1);
  printf("initializing semaphore\n");
  for (int i = 0; i < NTHREADS; i++) {
    stacks[i] = malloc(4096);
    if (!stacks[i] ||
        clone(counter_worker, 0, (char *)stacks[i] + 4096) < 0) {
      printf("Counter thread creation failed\n");
      exit(1);
    }
  }

  for (int i = 0; i < NTHREADS; i++)
    join();

  printf("Synchronized counter: %d\n", counter);
  printf("Expected counter: 300000\n");

  for (int i = 0; i < NTHREADS; i++)
    free(stacks[i]);

  // Producer-Consumer synchronization
  sem_init(1, BUFSIZE); // empty
  sem_init(2, 0);       // full
  sem_init(3, 1);       // mutex

  void *ps = malloc(4096);
  void *cs = malloc(4096);

  if (!ps || !cs) {
    printf("Stack allocation failed\n");
    exit(1);
  }

  if (clone(producer, 0, (char *)ps + 4096) < 0 ||
      clone(consumer, 0, (char *)cs + 4096) < 0) {
    printf("Producer/consumer creation failed\n");
    exit(1);
  }

  join();
  join();

  free(ps);
  free(cs);

  printf("Producer-Consumer completed\n");
  exit(0);
}
