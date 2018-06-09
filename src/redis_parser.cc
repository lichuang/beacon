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

bool RedisParser::Parse() {
  while (info_->hasUnprocessedQueryData() > 0) {
    parse_cmd_done_ = false;
    while (!parse_cmd_done_) {
      if (!(this->*state_fun_[state_])()) {
        return false;
      }
    }
  }
  return true;
}

void RedisParser::reset() {
  state_ = PARSE_BEGIN;
  cmd_   = NULL;
  item_  = NULL;
  type_  = REDIS_NONE_TYPE;
}

bool RedisParser::parseBegin() {
  Buffer *buf = info_->QueryBuffer();
  cmd_ = info_->getFreeCommand();
  cmd_->Init(buf, buf->ReadPos());
  state_ = PARSE_ITEM;
  return true;
}

bool RedisParser::parseItem() {
  Buffer *buf = info_->QueryBuffer();
  char t = *(buf->NextRead());

  if (!ParseType(t, &type_)) {
    return false;
  }
  item_ = newRedisItem(type_, cmd_, info_);
  state_ = PARSE_END;
  return item_->Parse();
}

bool RedisParser::parseEnd() {
  Buffer *buf = info_->QueryBuffer();
  cmd_->End(buf, buf->ReadPos());
  info_->addWaitingCommand(cmd_);
  cmd_ = NULL;
  parse_cmd_done_ = true;

  reset();

  return true;
}
