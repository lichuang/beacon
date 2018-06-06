#include "buffer.h"
#include "common.h"
#include "log.h"
#include "redis_parser.h"
#include "redis_command.h"
#include "redis_session.h"

RedisParser::RedisParser() {
  state_fun_[PARSE_BEGIN] = &RedisParser::parseBegin;
  state_fun_[PARSE_TYPE]  = &RedisParser::parseType;
  state_fun_[PARSE_ITEM]  = &RedisParser::parseItem;
  state_fun_[PARSE_END]   = &RedisParser::parseEnd;
}

RedisParser::~RedisParser() {
}

bool RedisParser::Parse(RedisSession *session) {
  RedisCommand *cmd;

  while (session->hasUnprocessedQueryData() > 0) {
    cmd = session->getFreeCommand();
    
    while (state_ <= PARSE_END) {
      if (!(this->*state_fun_[state_])(cmd, session)) {
        return false;
      }
    }

    reset();
  }
  return true;
}

void RedisParser::reset() {

}

bool RedisParser::parseBegin(RedisCommand *cmd, RedisSession *session) {
  Buffer *buf = session->QueryBuffer();
  cmd->Init(buf, buf->ReadPos());
  return true;
}

bool RedisParser::parseItem(RedisCommand *cmd, RedisSession *session) {
  switch (type_) {
  case REDIS_SIMPLE_STRING:
    item_ = new RedisSimpleStringItem(cmd, session);
    break;
  default:
    break;
  }

  return item_->Parse();
}

bool RedisParser::parseEnd(RedisCommand *cmd, RedisSession *session) {
  Buffer *buf = session->QueryBuffer();
  cmd->End(buf, buf->ReadPos());
  session->addWaitingCommand(cmd);

  return true;
}

bool RedisParser::parseType(RedisCommand *cmd, RedisSession *session) {
  Buffer *buf = session->QueryBuffer();
  char t = *(buf->NextRead());

  UNUSED(cmd);

  while (state_ == PARSE_TYPE) {
    switch (t) {
    case '*':
      type_ = REDIS_ARRAY;
      item_size_ = 0;
      break;
    case '$':
      type_ = REDIS_STRING;
      item_size_ = 0;
    case '+':
      type_ = REDIS_SIMPLE_STRING;
      break;
    case '\r':
    case '\n':
      // pass through \r\n
      buf->AdvanceRead(1);
      break;
    default:
      Errorf("unknown command type: %c", t);
      break;
    }
  }

  state_ = PARSE_ITEM;
  buf->AdvanceRead(1);
  return true;
}
