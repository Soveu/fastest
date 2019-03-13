#include <vector>
#include <unistd.h>

class Buffer : public std::vector<char> {
 public:
  size_t written = 0;

  using std::vector<char>::data;
  using std::vector<char>::size;

  Buffer() : std::vector<char>(4096) {};
  Buffer(const size_t sz) : std::vector<char>(sz) {};

  ssize_t ReadFromFd(const int fd);
};

ssize_t Buffer::ReadFromFd(const int fd) {
  if(this->size() == this->written) {
    this->resize(this->size() * 2);
  }

  ssize_t bytesRead = read(fd,
                           this->data() + this->written,
                           this->size() - this->written);
  if(bytesRead == -1) {
    return -1;
  }

  this->written += bytesRead;

  return bytesRead;
}

