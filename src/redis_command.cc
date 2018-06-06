#include "redis_command.h"

RedisCommand::RedisCommand() {
}

RedisCommand::~RedisCommand() {
}

void RedisCommand::Init(Buffer *buf, int start) {
  start_.buffer_ = buf;
  start_.pos_    = start;
}

void RedisCommand::End(Buffer *buf, int start) {
  end_.buffer_ = buf;
  end_.pos_    = start;
}
