#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>

typedef struct BigMEMBlock {
  unsigned char onebyte[4096];
} BigMEMBlock;

unsigned long long numblocks = (51ULL * 1024) * 1024 * 1024 / sizeof(BigMEMBlock);
int numworkers = 1;
int runtime = 30;
int prepare = 5;

size_t MEMBLOCKSIZE = sizeof(BigMEMBlock);
BigMEMBlock* mfd;
pthread_t *tid;
pthread_mutex_t filelock;
FILE* logfd;
int fd;
pthread_t timer_td;
int shouldend = 0;
int background = 0;
int record = 0;

long readmmap() {
  int idx = rand() % numblocks;
  struct timespec begin, end;
  long timediff;
  //char *source = mfd;
  //char tmp;
  sleep(1);
  return 0;
  int ret;
  BigMEMBlock *source = ((BigMEMBlock* )mfd) + idx;
  BigMEMBlock tmp;
  //ret = syscall(324, source);
  clock_gettime(CLOCK_MONOTONIC, &begin);
  memcpy(&tmp, source, MEMBLOCKSIZE);
  clock_gettime(CLOCK_MONOTONIC, &end);
  timediff = (end.tv_sec - begin.tv_sec) * 1000000000 + (end.tv_nsec - begin.tv_nsec);
  if (ret < 1) {
    timediff = -1000000;
  }
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
  printf("Child Pid: %ld\n", syscall(SYS_gettid));
  while(!shouldend) {
    timediff = readmmap();
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
  char *filenum;
  char filename[100];
  unsigned long long length;
  srand((unsigned) time(NULL));

  tid = malloc(numworkers * sizeof(pthread_t));
  printf("Main Pid: %ld\n", syscall(SYS_gettid));
  sleep(10);
  if (argc > 1) {
    filenum = argv[1];
    if (argc > 2) {
      background = 1;
      runtime += 5;
    }
  }
  mfd = (BigMEMBlock *)malloc(numblocks*sizeof(BigMEMBlock));  

  srand((unsigned) time(NULL)); 
  for (i = 0; i < numblocks; i ++) {
    memset(&mfd[i], rand()%256, sizeof(BigMEMBlock));
  } 
  if (!background) {
    logfd = fopen("outputs/memaccess.log", "w");
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
}
