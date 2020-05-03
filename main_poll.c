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

const char MEMFILENAME[] = "fastest_stdin";

bool revent_is_ok(int revent) {
  return !(revent == POLLNVAL || revent == POLLERR || revent == POLLHUP);
}
bool revent_can_read(int revent) {
  return revent == POLLIN || revent == POLLRDNORM 
    || revent == POLLRDBAND || revent == POLLPRI;
}

bool revents_are_ok(struct pollfd* fds, size_t n) {
  for(size_t i=0; i<n; ++i) {
    if(!revent_is_ok(fds[i].revents)) return false;
  }

  return true;
}

const int N = 2;

/* TODO: refactor it using for loops */
int main(int argc, char** argv) {
  if(argc < 3) {
    /* TODO: usage */
    puts("USAGE");
    return 1;
  }

  int memfd = create_memfile_from(0, MEMFILENAME);
  if(memfd == -1) {
    return -1;
  }

  struct ProcessChild child[N];
  struct pollfd fds[N];
  struct Buffer buffer[N];

  int initialized = 0;
  for(; initialized < N; ++initialized) {
    int fd = shm_open(MEMFILENAME, O_RDONLY, 0);
    if(fd == -1) goto loop_open_error;

    child[initialized] = spawn_process_child(argv[initialized], 0, fd, -1);
    if(child[initialized].status != 0) goto loop_child_error;

    buffer[initialized] = buffer_with_capacity(4096);
    if(buffer[initialized].ptr == 0) goto loop_buffer_error;

    fds[initialized].fd = child[initialized].stdout;
    fds[initialized].events = POLLIN;
    continue;

loop_buffer_error:
    kill(child[initialized].pid, 9);
loop_child_error:
    close(fd);
loop_open_error:
    goto destructors;
  };

  do {
    int x = poll(fds, 2, 3000);

    if(x < 0) {
      perror("poll");
      goto destructors;
    }

    /* Timeout, check if something happened to children */
    if(x == 0 && are_children_ok(child, N)) {
      puts("Something happened to one of the children");
      goto destructors;
    }

    for(int i=0; i<N; ++i) {
      if(!child[i].running) continue;
      if(!is_child_ok(&child[i])) {
        puts("One of the children returned with status other than 0");
        goto destructors;
      }
      if(!revent_can_read(fds[i].revents)) continue;

      ssize_t bytes_read = read_fd_to_buffer(fds[i].fd, &buffer[i]);
      if(bytes_read >= 0) continue;

      perror("main read_fd_to_buffer");
      goto destructors;
    }
  } while(revents_are_ok(fds, N));

  if(buffer[0].size != buffer[1].size) {
    puts("outputs dont have equal size");

    /*
    char _msg1[] = "First:\n";
    write(1, _msg1, sizeof(_msg1));
    write(1, buffer[0].ptr, buffer[0].size);
    char _msg2[] = "\nSecond:\n";
    write(1, _msg2, sizeof(_msg2));
    write(1, buffer[1].ptr, buffer[1].size);
    char _msg3[] = "\n";
    write(1, _msg3, sizeof(_msg3));
    */

    goto destructors;
  }

  if(memcmp(buffer[0].ptr, buffer[1].ptr, buffer[0].size) != 0) {
    puts("outputs arent equal");

    /*
    char _msg1[] = "First:\n";
    write(1, _msg1, sizeof(_msg1));
    char _msg2[] = "\nSecond:\n";
    write(1, _msg2, sizeof(_msg2));
    char _msg3[] = "\n";
    write(1, _msg3, sizeof(_msg3));
    */

    goto destructors;
  }

  puts("yaaay");

destructors:
  for(int i=0; i<initialized; ++i) {
    munmap(buffer[i].ptr, buffer[i].capacity);
    close(child[i].stdin);
    if(child[i].running) kill(child[i].pid, 9);
  }
  close(memfd);
unlink_memfd:
  shm_unlink(MEMFILENAME);

  return 0;
}

