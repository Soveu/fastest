#include <fcntl.h>

/* WARNING: works only with pipes!
 * Also, benchmarks show no profit */
ssize_t duplicate_fd_content_tee(int fdFrom, size_t len, int fdOne, int fdTwo) {
  if(len == 0) {
    return 0;
  }

  ssize_t tlen = tee(fdFrom, fdOne, len, SPLICE_F_MOVE);

  if(tlen < 0) {
    return len;
  }

  ssize_t slen = 0;
  while(slen < tlen) {
    ssize_t tmp = splice(fdFrom, 0,
                         fdTwo,  0,
                         tlen, SPLICE_F_MOVE);

    if(tmp <= 0) {
      return tmp;
    }

    slen += tmp;
  }

  return tlen;
}

