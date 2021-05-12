#ifndef MMAP_FILE_H
#define MMAP_FILE_H
#include <cstring>
#include <fcntl.h>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

class fakeFile {
public:
  fakeFile(const std::string filename);
  fakeFile(const char *filename);
  size_t read(char *dst, size_t len);
  char *read(size_t len);
  void ignore(const size_t pos = 1);
  void seek(const size_t pos);
  size_t tell();
  virtual ~fakeFile();
  size_t size();
  char *getMem();

private:
  size_t m_offset = 0;
  size_t m_fsize = 0;
  char *memFile;
  void open(const char *fn);
};

#endif
