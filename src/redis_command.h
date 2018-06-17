#ifndef __REDIS_COMMAND_H__
#define __REDIS_COMMAND_H__

#include <string>
#include "buffer.h"

using namespace std;

// redis command statemachine
enum {
  // recv client request
  REDIS_COMMAND_RECV,
  REDIS_COMMAND_RECV_DONE,
  // forward client request to redis server
  REDIS_COMMAND_FORWARD,
  // recv response from redis server
  REDIS_COMMAND_RECV_RESPONSE,
  REDIS_COMMAND_RECV_RESPONSE_DONE,
  // response to client
  REDIS_COMMAND_RESPONSE,
};

class RedisItem;

class RedisCommand {
public:
  RedisCommand();
  ~RedisCommand();

  void Init(Buffer *buf, int start);
  void End(Buffer *buf, int end, RedisItem*);
  void ReadyWrite();
  bool Parse();

  bool Ready() {
    return state_ == REDIS_COMMAND_RECV_DONE ||
           state_ == REDIS_COMMAND_RECV_RESPONSE_DONE;
  }

  RedisItem* item() {
    return item_;
  }

  BufferPos* Current() {
    return &current_;
  }

  void FreeBuffers();

  BufferPos*  NextBufferPos();

  bool NeedRoute() {
    return need_route_;
  }

  void MarkError(const string& err);
private:
  // for request
  BufferPos recv_start_, recv_end_;
  // for response
  BufferPos send_start_, send_end_;

  BufferPos *start_, *end_;

  BufferPos current_;
  int state_;
  RedisItem *item_;
  bool need_route_;
};

#endif // __REDIS_COMMAND_H__
