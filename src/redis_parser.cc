#include "buffer.h"
#include "common.h"
#include "log.h"
#include "redis_parser.h"
#include "redis_command.h"
#include "redis_session.h"

RedisParser::RedisParser()
  : item_index_(0) {
  state_fun_[PARSE_BEGIN] = &RedisParser::parseBegin;
}

RedisParser::~RedisParser() {
}

RedisItem* RedisParser::newItem(int type) {
  RedisItem *item;
  
  switch (type) {
  case REDIS_SIMPLE_STRING:
    item = new RedisSimpleStringItem();
    break;
  default:
    break;
  }

  items_.push_back(item);
  return item;
}

RedisItem* RedisParser::nextItem() {
  return items_[item_index_++];
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
      state_ = PARSE_ARRAY;
      item_size_ = 0;
      break;
    case '$':
      type_ = REDIS_STRING;
      state_ = PARSE_STRING;
      item_size_ = 0;
    case '+':
      type_ = REDIS_SIMPLE_STRING;
      state_ = PARSE_SIMPLE_STRING;
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

  newItem(type_);
  buf->AdvanceRead(1);
  return true;
}

bool RedisParser::parseSimpleString(RedisCommand *cmd, RedisSession *session) {
  char c;
  int  state;
  Buffer *buf;
  RedisSimpleStringItem* item = (RedisSimpleStringItem*)nextItem();

  UNUSED(cmd);

  item->state_ = PARSE_SIMPLE_STRING_BEGIN;

  while (session->hasUnprocessedQueryData()) {
    buf = session->QueryBuffer();
    state = item->state_;
    switch (state) {
    case PARSE_SIMPLE_STRING_BEGIN:
      item->state_ = PARSE_SIMPLE_STRING_LENGTH;
      item->start_.buffer_ = buf;
      item->start_.pos_ = buf->ReadPos();
      break;
    case PARSE_SIMPLE_STRING_LENGTH:
      c = *(buf->NextRead());
      if (c == '\r') {
        item->state_ = PARSE_SIMPLE_STRING_END;
      } else {
        item->len_ += 1;
        buf->AdvanceRead(1);
      }

      break;
    case PARSE_SIMPLE_STRING_END:
      item->end_.buffer_ = buf;
      item->end_.pos_ = buf->ReadPos();
      // buf->AdvanceRead(2);
      break;
    }
  }

  return true;
}
