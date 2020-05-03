#pragma once

#include <sys/mman.h>
#include <string.h>
#include <unistd.h>

#include "process_child.c"

struct Buffer {
  char* ptr;
  size_t size;
  size_t capacity;
};

/* Creates and returns a new buffer with given capacity.
 * If allocation of memory fails, program is shut down
 */
struct Buffer buffer_with_capacity(size_t n) {
  const int prot = PROT_READ | PROT_WRITE;
  const int flags = MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE;

  struct Buffer buf = {
    .ptr = mmap(0, n, prot, flags, 0, 0),
    .size = 0,
    .capacity = n
  };

  if(MAP_FAILED == buf.ptr) {
    // bruh
    perror("mmap");
    buf.ptr = 0;
  }

  return buf;
}

/* Reads data from a file descriptor into a Buffer, using only one read() call.
 * Reads maximum of buf->capacity minus buf->size bytes.
 * If the remaining space is small enough, the buffer is resized.
 * If allocation of memory fails, program is shut down.
 * Returns the value returned by read().
 */
ssize_t read_fd_to_buffer_once(int fd, struct Buffer* buf) {
  const int prot = PROT_READ | PROT_WRITE;
  const int flags = MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE;

  if(buf->capacity - buf->size < 1024) {
    char* new = mmap(0, 2 * buf->capacity, prot, flags, 0, 0);

    if(MAP_FAILED == new) {
      // bruh
      buf->ptr = 0;
      return -1;
    }

    memcpy(buf->ptr, new, buf->size);
    munmap(buf->ptr, buf->capacity);
    buf->ptr = new;
    buf->capacity *= 2;
  }

  ssize_t bytes_read = read(fd, buf->ptr + buf->size, buf->capacity - buf->size);
  if(bytes_read <= 0) {
    return bytes_read;
  }

  buf->size += bytes_read;
  return bytes_read;
}

/* Reads data from a file descriptor into a Buffer, until there __might__ be
 * no more data ready to be recieved.
 * Returns the last value of read()
 */
ssize_t read_fd_to_buffer(int fd, struct Buffer* buf) {
  ssize_t x;

  do {
    x = read_fd_to_buffer_once(fd, buf);
  } while(x > 0 && buf->size == buf->capacity);

  return x;
}

uint64_t get_hash(const struct Buffer* buf) {
  uint64_t hash = 0;

  uint64_t* ptr = (uint64_t*)(buf->ptr);
  const size_t n = (buf->size) / 8;

  for(size_t i=0; i<n; ++i) {
    hash ^= *ptr;
    ++ptr;
  }

  uint64_t rest = 0;
  char* rest_ptr = (char*)ptr;
  const size_t rest_n = (buf->size) % 8;

  for(size_t i=0; i<rest_n; ++i) {
    rest |= rest_ptr[i];
    rest <<= 8;
  }

  return hash ^ rest ^ (buf->size);
}
