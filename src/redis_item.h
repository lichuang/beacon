#ifndef __REDIS_ITEM_H__
#define __REDIS_ITEM_H__

#include "buffer.h"

class RedisCommand;
class RedisSession;

// redis data type
enum {
  REDIS_ARRAY,
  REDIS_STRING,
  REDIS_SIMPLE_STRING,
};

// item statemachine type
enum {
  NONE_ITEM_STATE,

  // for simple string
  PARSE_SIMPLE_STRING_BEGIN,
  PARSE_SIMPLE_STRING_LENGTH,
  PARSE_SIMPLE_STRING_END,
};

struct RedisItem {
public:

  RedisItem(int type, RedisCommand *cmd, RedisSession *session)
    : type_(type),
      state_(NONE_ITEM_STATE),
      len_(0),
      cmd_(cmd),
      session_(session) {
  }

  virtual ~RedisItem() {}

  virtual bool Parse() = 0;

  int type_;
  int state_;

  int len_;
  BufferPos start_;
  BufferPos end_;

  RedisCommand *cmd_;
  RedisSession *session_;
};

struct RedisSimpleStringItem : public RedisItem {
public:
  RedisSimpleStringItem(RedisCommand *cmd, RedisSession *session)
    : RedisItem(REDIS_SIMPLE_STRING, cmd, session), str_len_(0)
  {}

  virtual ~RedisSimpleStringItem() {}

  virtual bool Parse();

  int str_len_;
};

#endif //  __REDIS_ITEM_H__
