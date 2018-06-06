#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <vector>

using namespace std;

class Buffer {
public:
  Buffer(int size);
  ~Buffer();

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

  int WritableLength();
  int ReadableLength();

  void Reset();

  void  Reserve(int addlen);

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

private:
  vector<char> buf_;
  int write_pos_;
  int read_pos_;
  int init_size_;
  int ref_cnt_;
};

struct BufferPos {
  Buffer *buffer_;
  int     pos_;
};

#endif // __BUFFER_H__
