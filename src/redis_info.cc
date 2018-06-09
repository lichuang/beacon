#include "redis_command.h"
#include "redis_info.h"
#include "redis_session.h"

RedisInfo::RedisInfo(RedisSession *session)
  : parser_(this),
    session_(session),
    server_(NULL) {
}

Buffer* RedisInfo::QueryBuffer() {
  if (session_) {
    return session_->QueryBuffer();
  }

  return NULL;
}

bool RedisInfo::hasUnprocessedQueryData() {
  Buffer *buffer = QueryBuffer();
  if (buffer == NULL) {
    return false;
  }

  return buffer->ReadableLength() > 0;
}

void RedisInfo::addWaitingCommand(RedisCommand *cmd) {
  if (session_) {
    session_->addWaitingCommand(cmd);
  }
}

RedisCommand* RedisInfo::getFreeCommand() {
  RedisCommand *cmd;

  if (free_commands_.empty()) {
    cmd = new RedisCommand();
  } else {
    cmd = free_commands_.front();
    free_commands_.pop_front();
  }

  return cmd;
}

bool RedisInfo::Parse() {
  return parser_.Parse();
}
