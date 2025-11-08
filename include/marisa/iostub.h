#ifndef MARISA_IOSTUB_H_
#define MARISA_IOSTUB_H_

#include <cstddef>

namespace marisa {

class Reader {
 public:
  Reader() = default;

  Reader(const Reader &) = delete;
  Reader &operator=(const Reader &) = delete;

  virtual ~Reader() = default;

  // read n bytes into buf, throwing an exception on failure
  virtual void read(void *buf, size_t n) = 0;

  // skip n bytes, throwing an exception on failure
  virtual void seek(size_t n) = 0;
};

class Writer {
 public:
  Writer() = default;

  Writer(const Writer &) = delete;
  Writer &operator=(const Writer &) = delete;

  virtual ~Writer() = default;

  // write n bytes from buf, throwing an exception on failure
  virtual void write(const void *buf, size_t n) = 0;

  // write n zeros, throwing an exception on failure
  virtual void seek(size_t n) = 0;
};

}  // namespace marisa

#endif  // MARISA_IOSTUB_H_
