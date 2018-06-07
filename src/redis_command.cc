#include "redis_command.h"

RedisCommand::RedisCommand() {
}

RedisCommand::~RedisCommand() {
}

void RedisCommand::Init(Buffer *buf, int start) {
  buffer_pos_.buffer_ = buf;
  buffer_pos_.start_    = start;
}

void RedisCommand::End(int end) {
  buffer_pos_.end_    = end;
}
