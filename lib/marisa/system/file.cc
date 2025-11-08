#if (defined _WIN32) || (defined _WIN64)
 #include <io.h>
 #include <sys/stat.h>
 #include <sys/types.h>
 #include <windows.h>
#else  // (defined _WIN32) || (defined _WIN64)
 #include <fcntl.h>
 #include <sys/mman.h>
 #include <sys/stat.h>
 #include <sys/types.h>
 #include <unistd.h>
#endif  // (defined _WIN32) || (defined _WIN64)

#include <cerrno>
#include <limits>
#include <stdexcept>

#include "marisa/base.h"
#include "marisa/system/file.h"

namespace marisa::system {

#if (defined _WIN32) || (defined _WIN64)
 #ifdef __MSVCRT_VERSION__
  #if __MSVCRT_VERSION__ >= 0x0601
   #define MARISA_HAS_STAT64
  #endif  // __MSVCRT_VERSION__ >= 0x0601
 #endif   // __MSVCRT_VERSION__
Mapper::Mapper(const char *filename, int flags) {
  file_ = ::CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, nullptr,
                        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
  MARISA_THROW_SYSTEM_ERROR_IF(file_ == INVALID_HANDLE_VALUE, ::GetLastError(),
                               std::system_category(), "CreateFileA");

  DWORD size_high, size_low;
  size_low = ::GetFileSize(file_, &size_high);
  MARISA_THROW_SYSTEM_ERROR_IF(size_low == INVALID_FILE_SIZE, ::GetLastError(),
                               std::system_category(), "GetFileSize");
  size_ = (std::size_t{size_high} << 32) | size_low;

  map_ = ::CreateFileMapping(file_, nullptr, PAGE_READONLY, 0, 0, nullptr);
  MARISA_THROW_SYSTEM_ERROR_IF(map_ == nullptr, ::GetLastError(),
                               std::system_category(), "CreateFileMapping");

  data_ = ::MapViewOfFile(map_, FILE_MAP_READ, 0, 0, 0);
  MARISA_THROW_SYSTEM_ERROR_IF(data_ == nullptr, ::GetLastError(),
                               std::system_category(), "MapViewOfFile");

  if (flags & MARISA_MAP_POPULATE) {
    WIN32_MEMORY_RANGE_ENTRY range_entry;
    range_entry.VirtualAddress = data_;
    range_entry.NumberOfBytes = size_;
    ::PrefetchVirtualMemory(GetCurrentProcess(), 1, &range_entry, 0);
  }
}
#else  // (defined _WIN32) || (defined _WIN64)
Mapper::Mapper(const char *filename, int flags) {
  fd_ = ::open(filename, O_RDONLY);
  MARISA_THROW_SYSTEM_ERROR_IF(fd_ == -1, errno, std::generic_category(),
                               "open");

  struct stat st;
  MARISA_THROW_SYSTEM_ERROR_IF(::fstat(fd_, &st) != 0, errno,
                               std::generic_category(), "fstat");
  MARISA_THROW_IF(static_cast<uint64_t>(st.st_size) > SIZE_MAX,
                  std::runtime_error);
  size_ = static_cast<std::size_t>(st.st_size);

  int map_flags = MAP_SHARED;
  if ((flags & MARISA_MAP_POPULATE) != 0) {
 #if defined(MAP_POPULATE)
    // `MAP_POPULATE` is Linux-specific.
    map_flags |= MAP_POPULATE;
 #elif defined(MAP_PREFAULT_READ)
    // `MAP_PREFAULT_READ` is FreeBSD-specific.
    map_flags |= MAP_PREFAULT_READ;
 #endif
  }

  data_ = ::mmap(nullptr, size_, PROT_READ, map_flags, fd_, 0);
  MARISA_THROW_SYSTEM_ERROR_IF(data_ == MAP_FAILED, errno,
                               std::generic_category(), "mmap");
  printf("map %p\n", data_);
}
#endif  // (defined _WIN32) || (defined _WIN64)

#if (defined _WIN32) || (defined _WIN64)
Mapper::~Mapper() {
  ::UnmapViewOfFile(data_);
  ::CloseHandle(map_);
  ::CloseHandle(file_);
}
#else  // (defined _WIN32) || (defined _WIN64)
Mapper::~Mapper() {
  printf("unmap %p\n", data_);
  ::munmap(data_, size_);
  ::close(fd_);
}
#endif

const void *Mapper::data() const {
  return data_;
}

size_t Mapper::size() const {
  return size_;
}

Reader::Reader(int fd) : fd_(fd) {}

void Reader::read(void *buf, size_t n) {
  while (n != 0) {
#ifdef _WIN32
    constexpr std::size_t ChunkSize = std::numeric_limits<int>::max();
    const unsigned int count = (n < ChunkSize) ? n : ChunkSize;
    const int n_read = ::_read(fd_, buf, count);
    MARISA_THROW_SYSTEM_ERROR_IF(n_read <= 0, errno, std::generic_category(),
                                 "_read");
#else   // _WIN32
    constexpr std::size_t ChunkSize = std::numeric_limits< ::ssize_t>::max();
    const ::size_t count = (n < ChunkSize) ? n : ChunkSize;
    const ::ssize_t n_read = ::read(fd_, buf, count);
    MARISA_THROW_SYSTEM_ERROR_IF(n_read <= 0, errno, std::generic_category(),
                                 "read");
#endif  // _WIN32
    buf = static_cast<char *>(buf) + n_read;
    n -= static_cast<std::size_t>(n_read);
  }
}

void Reader::seek(size_t n) {
  if (n == 0) {
    return;
  }
  if (n <= 16) {
    char buf[16];
    read(buf, n);
  } else {
    char buf[1024];
    while (n != 0) {
      const std::size_t count = (n < sizeof(buf)) ? n : sizeof(buf);
      read(buf, count);
      n -= count;
    }
  }
}

Writer::Writer(int fd) : fd_(fd) {}

void Writer::write(const void *buf, size_t n) {
  while (n != 0) {
#ifdef _WIN32
    constexpr std::size_t ChunkSize = std::numeric_limits<int>::max();
    const unsigned int count = (n < ChunkSize) ? n : ChunkSize;
    const int n_written = ::_write(fd_, buf, count);
    MARISA_THROW_SYSTEM_ERROR_IF(n_written <= 0, errno, std::generic_category(),
                                 "_write");
#else   // _WIN32
    constexpr std::size_t ChunkSize = std::numeric_limits< ::ssize_t>::max();
    const ::size_t count = (n < ChunkSize) ? n : ChunkSize;
    const ::ssize_t n_written = ::write(fd_, buf, count);
    MARISA_THROW_SYSTEM_ERROR_IF(n_written <= 0, errno, std::generic_category(),
                                 "write");
#endif  // _WIN32
    buf = static_cast<const char *>(buf) + n_written;
    n -= static_cast<std::size_t>(n_written);
  }
}

void Writer::seek(size_t n) {
  if (n == 0) {
    return;
  }
  if (n <= 16) {
    const char buf[16] = {};
    write(buf, n);
  } else {
    const char buf[1024] = {};
    while (n != 0) {
      const std::size_t count = (n < sizeof(buf)) ? n : sizeof(buf);
      write(buf, count);
      n -= count;
    }
  }
}

}  // namespace marisa::system