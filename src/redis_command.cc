#include "redis_command.h"

RedisCommand::RedisCommand() : status_(REDIS_COMMAND_NONE) {
}

RedisCommand::~RedisCommand() {
}

void RedisCommand::Init(Buffer *buf, int start) {
  start_.buffer_ = buf;
  start_.pos_    = start;
}

void RedisCommand::End(Buffer *buf, int end) {
  end_.buffer_ = buf;
  end_.pos_    = end;
  status_ = REDIS_COMMAND_READY;
}

BufferPos* RedisCommand::NextBufferPos() {
  if (current_.buffer_ == NULL) {
    current_.buffer_ = start_.buffer_;
    current_.pos_    = start_.buffer_->WritePos();
    return &current_;
  }

  current_.buffer_ = current_.buffer_->NextBuffer();
  if (current_.buffer_ == NULL) {
    current_.pos_ = 0;
  } else if (current_.buffer_ == end_.buffer_){
    current_.pos_ = end_.pos_;
  } else {
    current_.pos_ = current_.buffer_->WritableLength();
  }

  return &current_;
}
