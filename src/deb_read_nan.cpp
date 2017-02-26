#include "deb_info_nan.h"
#include <archive.h>
#include <archive_entry.h>

using namespace std;

DebReader::DebReader(Nan::Callback *callback, const std::string filename, const bool listFiles)
    : Nan::AsyncWorker(callback), listFiles(listFiles), callback(callback) {
  this->debfile = new fakeFile(filename.c_str());
  this->header = (deb_header *)malloc(sizeof(deb_header));
}

DebReader::~DebReader() { this->cleanup(); }

int DebReader::read_header(deb_header *header) {
  READ_INTO(this->debfile, header, magic);
  if (memcmp(header->magic, "!<arch>", 7) != 0) {
    this->cleanup();
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
    this->err.assign("Bad entry ending, file corrupted? Got: " + std::string(deb_entry->end));
    return 1;
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
    // We only compare first several chars of the identifier, since it's
    // legal to have difference compressions
    if (memcmp(entry.identifier, "control.tar", 11) == 0) {
      read_control(filesize);
      if (!this->listFiles) {return;}
    } else if (memcmp(entry.identifier, "data.tar", 8) == 0) {
      list_files(filesize);
    }
    this->fuzzy_ignore(filesize);
  }
  return;
}

void DebReader::fuzzy_ignore(size_t skip_size) {
  if (this->debfile->tell() + skip_size + 10 > this->debfile->size()) {
    this->debfile->seek(this->debfile->size());
    return;
  }
  for (size_t i = 1; i < 20; i++) {
    int offset = (int)(i / 2) * (i % 2 ? 1 : -1);
    if (memcmp(this->debfile->getMem() + (offset + skip_size + 58), "`\x0a", 2) == 0) {
      this->debfile->ignore(skip_size + offset);
      return;
    }
  }
  this->err.assign("Bad entry ending, file corrupted?");
  return;
}

void DebReader::list_files(size_t len) {
  struct archive *a = archive_read_new();
  struct archive_entry *ark_entry;
  archive_read_support_filter_all(a);
  archive_read_support_format_tar(a);
  int r = archive_read_open_memory(a, this->debfile->getMem(), len);
  if (r != ARCHIVE_OK) {
    this->err.assign(archive_error_string(a));
    return;
  }
  while (archive_read_next_header(a, &ark_entry) == ARCHIVE_OK) {
    if (archive_entry_filetype(ark_entry) == AE_IFDIR) {continue;}  // Filter out folder entries
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
      char *control_buffer = (char *)malloc(entry_size);
      archive_read_data(a, control_buffer, entry_size);
      this->control_buffer.assign(control_buffer);
      free(control_buffer);
      break;
    }
  }
  archive_read_close(a);
  archive_read_free(a); // Remember to free the memory!
  return;
}

void DebReader::Execute() {
  if (this->debfile->size() < this->deb_pkg_header_size) {
    this->err.assign("File size less than standard header size!");
    return;
  }
  if (this->read_header(header)) {
    this->err.assign("File invaild!");
    return;
  }
  this->iterate_entries();
  return;
}

void DebReader::cleanup() {
  if (this->debfile != nullptr) {
    delete this->debfile;
    this->debfile = nullptr;
  }
  if (this->header) {
    free(this->header);
  }
}

void DebReader::HandleOKCallback() {
  Nan::HandleScope(); // Tell V8 GC we are going to introduce new objects to JS
                      // world
  v8::Local<v8::Value> cb_argv[2] = {Nan::Null(),
                                     Nan::Null()}; // Prepare callback args
  if (!this->err.empty()) {
    cb_argv[0] = Nan::Error(this->err.c_str());
    callback->Call(2, cb_argv);  // argc=2,argv[2]
    return;
  }
  // Convert std::vector into v8::Array (Array type in JS world)
  v8::Local<v8::Array> pkg_content_js =
      Nan::New<v8::Array>(this->pkg_content.size());
  for (size_t i = 0; i < pkg_content.size(); i++) {
    // Type in JS world: Array(String, String, ...)
    Nan::Set(pkg_content_js, i,
             Nan::New<v8::String>(pkg_content.at(i)).ToLocalChecked());
  }
  v8::Local<v8::Object> info =
      Nan::New<v8::Object>(); // Our information to be transferred back to JS
  Nan::Set(info, Nan::New("controlFile").ToLocalChecked(),
           Nan::New<v8::String>(this->control_buffer).ToLocalChecked());
  Nan::Set(info, Nan::New("fileList").ToLocalChecked(), pkg_content_js);
  cb_argv[1] = info;
  callback->Call(2, cb_argv);
}
