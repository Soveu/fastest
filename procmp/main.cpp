#include <sys/wait.h>   /* waitpid */
#include <unistd.h>     /* close */

#include <cstdio>       /* perror, puts */
#include <cstring>      /* std::memcmp */
#include <algorithm>    /* std::min */

#include "pchild.cpp"
#include "buffer.cpp"
#include "duplicate_fd_content.cpp"

/* TODO: the child arent terminated when an error occures */
int main(int argc, char** argv, char** envp) {
  Pchild    tester, testee;
  Buffer    buffer, buffee;
  int       status;

  if(argc < 3) {
    puts("Usage: <tester> <testee>");
    return 2;
  }

  if(spawn_pchild(&tester, argv[1], 0, envp) == -1) {
    perror("spawnPipedChild (tester)");
    return 3;
  }

  if(spawn_pchild(&testee, argv[2], 0, envp) == -1) {
    perror("spawnPipedChild (testee)");
    return 4;
  }

  bool canRead = true;
  ssize_t a, b;
  do {
    if(canRead) {
      ssize_t n;

      /* This prevents from getting a deadlock when a process needs whole input
       * to produce output. 
       * 4096 is the size of the buffer inside duplicate_pipe_content() function
       */
      do {
        n = duplicate_fd_content(STDIN_FILENO,
                                 tester.pipeIn, testee.pipeIn);
      } while(n == 4096);

      if(n <= 0) {
        if(n == -1) {
          perror("duplicate_pipe_content");
          return 5;
        }

        close(tester.pipeIn);
        close(testee.pipeIn);
        canRead = false;
      }
    }

    a = buffer.ReadFromFd(tester.pipeOut);
    b = buffee.ReadFromFd(testee.pipeOut);

    if(a == -1 || b == -1) {
      perror("ReadFromFd");
      return 6;
    }
  } while(a > 0 && b > 0);

  close(tester.pipeOut);
  close(testee.pipeOut);
  /* TODO: what if child gets terminated? */
  waitpid(tester.pid, &status, 0);
  waitpid(testee.pid, &status, 0);

  if(buffer.written == buffee.written && std::memcmp(buffer.data(), buffee.data(), buffer.written)) {
    write(1, "\nExpected: --------\n", 20);
    write(1, buffer.data(), buffer.written);
    write(1, "\nGot: -------------\n", 20);
    write(1, buffee.data(), buffee.written);

    return 1;
  }

  return 0;
}

