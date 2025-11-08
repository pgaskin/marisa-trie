#ifndef MARISA_SYSTEM_FILE_H_
#define MARISA_SYSTEM_FILE_H_

#include "marisa/iostub.h"

namespace marisa::system {

class Mapper {
 public:
  Mapper(const char *filename, int flags = 0);
  ~Mapper();

  Mapper(const Mapper &) = delete;
  Mapper &operator=(const Mapper &) = delete;

  const void *data() const;
  size_t size() const;

 private:
  void *data_ = nullptr;
  std::size_t size_ = 0;
#if (defined _WIN32) || (defined _WIN64)
  void *file_ = nullptr;
  void *map_ = nullptr;
#else   // (defined _WIN32) || (defined _WIN64)
  int fd_ = -1;
#endif  // (defined _WIN32) || (defined _WIN64)
};

class Reader : public marisa::Reader {
 public:
  Reader(int fd);

  void read(void *buf, size_t n) override;
  void seek(size_t n) override;

 private:
  int fd_ = -1;
};

class Writer : public marisa::Writer {
 public:
  Writer(int fd);

  void write(const void *buf, size_t n) override;
  void seek(size_t n) override;

 private:
  int fd_ = -1;
};

}  // namespace marisa::system

#endif
