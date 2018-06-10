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

void RedisCommand::ReadyWrite() {
  Buffer *buffer = start_.buffer_;
  buffer->SetReadPos(start_.pos_);

  buffer = buffer->NextBuffer();
  while (buffer != NULL)  {
    buffer->SetReadPos(0);
    if (buffer == end_.buffer_) {
      buffer->SetWritePos(end_.pos_);
    }
  }
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
    return NULL;
  } else if (current_.buffer_ == end_.buffer_){
    current_.pos_ = end_.pos_;
  } else {
    current_.pos_ = current_.buffer_->WritableLength();
  }

  return &current_;
}

void RedisCommand::FreeBuffers() {
  Buffer *buffer = start_.buffer_;
  while (buffer != NULL) {
    buffer->DescCnt();
    if (buffer == end_.buffer_) {
      break;
    }
    buffer = buffer->NextBuffer();
  }
}
