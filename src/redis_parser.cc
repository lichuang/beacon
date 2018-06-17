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
  : info_(info), item_(NULL), cmd_(NULL) {
  state_fun_[PARSE_BEGIN] = &RedisParser::parseBegin;
  state_fun_[PARSE_ITEM]  = &RedisParser::parseItem;
  state_fun_[PARSE_END]   = &RedisParser::parseEnd;
  reset();
}

RedisParser::~RedisParser() {
}

RedisCommand* RedisParser::Parse(Buffer *buffer, RedisCommand *cmd) {
  if (cmd == NULL) { // req mode
    mode_ = REDIS_REQ_MODE;
    cmd_ = info_->GetFreeCommand();
  } else { // response mode
    mode_ = REDIS_REP_MODE;
    cmd_ = cmd;
  }
  buffer_ = buffer;

  while (buffer_ && buffer_->hasUnprocessedData()) {
    if (!(this->*state_fun_[state_])()) {
      cmd_->MarkError();
      return cmd_;
    }
    if (cmd_ && cmd_->Ready()) {
      return cmd_;
    }
  }
  return cmd_;
}

void RedisParser::reset() {
  state_ = PARSE_BEGIN;
  type_  = REDIS_NONE_TYPE;
  item_  = NULL;
}

bool RedisParser::parseBegin() {
  reset();
  cmd_->Init(buffer_, buffer_->ReadPos());
  state_ = PARSE_ITEM;
  return true;
}

bool RedisParser::parseItem() {
  char t = *(buffer_->NextRead());

  if (!ParseType(t, &type_)) {
    buffer_->AdvanceRead(1);
    return false;
  }
  item_ = newRedisItem(type_);
  if (!item_->Parse(buffer_)) {
    return false;
  }
  if (item_->Ready()) {
    state_ = PARSE_END;
    cmd_->End(buffer_, buffer_->ReadPos(), item_);
  }
  return true;
}

bool RedisParser::parseEnd() {
  return true;
}
