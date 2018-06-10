#ifndef __REDIS_ITEM_H__
#define __REDIS_ITEM_H__

#include <vector>
#include "buffer.h"

using namespace std;

class RedisCommand;
class Buffer;

// redis data type
enum {
  REDIS_NONE_TYPE,
  REDIS_ARRAY,
  REDIS_BULK,
  REDIS_STRING,
  REDIS_INT,
};

// item statemachine type
enum {
  NONE_ITEM_STATE,

  // for array
  PARSE_ARRAY_BEGIN,
  PARSE_ARRAY_LENGTH,
  PARSE_ARRAY_ITEM,
  PARSE_ARRAY_END,

  // for string
  PARSE_STRING_BEGIN,
  PARSE_STRING_LENGTH,
  PARSE_STRING_END,

  // for bulk
  PARSE_BULK_BEGIN,
  PARSE_BULK_LENGTH,
  PARSE_BULK_CONTENT,
  PARSE_BULK_END,

  // for int
  PARSE_INT_BEGIN,
  PARSE_INT_NUMBER,
  PARSE_INT_END,
};

struct RedisItem {
public:
  RedisItem(int type)
    : type_(type),
      state_(NONE_ITEM_STATE),
      len_(0) {
  }

  virtual ~RedisItem() {}

  virtual bool Parse(RedisCommand *, Buffer *) = 0;

  void markStartPos(Buffer* buf) {
    start_.buffer_ = buf;
    start_.pos_ = buf->ReadPos();
  }

  void markEndPos(Buffer* buf) {
    end_.buffer_ = buf;
    end_.pos_ = buf->ReadPos();
  }

  int type_;
  int state_;

  int len_;
  BufferPos start_, end_;

  RedisCommand *cmd_;
  Buffer *buffer_;
};

struct RedisArrayItem : public RedisItem {
public:
  RedisArrayItem()
    : RedisItem(REDIS_ARRAY)
  {}

  virtual ~RedisArrayItem() {}
  virtual bool Parse(RedisCommand *, Buffer *);

  int sign_;
  int item_num_;
  vector<RedisItem*> array_;
};

struct RedisStringItem : public RedisItem {
public:
  RedisStringItem()
    : RedisItem(REDIS_STRING), str_len_(0)
  {}

  virtual ~RedisStringItem() {}

  virtual bool Parse(RedisCommand *, Buffer *);

  int str_len_;
};

struct RedisBulkItem : public RedisItem {
public:
  RedisBulkItem()
    : RedisItem(REDIS_BULK)
  {}

  virtual ~RedisBulkItem() {}

  virtual bool Parse(RedisCommand *, Buffer *);

  int len_;
};

struct RedisIntItem : public RedisItem {
public:
  RedisIntItem()
    : RedisItem(REDIS_INT)
  {}

  virtual ~RedisIntItem() {}

  virtual bool Parse(RedisCommand *, Buffer *);
};

extern RedisItem* newRedisItem(int type);

#endif //  __REDIS_ITEM_H__
