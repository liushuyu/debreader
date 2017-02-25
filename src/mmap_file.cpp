#include "mmap_file.h"

fakeFile::fakeFile(const std::string filename) {
  this->open(filename.c_str());
}

fakeFile::fakeFile(const char* filename) {
  this->open(filename);
}

size_t fakeFile::tell() {
  return this->m_offset;
}

char* fakeFile::getMem() {
  return this->memFile;
}

void fakeFile::ignore(const size_t pos) {
  if ((this->m_offset + pos) > this->m_fsize) {
    this->m_offset = m_fsize;
    return;
  }
  this->m_offset += pos;
  this->memFile += pos;
}

void fakeFile::seek(const size_t pos) {
  if (pos > this->m_fsize) {
    this->m_offset = m_fsize;
    return;
  }
//  this->memFile = pos;
}

size_t fakeFile::size() {
  return this->m_fsize;
}

fakeFile::~fakeFile() {
    munmap(this->memFile, this->m_fsize);
}

size_t fakeFile::read(char *dst, size_t len) {
  if ((this->m_offset + len) > this->m_fsize) {
    return -1;
  }
  memmove(dst, this->memFile, len);
  this->m_offset += len;
  this->memFile += len;
  return len;
}

char* fakeFile::read(size_t len) {
  char* buf = (char*)malloc(len);
  memset(buf, 0, len);
  this->read(buf, len);
  return buf;
}

void fakeFile::open(const char *fn) {
  struct stat sb;
  int fd = ::open(fn, O_RDONLY);
  if (fd == -1) {
    return;
  }
  if (fstat(fd, &sb) == -1) {
    return;
  }
  this->memFile = (char *)mmap(0, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
  if (this->memFile == MAP_FAILED) {
    return;
  }
  this->m_fsize = sb.st_size;
  return;
}
