#include "deb_info.h"
#include <archive.h>
#include <archive_entry.h>

using namespace std;

DebReader::DebReader(const std::string filename) {
  DebReader(filename.c_str());
}

DebReader::DebReader(const char* filename) {
  this->debfile = new fakeFile(filename);
}

DebReader::~DebReader() {
  if (this->debfile) {
    delete this->debfile;
    this->debfile = nullptr;
  }
    if (this->control_buffer) {
        free(this->control_buffer);
    }
}

int DebReader::read_header(deb_header *header) {
  READ_INTO(this->debfile, header, magic);
  if (memcmp(header->magic, "!<arch>", 7) != 0) {
    return 1;
  }
  read_entry(&header->header_entry);
  READ_INTO(this->debfile, header, version);
  return 0;
}

int DebReader::read_entry(deb_entry *deb_entry) {
  READ_INTO(this->debfile, deb_entry, identifier);
  READ_INTO(this->debfile, deb_entry, timestamp);
  READ_INTO(this->debfile, deb_entry, owner);
  READ_INTO(this->debfile, deb_entry, gid);
  READ_INTO(this->debfile, deb_entry, perm);
  READ_INTO(this->debfile, deb_entry, filesize);
  READ_INTO(this->debfile, deb_entry, end);
  if (memcmp(deb_entry->end, "`\x0a", 2) != 0) {
    return -1;
  }
  return 0;
}

deb_entry DebReader::read_entry() {
  deb_entry entry;
  read_entry(&entry);
  return entry;
}

void DebReader::iterate_entries() {
  while (this->debfile->tell() < this->debfile->size()) {
    deb_entry entry = read_entry();
    size_t filesize = atol(entry.filesize);
    if (memcmp(entry.identifier, "control.tar", 11) == 0) {
      read_control(filesize);
    } else if (memcmp(entry.identifier, "data.tar", 8) == 0) {
      list_files(filesize);
    }
    this->debfile->ignore(filesize);
  }
  return;
}

void DebReader::list_files(size_t len) {
  struct archive *a = archive_read_new();
  struct archive_entry *ark_entry;
  archive_read_support_filter_all(a);
  archive_read_support_format_tar(a);
  int r = archive_read_open_memory(a, this->debfile->getMem(), len);
  if (r != ARCHIVE_OK) {
    return;
  }
  while (archive_read_next_header(a, &ark_entry) == ARCHIVE_OK) {
    this->pkg_content.push_back(archive_entry_pathname(ark_entry));
  }
  archive_read_close(a);
  archive_read_free(a);
  return;
}

void DebReader::read_control(size_t len) {
  struct archive *a = archive_read_new();
  struct archive_entry *ark_entry;
  archive_read_support_filter_all(a);
  archive_read_support_format_tar(a);
  int r = archive_read_open_memory(a, this->debfile->getMem(), len);
  if (r != ARCHIVE_OK) {
    return;
  }
  while (archive_read_next_header(a, &ark_entry) == ARCHIVE_OK) {
    if (memcmp(archive_entry_pathname(ark_entry), "./control", 9) == 0) {
      size_t entry_size = archive_entry_size(ark_entry);
      char* control_buffer = (char*)malloc(entry_size);
      archive_read_data(a, this->control_buffer, entry_size);
      break;
    }
  }
  archive_read_close(a);
  archive_read_free(a);
  return;
}

int DebReader::read() {
  if (this->debfile->size() < this->deb_pkg_header_size) {
    return -1;
  }
  this->header = (deb_header*)malloc(sizeof(deb_header));
  if (this->read_header(header)) {return -1;}
  this->iterate_entries();
  return 0;
}

char* DebReader::getControlFile() {
  return this->control_buffer;
}

std::vector<std::string> DebReader::getFileList() {
  return this->pkg_content;
}
