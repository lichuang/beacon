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

RedisCommand* RedisInfo::GetFreeCommand() {
  return new RedisCommand();
}
