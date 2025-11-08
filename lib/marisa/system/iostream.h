#ifndef MARISA_SYSTEM_IOSTREAM_H_
#define MARISA_SYSTEM_IOSTREAM_H_

#include <iostream>

#include "marisa/iostub.h"

namespace marisa {

class StreamReader : public marisa::Reader {
 public:
  StreamReader(std::istream &stream);

  void read(void *buf, size_t n) override;
  void seek(size_t n) override;
};

class StreamWriter : public marisa::Writer {
 public:
  StreamWriter(std::ostream &stream);

  void write(const void *buf, size_t n) override;
  void seek(size_t n) override;
};

}  // namespace marisa

#endif
