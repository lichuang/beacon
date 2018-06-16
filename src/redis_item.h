#ifndef __REDIS_ITEM_H__
#define __REDIS_ITEM_H__

#include <vector>
#include <string>
#include "buffer.h"

using namespace std;

class Buffer;

// redis data type
enum RedisItemType {
  REDIS_NONE_TYPE,
  REDIS_ARRAY,
  REDIS_BULK,
  REDIS_STRING,
  REDIS_INT,
};

// item statemachine type
enum RedisItemStateType {
  NONE_ITEM_STATE,

  // for array
  PARSE_ARRAY_BEGIN,
  PARSE_ARRAY_LENGTH,
  PARSE_ARRAY_LENGTH_N,
  PARSE_ARRAY_ITEM_TYPE,
  PARSE_ARRAY_ITEM,
  PARSE_ARRAY_END,

  // for string
  PARSE_STRING_BEGIN,
  PARSE_STRING_CONTENT,
  PARSE_STRING_R,
  PARSE_STRING_N,
  PARSE_STRING_END,

  // for bulk
  PARSE_BULK_BEGIN,
  PARSE_BULK_LENGTH_R,
  PARSE_BULK_LENGTH_N,
  PARSE_BULK_CONTENT,
  PARSE_BULK_CONTENT_R,
  PARSE_BULK_CONTENT_N,
  PARSE_BULK_END,

  // for int
  PARSE_INT_BEGIN,
  PARSE_INT_NUMBER,
  PARSE_INT_END,
};

struct RedisItemPos {
  BufferPos start_, end_;

  RedisItemPos() {
  }
};

struct RedisItem {
public:
  RedisItem(RedisItemType type, RedisItemStateType state)
    : type_(type),
      state_(state),
      ready_(false) {
  }

  virtual ~RedisItem() {}

  virtual bool Parse(Buffer *) = 0;

  bool Ready() {
    return ready_;
  }

  void markItemStartPos(Buffer* buf) {
    item_pos_.start_.buffer_ = buf;
    item_pos_.start_.pos_ = buf->ReadPos();
    ready_ = false;
  }

  void markItemEndPos(Buffer* buf) {
    item_pos_.end_.buffer_ = buf;
    item_pos_.end_.pos_ = buf->ReadPos();
    ready_ = true;
  }

  void markItemValueStartPos(Buffer *buf) {
    value_pos_.start_.buffer_ = buf;
    value_pos_.start_.pos_ = buf->ReadPos();
  }

  void markItemValueEndPos(Buffer *buf) {
    value_pos_.end_.buffer_ = buf;
    value_pos_.end_.pos_ = buf->ReadPos();
  }

  // return item start buffer/pos
  Buffer* GetItemStartBuffer() {
    return item_pos_.start_.buffer_;
  }
  int GetItemStartPos() {
    return item_pos_.start_.pos_;
  }

  // return item end buffer/pos
  Buffer* GetItemEndBuffer() {
    return item_pos_.end_.buffer_;
  }
  int GetItemEndPos() {
    return item_pos_.end_.pos_;
  }

  // return value start buffer/pos
  Buffer* GetValueStartBuffer() {
    return value_pos_.start_.buffer_;
  }
  int GetValueStartPos() {
    return value_pos_.start_.pos_;
  }

  // return value end buffer/pos
  Buffer* GetValueEndBuffer() {
    return value_pos_.end_.buffer_;
  }
  int GetValueEndPos() {
    return value_pos_.end_.pos_;
  }

  // return start pos of the item and length of value
  void GetValue(string *val);

  RedisItemType GetType() {
    return type_;
  }
protected:
  RedisItemType type_;
  int state_;
  bool ready_;

  RedisItemPos value_pos_, item_pos_;
};

struct RedisArrayItem : public RedisItem {
public:
  RedisArrayItem()
    : RedisItem(REDIS_ARRAY, PARSE_ARRAY_BEGIN),
      sign_(1),item_num_(0),current_(NULL)
  {}

  virtual ~RedisArrayItem();
  virtual bool Parse(Buffer *);

  int sign_;
  int item_num_;
  RedisItem* current_;
  vector<RedisItem*> array_;
};

struct RedisStringItem : public RedisItem {
public:
  RedisStringItem()
    : RedisItem(REDIS_STRING, PARSE_STRING_BEGIN), str_len_(0)
  {}

  virtual ~RedisStringItem() {}

  virtual bool Parse(Buffer *);

  int str_len_;
};

struct RedisBulkItem : public RedisItem {
public:
  RedisBulkItem()
    : RedisItem(REDIS_BULK, PARSE_BULK_BEGIN)
  {}

  virtual ~RedisBulkItem() {}

  virtual bool Parse(Buffer *);

  int readed_len_;
  int len_;
};

struct RedisIntItem : public RedisItem {
public:
  RedisIntItem()
    : RedisItem(REDIS_INT, PARSE_INT_BEGIN), marked_int_(false)
  {}

  virtual ~RedisIntItem() {}

  virtual bool Parse(Buffer *);

private:
  bool marked_int_;
};

extern RedisItem* newRedisItem(int type);

#endif //  __REDIS_ITEM_H__
