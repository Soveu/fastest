#include <sys/mman.h>
#include <unistd.h>

/* WARNING: works only with objects with known lenght! */
/* TODO: it loops in one of the write()s */
ssize_t duplicate_fd_content_mmap(int fdFrom, size_t len, int fdOne, int fdTwo) {
  char* content = (char*)mmap(NULL, len, PROT_READ, MAP_PRIVATE, fdFrom, 0);

  ssize_t bytesCopiedToFdOne = 0;
  while(bytesCopiedToFdOne < len) {
    ssize_t tmp = write(fdOne, content+bytesCopiedToFdOne, len-bytesCopiedToFdOne);
    if(tmp <= 0) {
      return tmp;
    }
    bytesCopiedToFdOne += tmp;
  }

  ssize_t bytesCopiedToFdTwo = 0;
  while(bytesCopiedToFdTwo < len) {
    ssize_t tmp = write(fdTwo, content+bytesCopiedToFdTwo, len-bytesCopiedToFdTwo);
    if(tmp <= 0) {
      return tmp;
    }
    bytesCopiedToFdTwo += tmp;
  }

  munmap(content, len);
  return len;
}

