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

void RedisItem::GetValue(string *val) {
  Buffer *buf = value_pos_.start_.buffer_;
  char *start;
  int cnt;

  *val = "";
  // start and end in the same buffer case
  if (value_pos_.start_.buffer_ == value_pos_.end_.buffer_) {
    buf = value_pos_.start_.buffer_;
    start = buf->Start() + value_pos_.start_.pos_;
    cnt = value_pos_.end_.pos_ - value_pos_.start_.pos_;
    *val = string(start, cnt);
    return;
  }

  // start and end in the different buffer case
  while (buf != NULL) {
    if (buf == value_pos_.start_.buffer_) { // start buffer
      start = buf->Start() + value_pos_.start_.pos_;
      cnt = buf->WritePos() - value_pos_.start_.pos_;
      *val = string(start, cnt);
    } else if (buf == value_pos_.end_.buffer_) { // end buffer
      start = buf->Start();
      cnt = value_pos_.end_.pos_;
      *val += string(start, cnt);
    } else { // mid buffers
      start = buf->Start();
      cnt = buf->WritePos();
      *val += string(start, cnt);
    }
    buf = buf->NextBuffer();
  }
}

bool RedisArrayItem::Parse(Buffer *buffer) {
  char c;
  int i, type;
  RedisItem *item;

  while (buffer->hasUnprocessedData() && state_ != PARSE_ARRAY_END) {
    switch (state_) {
    case PARSE_ARRAY_BEGIN:
      if (*(buffer->NextRead()) != kRedisArrayPrefix) {
        return false;
      }
      markItemStartPos(buffer);
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
      markItemEndPos(buffer);
      break;
    }
  }

  return true;
}

RedisArrayItem::~RedisArrayItem() {
  size_t i;
  for (i = 0; i < array_.size(); ++i) {
    delete array_[i];
  }
}

bool RedisStringItem::Parse(Buffer *buffer) {
  char c;

  while (buffer->hasUnprocessedData() && state_ < PARSE_STRING_END) {
    switch (state_) {
    case PARSE_STRING_BEGIN:
      if (*(buffer->NextRead()) != kRedisStringPrefix) {
        return false;
      }
      state_ = PARSE_STRING_CONTENT;
      markItemStartPos(buffer);
      break;
    case PARSE_STRING_CONTENT:
      markItemValueStartPos(buffer);
      state_ = PARSE_STRING_R;
      break;
    case PARSE_STRING_R:
      c = *(buffer->NextRead());
      if (c == '\r') {
        markItemValueEndPos(buffer);
        state_ = PARSE_STRING_N;
      }
      break;
    case PARSE_STRING_N:
      c = *(buffer->NextRead());
      if (c != '\n') {
        return false;
      } else {
        state_ = PARSE_STRING_END;
      }
    case PARSE_STRING_END:
      c = *(buffer->NextRead());
      markItemEndPos(buffer);
      break;
    }
    buffer->AdvanceRead(1);
  }

  return true;
}

bool RedisBulkItem::Parse(Buffer *buffer) {
  char c;

  while (buffer->hasUnprocessedData()) {
    c = *(buffer->NextRead());
    switch (state_) {
    case PARSE_BULK_BEGIN:
      if (c != kRedisBulkPrefix) {
        return false;
      }
      state_ = PARSE_BULK_LENGTH_R;
      markItemStartPos(buffer);
      len_ = 0; 
      break;
    case PARSE_BULK_LENGTH_R:
      if (c == '\r') {
        if (len_ <= 0) {
          return false;
        }
        state_ = PARSE_BULK_LENGTH_N;
      } else if (!toNumber(c, &len_)) { 
        return false;
      }
      break;
    case PARSE_BULK_LENGTH_N:
      if (c != '\n') {
        return false;
      }
      state_ = PARSE_BULK_CONTENT;
      readed_len_ = 0;
      break;
    case PARSE_BULK_CONTENT:
      if (readed_len_ == 0) {
        markItemValueStartPos(buffer);
      } 
      ++readed_len_;
      if (readed_len_ == len_) {
        state_ = PARSE_BULK_CONTENT_R;
      }
      break;
    case PARSE_BULK_CONTENT_R:
      if (c != '\r') {
        return false;
      }
      markItemValueEndPos(buffer);
      state_ = PARSE_BULK_CONTENT_N;
      break;
    case PARSE_BULK_CONTENT_N:
      if (c != '\n') {
        return false;
      }
      state_ = PARSE_BULK_END;
      break;
    case PARSE_BULK_END:
      markItemEndPos(buffer);
      return true;
      break;
    }
    buffer->AdvanceRead(1);
  }
  if (state_ == PARSE_BULK_END && !buffer->hasUnprocessedData() && !ready_) {
    markItemEndPos(buffer);
  }

  return true;
}

bool RedisIntItem::Parse(Buffer *buffer) {
  char c;

  while (buffer->hasUnprocessedData()) {
    switch (state_) {
    case PARSE_INT_BEGIN:
      if (*(buffer->NextRead()) != kRedisIntPrefix) {
        return false;
      }
      state_ = PARSE_INT_NUMBER;
      markItemStartPos(buffer);
      buffer->AdvanceRead(1);
      break;
    case PARSE_INT_NUMBER:
      c = *(buffer->NextRead());
      if (c == '\r') {
        state_ = PARSE_INT_END;
      } else if (!isdigit(c)) { 
        return false;
      }
      buffer->AdvanceRead(1);
      break;
    case PARSE_INT_END:
      markItemEndPos(buffer);
      return true;
      break;
    }
  }

  return false;
}
