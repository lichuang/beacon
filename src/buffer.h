#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <stdio.h>
#include <vector>

using namespace std;

class Buffer {
public:
  Buffer(int size);
  ~Buffer();

  bool Write(const char*, int len);
  // return next writeable char
  char* NextWrite();

  char* NextRead();

  // return the start pos of the buffer
  char* Start();

  // advance write pos
  void  AdvanceWrite(int size) {
    write_pos_ += size;
  }

  void  AdvanceRead(int size) {
    read_pos_ += size;
  }

  void SetReadPos(int pos) {
    read_pos_ = pos;
  }
  void SetWritePos(int pos) {
    write_pos_ = pos;
  }

  int WritableLength();
  int ReadableLength();

  void Reset();

  void IncrCnt() {
    ++ref_cnt_;
  }
  void DescCnt() {
    --ref_cnt_;
  }
  int  RefCnt() {
    return ref_cnt_;
  }

  int ReadPos() {
    return read_pos_;
  }
  int WritePos() {
    return write_pos_;
  }

  bool Full() {
    return write_pos_ == size_;
  }

  bool hasUnprocessedData() {
    return read_pos_ < write_pos_;
  }

  Buffer* NextBuffer() {
    return next_;
  }
  void SetNextBuffer(Buffer *next) {
    next_ = next;
  }
private:
  char* buf_;
  int write_pos_;
  int read_pos_;
  int size_;
  int ref_cnt_;
  Buffer* next_;
};

struct BufferPos {
  Buffer *buffer_;
  int     pos_;
  int     write_pos_;

  BufferPos()
    : buffer_(NULL), pos_(0), write_pos_(0) {}

  bool Done() {
    return pos_ == write_pos_;
  }
};

#endif // __BUFFER_H__
