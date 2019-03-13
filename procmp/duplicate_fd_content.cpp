#include <unistd.h>

ssize_t duplicate_fd_content(int fdFrom, int fdOne, int fdTwo) {
  char buf[4096];

  ssize_t bytesRead = read(fdFrom, buf, sizeof(buf));
  if(bytesRead <= 0) {
    return bytesRead;
  }

  ssize_t bytesCopiedToFdOne = 0;
  while(bytesCopiedToFdOne < bytesRead) {
    ssize_t tmp = write(fdOne, buf, bytesRead-bytesCopiedToFdOne);
    if(tmp <= 0) {
      return tmp;
    }
    bytesCopiedToFdOne += tmp;
  }

  ssize_t bytesCopiedToFdTwo = 0;
  while(bytesCopiedToFdTwo < bytesRead) {
    ssize_t tmp = write(fdTwo, buf, bytesRead-bytesCopiedToFdTwo);
    if(tmp <= 0) {
      return tmp;
    }
    bytesCopiedToFdTwo += tmp;
  }

  return bytesRead;
}

