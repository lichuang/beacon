#include "redis_item.h"
#include "redis_command.h"
#include "redis_session.h"

bool RedisSimpleStringItem::Parse() {
  char c;
  Buffer *buf;

  state_ = PARSE_SIMPLE_STRING_BEGIN;

  while (session_->hasUnprocessedQueryData()) {
    buf = session_->QueryBuffer();
    switch (state_) {
    case PARSE_SIMPLE_STRING_BEGIN:
      state_ = PARSE_SIMPLE_STRING_LENGTH;
      start_.buffer_ = buf;
      start_.pos_ = buf->ReadPos();
      break;
    case PARSE_SIMPLE_STRING_LENGTH:
      c = *(buf->NextRead());
      if (c == '\r') {
        state_ = PARSE_SIMPLE_STRING_END;
      } else {
        len_ += 1;
        buf->AdvanceRead(1);
      }

      break;
    case PARSE_SIMPLE_STRING_END:
      end_.buffer_ = buf;
      end_.pos_ = buf->ReadPos();
      break;
    }
  }

  return true;
}
