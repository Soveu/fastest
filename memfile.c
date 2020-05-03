#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

/* WARNING: this function assumes write() will always write all the data.
 * This approach is sufficient for writing into files, however it
 * should not be used with pipes
 */
int copy_fd_to_memfd(int fd, int memfd) {
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
  int memfd = shm_open(filename, flags, 01600);

  if(memfd == -1) {
    perror("shm_open");
    return -1;
  }

  struct stat s;
  if(fstat(fd, &s) == -1) {
    perror("fstat");
    goto error;
  }

  /* If fd is a regular file, preallocate enough memory */
  if(S_ISREG(s.st_mode) && fallocate(memfd, 0, 0, s.st_size) == -1) {
    perror("fallocate");
    goto error;
  }

  if(copy_fd_to_memfd(fd, memfd) == -1) {
    goto error;
  }

  /* Mark the file and file descriptor as readonly */
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
  shm_unlink(filename);
  return -1;
}

