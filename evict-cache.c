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

int main(int argc, char** argv) {
  unsigned long long i, length, numpages;
  int fd;
  char * mfd;
  char *evict_start;
  char * filename = argv[1];
  double percentage = atof(argv[2]);
  srand((unsigned) time(NULL));

  fd = open(filename, O_NOATIME | O_RDONLY);
  length = lseek(fd, 0, SEEK_END);
  numpages = length/PAGESIZE;
  printf("length %llu, numpages %llu, percentage %.2f%%\n", length, numpages, percentage);
  lseek(fd, 0, SEEK_SET);
  mfd = mmap(NULL, length, PROT_READ, MAP_SHARED, fd, 0);

  for (i = 0; i < numpages; i++) {
    int rn = rand() % 10000;
    if (rn < 100 * percentage) {
      posix_fadvise(fd, i*PAGESIZE, PAGESIZE, POSIX_FADV_DONTNEED);
    }
  }
  /*  if (posix_fadvise(fd, length/4, length/2, POSIX_FADV_DONTNEED))
    printf("unable to posix_fadvise file %s (%s)", "xxx", strerror(errno));
  if(msync(evict_start, pages/2*4096, MS_INVALIDATE)) {
    perror("");
    }*/
  munmap(mfd, length);
  close(fd);
}
