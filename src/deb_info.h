#ifndef DEB_INFO_H
#define DEB_INFO_H
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include "mmap_file.h"

#define READ_INTO(STM, STRUCT, DST)  STM->read((char*)&STRUCT->DST, sizeof(STRUCT->DST));
typedef struct {
  char identifier[16];
  char timestamp[12];
  char owner[6];
  char gid[6];
  char perm[8];
  char filesize[10];
  char end[2]; // `\n
} deb_entry;

typedef struct {
  char magic[8];
  deb_entry header_entry;
  char version[4];
} deb_header;

class DebReader {
private:
  const unsigned int deb_pkg_header_size = 71;
  int read_header(deb_header *header);
  int read_entry(deb_entry *deb_entry);
  deb_entry read_entry();
  void iterate_entries();
  void list_files(size_t len);
  void read_control(size_t len);
  void pack_vector();
  void fuzzy_ignore(size_t skip_size);
  std::string control_buffer;
  std::string err;
  bool listFiles;
  int read_proxy();
  std::vector<std::string> pkg_content;
  fakeFile *debfile = nullptr;
  deb_header *header = nullptr;

public:
  DebReader (const std::string filename);
  DebReader (const char* filename);
  int read();
  void cleanup();
  std::string getControlFile();
  std::vector<std::string> getFileList();
  virtual ~DebReader ();
};

#endif
