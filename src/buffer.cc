#include <string.h>
#include "buffer.h"

Buffer::Buffer(int size)
  : buf_(new char[size]),
    write_pos_(0),
    read_pos_(0),
    size_(size),
    ref_cnt_(0),
    next_(NULL) {
}

Buffer::~Buffer() {
}

bool Buffer::Write(const char *p, int len) {
  if (len > WritableLength()) {
    return false;
  }
  memcpy(NextWrite(), p, len);
  AdvanceWrite(len);
  return true;
}

char* Buffer::Start() {
  return buf_;
}

char* Buffer::NextWrite() {
  return &(buf_[write_pos_]);
}

char* Buffer::NextRead() {
  return &(buf_[read_pos_]);
}

int Buffer::WritableLength() {
  return size_ - write_pos_;
}

int Buffer::ReadableLength() {
  return write_pos_ - read_pos_;
}

void Buffer::Reset() {
  write_pos_ = read_pos_ = 0;
}
