#ifndef __REDIS_ITEM_H__
#define __REDIS_ITEM_H__

#include <vector>
#include "buffer.h"

using namespace std;

class RedisCommand;
class RedisInfo;

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
  RedisItem(int type, RedisCommand *cmd, RedisInfo *info)
    : type_(type),
      state_(NONE_ITEM_STATE),
      len_(0),
      cmd_(cmd),
      info_(info) {
  }

  virtual ~RedisItem() {}

  virtual bool Parse() = 0;

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
  RedisInfo *info_;
};

struct RedisArrayItem : public RedisItem {
public:
  RedisArrayItem(RedisCommand *cmd, RedisInfo *info)
    : RedisItem(REDIS_ARRAY, cmd, info)
  {}

  virtual ~RedisArrayItem() {}
  virtual bool Parse();

  int sign_;
  int item_num_;
  vector<RedisItem*> array_;
};

struct RedisStringItem : public RedisItem {
public:
  RedisStringItem(RedisCommand *cmd, RedisInfo *info)
    : RedisItem(REDIS_STRING, cmd, info), str_len_(0)
  {}

  virtual ~RedisStringItem() {}

  virtual bool Parse();

  int str_len_;
};

struct RedisBulkItem : public RedisItem {
public:
  RedisBulkItem(RedisCommand *cmd, RedisInfo *info)
    : RedisItem(REDIS_BULK, cmd, info)
  {}

  virtual ~RedisBulkItem() {}

  virtual bool Parse();

  int len_;
};

struct RedisIntItem : public RedisItem {
public:
  RedisIntItem(RedisCommand *cmd, RedisInfo *info)
    : RedisItem(REDIS_INT, cmd, info)
  {}

  virtual ~RedisIntItem() {}

  virtual bool Parse();
};

extern RedisItem* newRedisItem(int type, RedisCommand *cmd, RedisInfo *info);

#endif //  __REDIS_ITEM_H__
