#include "buffer.h"

Buffer::Buffer(int size)
  : write_pos_(0),
    read_pos_(0),
    init_size_(size) {
  buf_.reserve(size);
}

Buffer::~Buffer() {
}

char* Buffer::Start() {
  return &buf_[0];
}

char* Buffer::NextWrite() {
  return &(buf_[write_pos_]);
}

char* Buffer::NextRead() {
  return &(buf_[read_pos_]);
}

int Buffer::WritableLength() {
  return buf_.capacity() - write_pos_;
}

int Buffer::ReadableLength() {
  return write_pos_ - read_pos_;
}

void Buffer::Reserve(int addlen) {
  if (buf_.capacity() - write_pos_ >= addlen) {
    return;
  }
  buf_.reserve(addlen + write_pos_);
}

void Buffer::Reset() {
  vector<char>(buf_).swap(buf_);
  buf_.reserve(init_size_);
  write_pos_ = read_pos_ = 0;
}
