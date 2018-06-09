#include <stdio.h>
#include <ctype.h>
#include "const.h"
#include "redis_item.h"
#include "redis_command.h"
#include "redis_parser.h"
#include "redis_info.h"

static bool toNumber(char c, int *v) {
  if (!isdigit(c)) {
    return false;
  }
  *v = *v * 10 + c - '0';
  return true;
}

RedisItem* newRedisItem(int type, RedisCommand *cmd, RedisInfo *info) {
  switch (type) {
  case REDIS_ARRAY:
    return new RedisArrayItem(cmd, info);
  case REDIS_STRING:
    return new RedisStringItem(cmd, info);
  case REDIS_BULK:
    return new RedisBulkItem(cmd, info);
  case REDIS_INT:
    return new RedisIntItem(cmd, info);
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

  while (info_->hasUnprocessedQueryData()) {
    buf = info_->QueryBuffer();
    switch (state_) {
    case PARSE_ARRAY_BEGIN:
      if (*(buf->NextRead()) != kRedisArrayPrefix) {
        return false;
      }
      state_ = PARSE_ARRAY_LENGTH;
      sign_ = 1;
      item_num_ = 0;
      buf->AdvanceRead(1);
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
        if (*buf->NextRead() != '\n') {
          return false;
        }
        break;
      default:
        if (!toNumber(c, &item_num_)) {
          return false;
        }
        break;
      }
      buf->AdvanceRead(1);
      break;
    case PARSE_ARRAY_ITEM:
      if (sign_ < 0) { // NULL array
        state_ = PARSE_ARRAY_END;
        item_num_ = 0;
      } else {
        for (i = 0; i < item_num_; ++i) {
          c = *(buf->NextRead());
          if (!ParseType(c, &type)) {
            return false;
          }
          item = newRedisItem(type, cmd_, info_);
          array_.push_back(item);
          if (!item->Parse()) {
            return false;
          }
        }
        state_ = PARSE_ARRAY_END;
      }
      break;
    case PARSE_ARRAY_END:
      break;
    }
  }

  return state_ == PARSE_ARRAY_END;
}

bool RedisStringItem::Parse() {
  char c;
  Buffer *buf;

  state_ = PARSE_STRING_BEGIN;

  while (info_->hasUnprocessedQueryData()) {
    buf = info_->QueryBuffer();
    switch (state_) {
    case PARSE_STRING_BEGIN:
      if (*(buf->NextRead()) != kRedisStringPrefix) {
        return false;
      }
      state_ = PARSE_STRING_LENGTH;
      markStartPos(buf);
      buf->AdvanceRead(1);
      break;
    case PARSE_STRING_LENGTH:
      c = *(buf->NextRead());
      if (c == '\r') {
        state_ = PARSE_STRING_END;
      } else {
        len_ += 1;
      }
      buf->AdvanceRead(1);
      break;
    case PARSE_STRING_END:
      c = *(buf->NextRead());
      if (c != '\n') {
        state_ = PARSE_STRING_LENGTH;
      } else {
        markEndPos(buf);
        buf->AdvanceRead(1);
        return true;
      }
      break;
    }
  }

  return false;
}

bool RedisBulkItem::Parse() {
  char c;
  Buffer *buf;

  state_ = PARSE_BULK_BEGIN;

  while (info_->hasUnprocessedQueryData()) {
    buf = info_->QueryBuffer();
    switch (state_) {
    case PARSE_BULK_BEGIN:
      if (*(buf->NextRead()) != kRedisBulkPrefix) {
        return false;
      }
      state_ = PARSE_BULK_LENGTH;
      markStartPos(buf);
      len_ = 0; 
      buf->AdvanceRead(1);
      break;
    case PARSE_BULK_LENGTH:
      c = *(buf->NextRead());
      if (c == '\r') {
        state_ = PARSE_BULK_CONTENT;
        buf->AdvanceRead(1);
      } else if (!toNumber(c, &len_)) { 
        return false;
      }
      buf->AdvanceRead(1);
      break;
    case PARSE_BULK_CONTENT:
      if (len_ <= 0) {
        return false;
      }
      if (buf->ReadableLength() < len_) {
        return false;
      }
      buf->AdvanceRead(len_);
      state_ = PARSE_BULK_END;
      break;
    case PARSE_BULK_END:
      buf->AdvanceRead(1);
      markEndPos(buf);
      buf->AdvanceRead(1);
      return true;
      break;
    }
  }

  return false;
}

bool RedisIntItem::Parse() {
  char c;
  Buffer *buf;

  state_ = PARSE_INT_BEGIN;

  while (info_->hasUnprocessedQueryData()) {
    buf = info_->QueryBuffer();
    switch (state_) {
    case PARSE_INT_BEGIN:
      if (*(buf->NextRead()) != kRedisIntPrefix) {
        return false;
      }
      state_ = PARSE_INT_NUMBER;
      markStartPos(buf);
      buf->AdvanceRead(1);
      break;
    case PARSE_INT_NUMBER:
      c = *(buf->NextRead());
      if (c == '\r') {
        state_ = PARSE_INT_END;
        buf->AdvanceRead(1);
        if (*buf->NextRead() != '\n') {
          return false;
        }
        markEndPos(buf);
        return true;
      } else if (!isdigit(c)) { 
        return false;
      }
      buf->AdvanceRead(1);
      break;
    case PARSE_INT_END:
      break;
    }
  }

  return false;
}
