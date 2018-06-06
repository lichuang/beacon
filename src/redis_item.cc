#include <stdio.h>
#include <ctype.h>
#include "redis_item.h"
#include "redis_command.h"
#include "redis_parser.h"
#include "redis_session.h"

RedisItem* newRedisItem(int type, RedisCommand *cmd, RedisSession *session) {
  switch (type) {
  case REDIS_ARRAY:
    return new RedisArrayItem(cmd, session);
  case REDIS_SIMPLE_STRING:
    return new RedisSimpleStringItem(cmd, session);
  default:
    break;
  }

  return NULL;
}

bool RedisArrayItem::Parse() {
  char c;
  int i, type;
  RedisItem *item;
  Buffer *buf;

  state_ = PARSE_ARRAY_BEGIN;

  while (session_->hasUnprocessedQueryData()) {
    buf = session_->QueryBuffer();
    switch (state_) {
    case PARSE_ARRAY_BEGIN:
      state_ = PARSE_ARRAY_LENGTH;
      sign_ = 1;
      item_num_ = 0;
      break;
    case PARSE_ARRAY_LENGTH:
      c = *(buf->NextRead());
      switch (c) {
      case '-':
        sign_ = -1;
        break;
      case '\r':
        state_ = PARSE_ARRAY_ITEM;
        // pass \r\n
        buf->AdvanceRead(1);
        break;
      default:
        if (!isdigit(c)) {
          return false;
        }
        item_num_ = item_num_ * 10 + c - '0';
        break;
      }
      break;
    case PARSE_ARRAY_ITEM:
      if (sign_ < 0) { // NULL array
        state_ = PARSE_ARRAY_END;
        item_num_ = 0;
      } else {
        for (i = 0; i < item_num_; ++i) {
          if (!ParseType(c, &type)) {
            return false;
          }
          // pass type
          buf->AdvanceRead(1);
          item = newRedisItem(type, cmd_, session_);
          array_.push_back(item);
          if (!item->Parse()) {
            return false;
          }
        }
      }
      break;
    }
    buf->AdvanceRead(1);
  }
  return true;
}

bool RedisSimpleStringItem::Parse() {
  char c;
  Buffer *buf;

  state_ = PARSE_SIMPLE_STRING_BEGIN;

  while (session_->hasUnprocessedQueryData()) {
    buf = session_->QueryBuffer();
    switch (state_) {
    case PARSE_SIMPLE_STRING_BEGIN:
      state_ = PARSE_SIMPLE_STRING_LENGTH;
      start_.buffer_ = buf;
      start_.pos_ = buf->ReadPos();
      break;
    case PARSE_SIMPLE_STRING_LENGTH:
      c = *(buf->NextRead());
      if (c == '\r') {
        state_ = PARSE_SIMPLE_STRING_END;
      } else {
        len_ += 1;
        buf->AdvanceRead(1);
      }

      break;
    case PARSE_SIMPLE_STRING_END:
      end_.buffer_ = buf;
      end_.pos_ = buf->ReadPos();
      break;
    }
  }

  return true;
}
