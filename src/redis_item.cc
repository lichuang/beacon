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
  int type;

  while (buffer->hasUnprocessedData() && !ready_) {
    c = *(buffer->NextRead());

    switch (state_) {
    case PARSE_ARRAY_BEGIN:
      if (c != kRedisArrayPrefix) {
        return false;
      }
      markItemStartPos(buffer);
      state_ = PARSE_ARRAY_LENGTH;
      sign_ = 1;
      item_num_ = 0;
      buffer->AdvanceRead(1);
      break;
    case PARSE_ARRAY_LENGTH:
      switch (c) {
      case '-':
        sign_ = -1;
        break;
      case '\r':
        state_ = PARSE_ARRAY_LENGTH_N;
        break;
      default:
        if (!toNumber(c, &item_num_)) {
          return false;
        }
        break;
      }
      buffer->AdvanceRead(1);
      break;
    case PARSE_ARRAY_LENGTH_N:
      if (item_num_ < 0) {
        return false;
      }
      if (c != '\n') {
        return false;
      }
      state_ = PARSE_ARRAY_ITEM_TYPE;
      buffer->AdvanceRead(1);
      break;
    case PARSE_ARRAY_ITEM_TYPE:
      if (sign_ < 0) { // NULL array
        state_ = PARSE_ARRAY_END;
        item_num_ = 0;
      } else {
        if (!ParseType(c, &type)) {
          return false;
        }
        current_ = newRedisItem(type);
        array_.push_back(current_);
        state_ = PARSE_ARRAY_ITEM;
      }
      break;
    case PARSE_ARRAY_ITEM:
      if (!current_->Parse(buffer)) {
        return false;
      }
      if (!current_->Ready()) {
        return true;
      }
      if (int(array_.size()) == item_num_) {
        state_ = PARSE_ARRAY_END;
      } else {
        state_ = PARSE_ARRAY_ITEM_TYPE;
      }
      break;
    case PARSE_ARRAY_END:
      markItemEndPos(buffer);
      break;
    }
  }

  if (!ready_ && state_ == PARSE_ARRAY_END) {
    markItemEndPos(buffer);
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

  while (buffer->hasUnprocessedData() && !ready_) {
    c = *(buffer->NextRead());
    switch (state_) {
    case PARSE_STRING_BEGIN:
      if (c != kRedisStringPrefix) {
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
      if (c == '\r') {
        markItemValueEndPos(buffer);
        state_ = PARSE_STRING_N;
      }
      break;
    case PARSE_STRING_N:
      if (c != '\n') {
        return false;
      } else {
        state_ = PARSE_STRING_END;
      }
    case PARSE_STRING_END:
      markItemEndPos(buffer);
      break;
    }
    buffer->AdvanceRead(1);
  }
  if (!ready_ && state_ == PARSE_STRING_END) {
    markItemEndPos(buffer);
  }

  return true;
}

bool RedisBulkItem::Parse(Buffer *buffer) {
  char c;

  while (buffer->hasUnprocessedData() && !ready_) {
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
  if (state_ == PARSE_BULK_END && !ready_) {
    markItemEndPos(buffer);
  }

  return true;
}

bool RedisIntItem::Parse(Buffer *buffer) {
  char c;

  while (buffer->hasUnprocessedData() && !ready_) {
    c = *(buffer->NextRead());
    switch (state_) {
    case PARSE_INT_BEGIN:
      if (c != kRedisIntPrefix) {
        return false;
      }
      state_ = PARSE_INT_NUMBER;
      markItemStartPos(buffer);
      break;
    case PARSE_INT_NUMBER:
      if (!marked_int_) {
        marked_int_ = true;
        markItemValueStartPos(buffer);
      }
      if (c == '\r') {
        state_ = PARSE_INT_END;
        markItemValueEndPos(buffer);
      } else if (!isdigit(c)) { 
        return false;
      }
      break;
    case PARSE_INT_END:
      markItemEndPos(buffer);
      break;
    }
    buffer->AdvanceRead(1);
  }

  if (!ready_ && state_ == PARSE_INT_END) {
    markItemEndPos(buffer);
  }

  return true;
}
