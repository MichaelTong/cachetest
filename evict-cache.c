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

int main(int argc, char** argv) {
  int i;
  unsigned long long length, pages;
  int fd;
  char * mfd;
  char *evict_start;

  fd = open("imgs/random-1.img", O_NOATIME | O_RDONLY);
  length = lseek(fd, 0, SEEK_END);
  pages = length/4096;
  printf("length %lld\n", length);
  lseek(fd, 0, SEEK_SET);
  mfd = mmap(NULL, length, PROT_READ, MAP_SHARED, fd, 0);

  evict_start = mfd + pages/4*4096;
  if (posix_fadvise(fd, length/4, length/2, POSIX_FADV_DONTNEED))
    printf("unable to posix_fadvise file %s (%s)", "xxx", strerror(errno));
  if(msync(evict_start, pages/2*4096, MS_INVALIDATE)) {
    perror("");
  }
  munmap(mfd, length);
  close(fd);
}
