#ifndef MARISA_SYSTEM_STDIO_H_
#define MARISA_SYSTEM_STDIO_H_

#include <cstdio>

#include "marisa/iostub.h"

namespace marisa::system {

class StdioReader : public marisa::Reader {
 public:
  StdioReader(const char *filename);
  StdioReader(std::FILE *file);
  ~StdioReader() override;

  void read(void *buf, size_t n) override;
  void seek(size_t n) override;
};

class StdioWriter : public marisa::Writer {
 public:
  StdioWriter(const char *filename);
  StdioWriter(std::FILE *file);
  ~StdioWriter() override;

  void write(const void *buf, size_t n) override;
  void seek(size_t n) override;
};

}  // namespace marisa::system

#endif
