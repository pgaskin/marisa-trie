#ifndef MARISA_IOSTUB_H_
#define MARISA_IOSTUB_H_

#include <cstddef>

namespace marisa {

class Reader {
 public:
  // read n bytes into buf, throwing an exception on failure
  virtual void read(void *buf, size_t n) = 0;
  // skip n bytes, throwing an exception on failure
  virtual void seek(size_t n) = 0;
};

class Writer {
 public:
  // write n bytes from buf, throwing an exception on failure
  virtual void write(const void *buf, size_t n) = 0;
  // write n zeros, throwing an exception on failure
  virtual void seek(size_t n) = 0;
};

}  // namespace marisa

#endif  // MARISA_IOSTUB_H_
