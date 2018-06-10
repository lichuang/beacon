#ifndef __REDIS_COMMAND_H__
#define __REDIS_COMMAND_H__

#include "buffer.h"

enum {
  REDIS_COMMAND_READY,
  REDIS_COMMAND_ERROR,
  REDIS_COMMAND_NONE,
};

class RedisCommand {
public:
  RedisCommand();
  ~RedisCommand();

  void Init(Buffer *buf, int start);
  void End(Buffer *buf, int end);
  void ReadyWrite();

  BufferPos* Current() {
    return &current_;
  }

  void FreeBuffers();

  BufferPos*  NextBufferPos();

  void SetMode(int mode) {
    mode_ = mode;
  }
  void SetStatus(int status) {
    status_ = status;
  }

  int GetStatus() {
    return status_;
  }

  bool GetReady() {
    return status_ == REDIS_COMMAND_READY;
  }

  bool Error() {
    return status_ == REDIS_COMMAND_ERROR;
  }

private:
  BufferPos start_, end_;
  BufferPos current_;
  int status_;
  int mode_;
};

#endif // __REDIS_COMMAND_H__
