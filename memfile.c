#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

int transfer_pipe_to_fd(int pipe, int fd) {
  ssize_t bytes_transferred;
  do {
    bytes_transferred = splice(pipe, 0, fd, 0, 65536, 0);
  } while(bytes_transferred > 0);

  if(bytes_transferred == -1) {
    perror("splice");
    return -1;
  }

  return 0;
}

/* WARNING: this function assumes write() will always write all the data.
 * This approach is sufficient for writing into files, however this approach
 * should not be used with pipes
 */
int copy_file_to_memfd(int fd, int memfd) {
  char buffer[65536];

  ssize_t bytes_read = read(fd, buffer, sizeof(buffer));
  while(bytes_read > 0) {
    write(memfd, buffer, bytes_read);
    bytes_read = read(fd, buffer, sizeof(buffer));
  }

  if(bytes_read == -1) {
    perror("read");
    return -1;
  }

  return 0;
}

int create_memfile_from(int fd, const char* filename) {
  int flags = O_CREAT | O_EXCL | O_RDWR;
  int memfd = open(filename, flags, 01600);

  if(memfd == -1) {
    perror("failed to create file at /dev/shm");
    return -1;
  }

  struct stat s;
  if(fstat(fd, &s) == -1) {
    perror("fstat");
    goto error;
  }

  int ret = -1;

  if(S_ISREG(s.st_mode) && s.st_size != 0) {
    if(fallocate(memfd, 0 /* allocate space */, 0, s.st_size) == -1) {
      perror("fallocate");
      goto error;
    }
    ret = copy_file_to_memfd(fd, memfd);
  } else {
    ret = transfer_pipe_to_fd(fd, memfd);
  }

  if(ret == -1) {
    goto error;
  }

  int newflags = O_RDONLY;
  if(fcntl(memfd, F_SETFD, newflags) == -1) {
    perror("fcntl");
    goto error;
  }
  if(fchmod(memfd, 01400) == -1) {
    perror("fchmod");
    goto error;
  }

  return memfd;
error:
  close(memfd);
  unlink(filename);
  return -1;
}

