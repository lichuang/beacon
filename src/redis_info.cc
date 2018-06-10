#include "redis_command.h"
#include "redis_info.h"
#include "redis_session.h"
#include "redis_server.h"

RedisInfo::RedisInfo(RedisSession *session)
  : parser_(this),
    session_(session),
    server_(NULL) {
}

RedisInfo::RedisInfo(RedisServer *server)
  : parser_(this),
    session_(NULL),
    server_(server) {
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

RedisCommand* RedisInfo::Parse(Buffer *buffer, int mode) {
  return parser_.Parse(buffer, mode);
}
