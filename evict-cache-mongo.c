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
#include <error.h>
#include <errno.h>
#include <string.h>

#define PAGESIZE 4096
unsigned int junk_counter;
int main(int argc, char** argv) {
  unsigned long long i, length, numpages, inmem=0;
  int fd;
  char * mfd;
  //  char *evict_start;
  char * filename = argv[1];
  unsigned char *min_array;
  double percentage = atof(argv[2]), new_percentage=0;
  srand((unsigned) time(NULL));

  fd = open(filename, O_NOATIME | O_RDONLY);
  length = lseek(fd, 0, SEEK_END);
  numpages = length/PAGESIZE;
  //  printf("length %lld\n", length);
  lseek(fd, 0, SEEK_SET);
  mfd = mmap(NULL, length, PROT_READ, MAP_SHARED, fd, 0);
  min_array = malloc(numpages);
  mincore(mfd, length, min_array);
  for (i = 0; i < numpages; i++) {
    if (min_array[i] & 0x1)
      inmem++;
  }
  if (inmem == numpages) {
    return 0;
  }
  new_percentage = numpages*percentage/(numpages - inmem);
  printf("length %llu, numpages %llu, inmem %llu, percentage %.2f%%, new_percentage %.2f%%\n", length, numpages, inmem, percentage, new_percentage);
  for (i = 0; i < numpages; i++) {
    junk_counter += ((char*)mfd)[i*PAGESIZE];
  }
  munmap(mfd, length);

  lseek(fd, 0, SEEK_SET);
  mfd = mmap(NULL, length, PROT_READ, MAP_SHARED, fd, 0);
  for (i = 0; i < numpages; i++) {
    int rn = rand() % 10000000;
    if (rn < 100000 * new_percentage && !(min_array[i] & 0x1)) {
      posix_fadvise(fd, i*PAGESIZE, PAGESIZE, POSIX_FADV_DONTNEED);
    }
  }
  /*
  for (i = 0; i < numpages; i++) {
    if (!(min_array[i] & 0x1)) {
      int rn = rand() %10000000;
      if (rn > 100000 * new_percentage) {
        junk_counter += ((char*)mfd)[i*PAGESIZE];
      }
    }
    }*/
  /*
  for (i = 0; i < numpages; i++) {
    junk_counter += ((char*)mfd)[i*PAGESIZE];
  }
  for (i = 0; i < numpages; i++) {
    int rn = rand() % 10000;
    if (rn < 100 * new_percentage && !(min_array[i] & 0x1)) {
      posix_fadvise(fd, i*PAGESIZE, PAGESIZE, POSIX_FADV_DONTNEED);
    }
    }*/
  /*  if (posix_fadvise(fd, length/4, length/2, POSIX_FADV_DONTNEED))
    printf("unable to posix_fadvise file %s (%s)", "xxx", strerror(errno));
  if(msync(evict_start, pages/2*4096, MS_INVALIDATE)) {
    perror("");
    }*/
  munmap(mfd, length);
  close(fd);
}
