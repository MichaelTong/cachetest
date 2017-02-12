#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <inttypes.h>
#include <linux/fs.h>
#include <sys/ioctl.h>

// compile: gcc replay.c -pthread

//Note: all sizes are in block (1 block = block_size bytes)

// CONFIGURATION PART


int MEM_ALIGN = 4096; //bytes
int numworkers = 128; // =number of threads
int block_size = 512;
int64_t DISK_SIZE_IN_SECTS = 0;
pthread_t *tid;
// ANOTHER GLOBAL VARIABLES
int fd;

int64_t get_disksz_in_sects(int devfd)
{
  int64_t sz;
  ioctl(devfd, BLKGETSIZE64, &sz);
  return sz/512;
}

void *performIO(){
  int64_t offset;
  void* buff;
  posix_memalign(&buff, MEM_ALIGN, 4096);
  while(1) {
    offset = rand()%(DISK_SIZE_IN_SECTS - 16);
    pread(fd, buff, 4096, offset*512);
  }
  return NULL;
}

void operateWorkers(){
  tid = malloc(numworkers * sizeof(pthread_t));
  if(tid == NULL){
    fprintf(stderr,"Error malloc thread!\n");
    exit(1);
  }

  assert(pthread_mutex_init(&lock, NULL) == 0);

  for(x = 0; x < numworkers; x++){
    pthread_create(&tid[x], NULL, performIO, NULL);
  }

  for(x = 0; x < numworkers; x++){
    pthread_join(tid[x], NULL);
  }
  assert(pthread_mutex_destroy(&lock) == 0);

}

int main(int argc, char *argv[]) {
  char device[64];

  srand(time(NULL));
  if (argc != 2){
    printf("Usage: ./replayer /dev/sda\n");
    exit(1);
  }else{
    printf("%s\n", argv[1]);
    sprintf(device,"%s",argv[1]);
    printf("Disk ==> %s\n", device);
  }

  // start the disk part
  fd = open(device, O_DIRECT | O_SYNC | O_RDWR);
  if(fd < 0) {
    fprintf(stderr,"Cannot open %s\n", device);
    exit(1);
  }

  DISK_SIZE_IN_SECTS = get_disksz_in_sects(fd);

  if (posix_memalign(&buff,MEM_ALIGN,LARGEST_REQUEST_SIZE * block_size)){
    fprintf(stderr,"memory allocation failed\n");
    exit(1);
  }

  operateWorkers();

  free(buff);

  return 0;
}
