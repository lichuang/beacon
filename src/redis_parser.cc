#include "buffer.h"
#include "const.h"
#include "common.h"
#include "log.h"
#include "redis_parser.h"
#include "redis_command.h"
#include "redis_info.h"

bool ParseType(char c, int *type) {
  *type = REDIS_NONE_TYPE;
  switch (c) {
  case kRedisArrayPrefix:
    *type = REDIS_ARRAY;
    break;
  case kRedisStringPrefix:
    *type = REDIS_STRING;
    break;
  case kRedisBulkPrefix:
    *type = REDIS_BULK;
    break;
  case kRedisIntPrefix:
    *type = REDIS_INT;
    break;
  default:
    Errorf("unknown command type: %c", c);
    return false;
  }
  return true;
}

RedisParser::RedisParser(RedisInfo *info)
  : info_(info), cmd_(NULL) {
  state_fun_[PARSE_BEGIN] = &RedisParser::parseBegin;
  state_fun_[PARSE_ITEM]  = &RedisParser::parseItem;
  state_fun_[PARSE_END]   = &RedisParser::parseEnd;
  reset();
}

RedisParser::~RedisParser() {
}

RedisCommand* RedisParser::Parse(Buffer *buffer, int mode) {
  mode_ = mode;
  if (cmd_ == NULL) {
    cmd_ = info_->getFreeCommand();
  }
  buffer_ = buffer;

  while (buffer_ && buffer_->hasUnprocessedData()) {
    if (!(this->*state_fun_[state_])()) {
      cmd_->SetStatus(REDIS_COMMAND_ERROR);
      return cmd_;
    }
    if (cmd_ && cmd_->GetReady()) {
      return cmd_;
    }
  }
  return cmd_;
}

void RedisParser::reset() {
  state_ = PARSE_BEGIN;
  type_  = REDIS_NONE_TYPE;
}

bool RedisParser::parseBegin() {
  cmd_->Init(buffer_, buffer_->ReadPos());
  state_ = PARSE_ITEM;
  return true;
}

bool RedisParser::parseItem() {
  // TODO: 
  if (item_ != NULL) {

  }
  char t = *(buffer_->NextRead());

  if (!ParseType(t, &type_)) {
    return false;
  }
  item_ = newRedisItem(type_);
  state_ = PARSE_END;
  return item_->Parse(buffer_);
}

bool RedisParser::parseEnd() {
  if (*buffer_->NextRead() != '\n') {
    return false;
  }
  buffer_->AdvanceRead(1);
  cmd_->End(buffer_, buffer_->ReadPos());

  reset();

  return true;
}
