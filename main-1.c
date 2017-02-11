#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

typedef struct fourKB {
  int x;
  int rst[1023];
} fourKB;

typedef struct BigMEMBlock {
  fourKB fourkb[256];
} BigMEMBlock;

size_t MEMBLOCKSIZE = sizeof(BigMEMBlock);

int numblocks = 1024 ;
int numworkers = 2;
int runtime = 15;
int prepare = 5;


BigMEMBlock *mempool;
pthread_t *tid;
pthread_mutex_t filelock;
FILE* logfd;
pthread_t timer_td;
int shouldend = 0;
int background = 0;
int record = 0;

long setmem() {
  int idx = rand() % numblocks;
  register int newvalue = rand();
  register int i;
  struct timespec begin, end;
  long timediff;
  BigMEMBlock *source = mempool+idx;
  BigMEMBlock tmp;

  clock_gettime(CLOCK_MONOTONIC, &begin);
  memcpy(&tmp, source, sizeof(MEMBLOCKSIZE));
  for (i = 0; i < 256; i++) {
    source->fourkb[i].x = newvalue;
  }
  //memset(target, newvalue, sizeof(MEMBLOCKSIZE));
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
    if (record && !background && timeidx < 10000) {
      timerecords[timeidx] = timediff;
      timeidx ++;
    }
    if (record && !background && timeidx == 10000) {
      flushlog(timerecords, 10000);
      timeidx = 0;
    }
  }
  if (record && !background && timeidx != 0) {
    flushlog(timerecords, timeidx);
  }
  return NULL;
}

void *timerend() {
  int i;
  for (i = 0; background || i < prepare; i++) {
    if (!background) {
      printf("Prepare: %d/%d                \r", i, prepare);
      fflush(stdout);
    }
    sleep(1);
  }
  record = 1;
  for (i = 0; background || i < runtime; i++) {
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
  int i;
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
  for(i = 0; i < numworkers; i ++) {
    pthread_create(&tid[i], NULL, dowork, NULL);
  }
  pthread_create(&timer_td, NULL, timerend, NULL);

  pthread_join(timer_td, NULL);
  for (i = 0; i < numworkers; i++) {
    pthread_join(tid[i], NULL);
  }
  pthread_mutex_destroy(&filelock);
  if (!background) {
    fclose(logfd);
  }
  free(mempool);
  free(tid);
}
