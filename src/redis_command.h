#ifndef __REDIS_COMMAND_H__
#define __REDIS_COMMAND_H__

#include "buffer.h"

class RedisCommand {
public:
  RedisCommand();
  ~RedisCommand();

  void Init(Buffer *buf, int start);
  void End(Buffer *buf, int end);

private:
  BufferPos start_;
  BufferPos end_;
};

#endif // __REDIS_COMMAND_H__
