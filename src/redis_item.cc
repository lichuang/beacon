#include <stdio.h>
#include <ctype.h>
#include "const.h"
#include "common.h"
#include "redis_item.h"
#include "redis_command.h"
#include "redis_parser.h"

static bool toNumber(char c, int *v) {
  if (!isdigit(c)) {
    return false;
  }
  *v = *v * 10 + c - '0';
  return true;
}

RedisItem* newRedisItem(int type) {
  switch (type) {
  case REDIS_ARRAY:
    return new RedisArrayItem();
  case REDIS_STRING:
    return new RedisStringItem();
  case REDIS_BULK:
    return new RedisBulkItem();
  case REDIS_INT:
    return new RedisIntItem();
  default:
    break;
  }

  return NULL;
}

bool RedisArrayItem::Parse(Buffer *buffer) {
  char c;
  int i, type;
  RedisItem *item;

  state_ = PARSE_ARRAY_BEGIN;

  while (buffer->hasUnprocessedData() && state_ != PARSE_ARRAY_END) {
    switch (state_) {
    case PARSE_ARRAY_BEGIN:
      if (*(buffer->NextRead()) != kRedisArrayPrefix) {
        return false;
      }
      state_ = PARSE_ARRAY_LENGTH;
      sign_ = 1;
      item_num_ = 0;
      buffer->AdvanceRead(1);
      break;
    case PARSE_ARRAY_LENGTH:
      c = *(buffer->NextRead());
      switch (c) {
      case '-':
        sign_ = -1;
        break;
      case '\r':
        state_ = PARSE_ARRAY_ITEM;
        break;
      default:
        if (!toNumber(c, &item_num_)) {
          return false;
        }
        break;
      }
      buffer->AdvanceRead(1);
      break;
    case PARSE_ARRAY_ITEM:
      if (sign_ < 0) { // NULL array
        state_ = PARSE_ARRAY_END;
        item_num_ = 0;
      } else {
        for (i = 0; i < item_num_; ++i) {
          c = *(buffer->NextRead());
          if (c != '\n') {
            return false;
          }
          buffer->AdvanceRead(1);
          c = *(buffer->NextRead());
          if (!ParseType(c, &type)) {
            return false;
          }
          item = newRedisItem(type);
          array_.push_back(item);
          if (!item->Parse(buffer)) {
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

bool RedisStringItem::Parse(Buffer *buffer) {
  char c;

  state_ = PARSE_STRING_BEGIN;

  while (buffer->hasUnprocessedData()) {
    switch (state_) {
    case PARSE_STRING_BEGIN:
      if (*(buffer->NextRead()) != kRedisStringPrefix) {
        return false;
      }
      state_ = PARSE_STRING_LENGTH;
      markStartPos(buffer);
      buffer->AdvanceRead(1);
      break;
    case PARSE_STRING_LENGTH:
      c = *(buffer->NextRead());
      if (c == '\r') {
        state_ = PARSE_STRING_END;
      } else {
        len_ += 1;
      }
      buffer->AdvanceRead(1);
      break;
    case PARSE_STRING_END:
      c = *(buffer->NextRead());
      markEndPos(buffer);
      return true;
      break;
    }
  }

  return false;
}

bool RedisBulkItem::Parse(Buffer *buffer) {
  char c;

  state_ = PARSE_BULK_BEGIN;

  while (buffer->hasUnprocessedData()) {
    switch (state_) {
    case PARSE_BULK_BEGIN:
      if (*(buffer->NextRead()) != kRedisBulkPrefix) {
        return false;
      }
      state_ = PARSE_BULK_LENGTH;
      markStartPos(buffer);
      len_ = 0; 
      buffer->AdvanceRead(1);
      break;
    case PARSE_BULK_LENGTH:
      c = *(buffer->NextRead());
      if (c == '\r') {
        state_ = PARSE_BULK_CONTENT;
        buffer->AdvanceRead(1);
      } else if (!toNumber(c, &len_)) { 
        return false;
      }
      buffer->AdvanceRead(1);
      break;
    case PARSE_BULK_CONTENT:
      if (len_ <= 0) {
        return false;
      }
      if (buffer->ReadableLength() < len_) {
        return false;
      }
      buffer->AdvanceRead(len_);
      state_ = PARSE_BULK_END;
      break;
    case PARSE_BULK_END:
      buffer->AdvanceRead(1);
      markEndPos(buffer);
      return true;
      break;
    }
  }

  return false;
}

bool RedisIntItem::Parse(Buffer *buffer) {
  char c;

  state_ = PARSE_INT_BEGIN;

  while (buffer->hasUnprocessedData() && state_ != PARSE_INT_END) {
    switch (state_) {
    case PARSE_INT_BEGIN:
      if (*(buffer->NextRead()) != kRedisIntPrefix) {
        return false;
      }
      state_ = PARSE_INT_NUMBER;
      markStartPos(buffer);
      buffer->AdvanceRead(1);
      break;
    case PARSE_INT_NUMBER:
      c = *(buffer->NextRead());
      if (c == '\r') {
        state_ = PARSE_INT_END;
        markEndPos(buffer);
        return true;
      } else if (!isdigit(c)) { 
        return false;
      }
      buffer->AdvanceRead(1);
      break;
    case PARSE_INT_END:
      break;
    }
  }

  return false;
}
