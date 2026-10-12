#include "kernel/types.h"
#include "user/user.h"

#define N 5
#define M 3

// ---------- Deadlock Detection ----------
void detect_deadlock(int n, int m,
                     int allocation[5][3],
                     int request[5][3],
                     int available[3])
{
  int work[3], finish[5], found, deadlock = 0;

  for (int j = 0; j < m; j++)
    work[j] = available[j];

  for (int i = 0; i < n; i++)
    finish[i] = 0;

  printf("\nDeadlock detection started\n");

  do {
    found = 0;

    for (int i = 0; i < n; i++) {
      if (finish[i])
        continue;

      int possible = 1;
      for (int j = 0; j < m; j++) {
        if (request[i][j] > work[j]) {
          possible = 0;
          break;
        }
      }

      if (possible) {
        printf("P%d can finish; releases its resources\n", i);

        for (int j = 0; j < m; j++)
          work[j] += allocation[i][j];

        finish[i] = 1;
        found = 1;
      }
    }
  } while (found);

  for (int i = 0; i < n; i++) {
    if (!finish[i]) {
      printf("Deadlocked process: P%d\n", i);
      deadlock = 1;
    }
  }

  if (!deadlock)
    printf("No deadlock detected\n");
}

// ---------- Banker Safety Algorithm ----------
int safety(int n, int m,
           int allocation[5][3],
           int need[5][3],
           int available[3],
           int print_steps)
{
  int work[3], finish[5], sequence[5];
  int count = 0, found;

  for (int j = 0; j < m; j++)
    work[j] = available[j];

  for (int i = 0; i < n; i++)
    finish[i] = 0;

  if (print_steps)
    printf("\nChecking system safety...\n");

  do {
    found = 0;

    for (int i = 0; i < n; i++) {
      if (finish[i])
        continue;

      int possible = 1;

      for (int j = 0; j < m; j++) {
        if (need[i][j] > work[j]) {
          possible = 0;
          break;
        }
      }

      if (possible) {
        if (print_steps)
          printf("P%d finishes; Work = ", i);

        for (int j = 0; j < m; j++)
          work[j] += allocation[i][j];

        if (print_steps)
          for (int j = 0; j < m; j++)
            printf("%d ", work[j]);

        if (print_steps)
          printf("\n");

        sequence[count++] = i;
        finish[i] = 1;
        found = 1;
      }
    }
  } while (found);

  if (count == n) {
    printf("Safe sequence: ");

    for (int i = 0; i < n; i++) {
      printf("P%d", sequence[i]);
      if (i < n - 1)
        printf(" -> ");
    }

    printf("\nSystem is SAFE\n");
    return 1;
  }

  printf("System is UNSAFE\n");
  return 0;
}

// ---------- Banker Resource Request ----------
void request_resources(int pid, int request[3],
                       int allocation[5][3],
                       int need[5][3],
                       int available[3])
{
  for (int j = 0; j < M; j++) {
    if (request[j] > need[pid][j]) {
      printf("Request denied: exceeds remaining Need\n");
      return;
    }

    if (request[j] > available[j]) {
      printf("Request denied: insufficient Available resources\n");
      return;
    }
  }

  // Temporarily grant the request.
  for (int j = 0; j < M; j++) {
    available[j] -= request[j];
    allocation[pid][j] += request[j];
    need[pid][j] -= request[j];
  }

  printf("Checking safety after granting request...\n");

  if (safety(N, M, allocation, need, available, 0)) {
    printf("Request GRANTED: resulting state is safe\n");
  } else {
    // Roll back an unsafe request.
    for (int j = 0; j < M; j++) {
      available[j] += request[j];
      allocation[pid][j] -= request[j];
      need[pid][j] += request[j];
    }

    printf("Request DENIED: would cause an unsafe state\n");
  }
}

void print_matrix(char *name, int matrix[5][3], int n)
{
  printf("%s:\n", name);

  for (int i = 0; i < n; i++)
    printf("P%d: %d %d %d\n",
           i, matrix[i][0], matrix[i][1], matrix[i][2]);
}

void print_available(int available[3])
{
  printf("Available: %d %d %d\n",
         available[0], available[1], available[2]);
}

int main(void)
{
  printf("DEADLOCK DETECTION AND BANKER'S ALGORITHM\n");

  // Test 1: Deadlock.
  printf("\n=== TEST 1: DEADLOCK ===\n");

  int a1[5][3] = {
    {1, 0, 0}, {0, 1, 0}, {1, 0, 0}
  };
  int r1[5][3] = {
    {0, 1, 0}, {1, 0, 0}, {0, 1, 0}
  };
  int v1[3] = {0, 0, 0};

  print_matrix("Allocation", a1, 3);
  print_matrix("Request", r1, 3);
  print_available(v1);
  detect_deadlock(3, 3, a1, r1, v1);

  // Test 2: No deadlock.
  printf("\n=== TEST 2: NO DEADLOCK ===\n");

  int a2[5][3] = {
    {1, 0, 0}, {0, 1, 0}, {1, 0, 0}
  };
  int r2[5][3] = {
    {0, 1, 0}, {1, 0, 0}, {0, 0, 0}
  };
  int v2[3] = {0, 0, 0};

  print_matrix("Allocation", a2, 3);
  print_matrix("Request", r2, 3);
  print_available(v2);
  detect_deadlock(3, 3, a2, r2, v2);

  // Standard Banker's Algorithm example.
  printf("\n=== TEST 3: BANKER'S ALGORITHM ===\n");

  int allocation[N][M] = {
    {0, 1, 0},
    {2, 0, 0},
    {3, 0, 2},
    {2, 1, 1},
    {0, 0, 2}
  };

  int max[N][M] = {
    {7, 5, 3},
    {3, 2, 2},
    {9, 0, 2},
    {2, 2, 2},
    {4, 3, 3}
  };

  int need[N][M];
  int available[M] = {3, 3, 2};

  for (int i = 0; i < N; i++)
    for (int j = 0; j < M; j++)
      need[i][j] = max[i][j] - allocation[i][j];

  print_available(available);
  print_matrix("Allocation", allocation, N);
  print_matrix("Need", need, N);

  printf("\n--- Initial safety check ---\n");
  safety(N, M, allocation, need, available, 1);

  printf("\n--- Safe request: P0 requests (0,0,1) ---\n");
  int safe_request[M] = {0, 0, 1};
  request_resources(0, safe_request, allocation, need, available);

  printf("\n--- Unsafe request: P0 requests (0,0,2) ---\n");
  int unsafe_request[M] = {3, 3, 0};
  request_resources(0, unsafe_request, allocation, need, available);

  printf("\nAll tests completed.\n");
  exit(0);
}