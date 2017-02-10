#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <unistd.h>


typedef struct BigMEMBlock {
  unsigned char onebyte[1024*1024];
} BigMEMBlock;

int numblocks = 1024;
int numworkers = 16;
int runtime = 30;


size_t MEMBLOCKSIZE = sizeof(BigMEMBlock);
BigMEMBlock *mempool;
pthread_t *tid;
pthread_mutex_t filelock;
FILE* logfd;
pthread_t timer_td;
int shouldend = 0;
int background = 0;

long setmem() {
  int idx = rand() % numblocks;
  struct timespec begin, end;
  long timediff;
  BigMEMBlock *target = mempool+idx;

  clock_gettime(CLOCK_MONOTONIC, &begin);
  memset(target, 0, sizeof(MEMBLOCKSIZE));
  clock_gettime(CLOCK_MONOTONIC, &end);
  timediff = (end.tv_sec - begin.tv_sec) * 1000000000 + (end.tv_nsec - begin.tv_nsec);
  return timediff;
}

void flushlog(long *timerecords, int size) {
  int i = 0;
  if (background) {
    return;
  }
  pthread_mutex_lock(&filelock);
  for (i = 0; i < size; i++) {
    fprintf(logfd, "%ld\n", timerecords[i]);
  }
  pthread_mutex_unlock(&filelock);
}

void *dowork() {
  long timerecords[10000], timediff;
  int timeidx = 0;
  while(!shouldend) {
    timediff = setmem();
    if (!background && timeidx < 10000) {
      timerecords[timeidx] = timediff;
      timeidx ++;
    }
    if (!background && timeidx == 10000) {
      flushlog(timerecords, 10000);
      timeidx = 0;
    }
  }
  if (!background && timeidx != 0) {
    flushlog(timerecords, 10000);
  }
  return NULL;
}

void *timerend() {
  for (int i = 0; i < runtime; i++) {
    if (!background) {
      printf("Progress: %d/%d               \r", i, runtime);
    }
    fflush(stdout);
    sleep(1);
  }
  shouldend = 1;
  return NULL;
}


int main(int argc, char** argv) {
  mempool = (BigMEMBlock *)calloc(numblocks, sizeof(BigMEMBlock));
  tid = malloc(numworkers * sizeof(pthread_t));

  if (argc > 1) {
    background = 1;
    runtime += 5;
  }

  if (!background) {
    logfd = fopen("memaccess.log", "w");
  }

  pthread_mutex_init(&filelock, NULL);
  for(int i = 0; i < numworkers; i ++) {
    pthread_create(&tid[i], NULL, dowork, NULL);
  }
  pthread_create(&timer_td, NULL, timerend, NULL);

  pthread_join(timer_td, NULL);
  for (int i = 0; i < numworkers; i++) {
    pthread_join(tid[i], NULL);
  }
  pthread_mutex_destroy(&filelock);
  if (!background) {
    fclose(logfd);
  }
  free(mempool);
  free(tid);
}
