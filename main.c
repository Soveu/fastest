#define _GNU_SOURCE

#include <signal.h>
#include <poll.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdbool.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "memfile.c"
#include "process_child.c"
#include "buffer.c"

const char MEMFILENAME[] = "/dev/shm/fastest";

bool revent_is_ok(int revent) {
  return !(revent == POLLNVAL || revent == POLLERR || revent == POLLHUP);
}
bool revent_can_read(int revent) {
  return revent == POLLIN || revent == POLLRDNORM 
    || revent == POLLRDBAND || revent == POLLPRI;
}

int main(int argc, char** argv) {
  if(argc < 3) {
    /* TODO: usage */
    puts("USAGE");
    return 1;
  }

  int memfd = create_memfile_from(0, MEMFILENAME);
  if(memfd == -1) {
    goto close_memfd;
  }

  int fd[2] = {
    open(MEMFILENAME, O_RDONLY),
    open(MEMFILENAME, O_RDONLY)
  };

  if(fd[0] == -1 || fd[1] == -1) {
    perror("open");
    goto close_fd;
  }

  struct ProcessChild child[2] = {
    spawn_process_child(argv[1], 0, fd[0], -1),
    spawn_process_child(argv[2], 0, fd[1], -1),
  };

  for(int i=0; i<2; ++i) {
    if(child[i].running) continue;
    goto kill_children;
  }
  
  struct pollfd fds[2] = {
    [0] = {
      .fd = child[0].stdout,
      .events = POLLIN,
      .revents = 0,
    },
    [1] = {
      .fd = child[1].stdout,
      .events = POLLIN,
      .revents = 0,
    },
  };

  struct Buffer buffer[2] = {
    buffer_with_capacity(4096),
    buffer_with_capacity(4096),
  };

  if(buffer[0].ptr == 0 || buffer[1].ptr == 0) {
    goto destructors;
  }

  do {
    int x = poll(fds, 2, 3000);

    if(x < 0) {
      perror("poll");
      goto destructors;
    }

    if(x == 0) {
      puts("Timeout");

      for(int i=0; i<2; ++i) {
        int pid = waitpid(child[i].pid, &child[i].status, WNOHANG);

        /* Nothing happened */
        if(pid == 0) continue; 

        /* This should NOT happen */
        if(pid == -1) {
          perror("waitpid (after poll timeout)");
          goto destructors;
        }

        child[i].running = false;
        goto destructors;
      }
    }

    for(int i=0; i<2; ++i) {
      if(!revent_can_read(fds[i].revents)) continue;

      ssize_t bytes_read = read_fd_to_buffer(fds[i].fd, &buffer[i]);
      if(bytes_read >= 0) continue;

      perror("main read_fd_to_buffer");
      goto destructors;
    }
  } while(revent_is_ok(fds[0].revents) || revent_is_ok(fds[1].revents));

  if(buffer[0].size != buffer[1].size) {
    puts("buffers dont have equal size");

    char _msg1[] = "First:\n'";
    write(1, _msg1, sizeof(_msg1));
    write(1, buffer[0].ptr, buffer[0].size);
    char _msg2[] = "'\nSecond:\n'";
    write(1, _msg2, sizeof(_msg2));
    write(1, buffer[1].ptr, buffer[1].size);
    char _msg3[] = "'\n";
    write(1, _msg3, sizeof(_msg3));

    goto destructors;
  }

  if(memcmp(buffer[0].ptr, buffer[1].ptr, buffer[0].size) != 0) {
    puts("buffers arent equal");

    char _msg1[] = "First:\n'";
    write(1, _msg1, sizeof(_msg1));
    char _msg2[] = "'\nSecond:\n'";
    write(1, _msg2, sizeof(_msg2));
    char _msg3[] = "'\n";
    write(1, _msg3, sizeof(_msg3));

    goto destructors;
  }

  puts("yaaay");

destructors:
dealloc_buffers:
  for(int i=0; i<2; ++i) {
    munmap(buffer[i].ptr, buffer[i].capacity);
  }

kill_children:
  for(int i=0; i<2; ++i) {
    if(!child[i].running) continue;
    kill(child[i].pid, 9);
    waitpid(child[i].pid, 0, 0);
  }
close_fd:
  for(int i=0; i<2; ++i) {
    close(fd[i]);
  }
close_memfd:
  close(memfd);
  unlink(MEMFILENAME);

  return 0;
}

