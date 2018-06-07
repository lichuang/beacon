#include "buffer.h"
#include "const.h"
#include "common.h"
#include "log.h"
#include "redis_parser.h"
#include "redis_command.h"
#include "redis_session.h"

bool ParseType(char c, int *type) {
  switch (c) {
  case kRedisArrayPrefix:
    *type = REDIS_ARRAY;
    break;
  case kRedisStringPrefix:
    *type = REDIS_STRING;
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

RedisParser::RedisParser(RedisSession *session)
  : session_(session), cmd_(NULL) {
  state_fun_[PARSE_BEGIN] = &RedisParser::parseBegin;
  state_fun_[PARSE_TYPE]  = &RedisParser::parseType;
  state_fun_[PARSE_ITEM]  = &RedisParser::parseItem;
  state_fun_[PARSE_END]   = &RedisParser::parseEnd;
  reset();
}

RedisParser::~RedisParser() {
}

bool RedisParser::Parse() {
  while (session_->hasUnprocessedQueryData() > 0) {
    while (state_ <= PARSE_END) {
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
  Buffer *buf = session_->QueryBuffer();
  cmd_ = session_->getFreeCommand();
  cmd_->Init(buf, buf->ReadPos());
  state_ = PARSE_TYPE;
  Debugf("in parse begin state");
  return true;
}

bool RedisParser::parseItem() {
  Buffer *buf = session_->QueryBuffer();
  char t = *(buf->NextRead());

  if (!ParseType(t, &type_)) {
    return false;
  }
  item_ = newRedisItem(type_, cmd_, session_);
  state_ = PARSE_END;
  return item_->Parse();
}

bool RedisParser::parseEnd() {
  Buffer *buf = session_->QueryBuffer();
  cmd_->End(buf, buf->ReadPos());
  session_->addWaitingCommand(cmd_);

  reset();

  return true;
}
