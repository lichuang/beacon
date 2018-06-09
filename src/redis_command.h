#ifndef __REDIS_COMMAND_H__
#define __REDIS_COMMAND_H__

#include "buffer.h"

class RedisCommand {
public:
  RedisCommand();
  ~RedisCommand();

  void Init(Buffer *buf, int start);
  void End(Buffer *buf, int end);
  BufferPos* Current() {
    return &current_;
  }

  BufferPos*  NextBufferPos();

private:
  BufferPos start_, end_;
  BufferPos current_;
};

#endif // __REDIS_COMMAND_H__
