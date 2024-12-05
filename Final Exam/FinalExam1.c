#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


#define KEY 1234

union semun {
  int val;
  struct semid_ds *buf;
  ushort *array;
};

void performSemaphoreOperation(int sem_id, int sem_num, int sem_op) {
  struct sembuf semaphore;
  semaphore.sem_num = sem_num;
  semaphore.sem_op = sem_op;
  semaphore.sem_flg = 0; // No special flags

  if (semop(sem_id, &semaphore, 1) == -1) {
    perror("Semaphore operation failed");
    exit(EXIT_FAILURE);
  }
}

int main() {
  int sem_id, num_processes = 5;

  if ((sem_id = semget(KEY, 1, IPC_CREAT | 0666)) == -1) {
    perror("Semaphore creation failed");
    exit(EXIT_FAILURE);
  }

  union semun arg;
  arg.val = 1;
  if (semctl(sem_id, 0, SETVAL, arg) == -1) {
    perror("Semaphore initialization failed");
    exit(EXIT_FAILURE);
  }

  for (int i = 0; i < num_processes; ++i) {
    pid_t pid = fork();

    if (pid == -1) {
      perror("Fork failed");
      exit(EXIT_FAILURE);
    } else if (pid == 0) {

      printf("Child %d attempting to enter critical section...\n", i);

      performSemaphoreOperation(sem_id, 0, -1);

      printf("Child %d in critical section\n", i);
      sleep(2);

      performSemaphoreOperation(sem_id, 0, 1);

      exit(EXIT_SUCCESS);
    }
  }

  for (int i = 0; i < num_processes; ++i) {
    wait(NULL);
  }

  if (semctl(sem_id, 0, IPC_RMID) == -1) {
    perror("Semaphore removal failed");
    exit(EXIT_FAILURE);
  }

  return 0;
}
