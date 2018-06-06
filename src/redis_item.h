#ifndef __REDIS_ITEM_H__
#define __REDIS_ITEM_H__

#include "buffer.h"

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
  int type_;
  int state_;

  RedisItem(int type)
    : type_(type),
      state_(NONE_ITEM_STATE) {
  }

  virtual ~RedisItem() {}

  int len_;
  BufferPos start_;
  BufferPos end_;
};

struct RedisSimpleStringItem : public RedisItem {
public:
  RedisSimpleStringItem()
    : RedisItem(REDIS_SIMPLE_STRING),
      len_(0) {}

  virtual ~RedisSimpleStringItem() {}

  int len_;
};

#endif //  __REDIS_ITEM_H__
