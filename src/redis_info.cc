#include "redis_command.h"
#include "redis_info.h"
#include "redis_session.h"
#include "redis_server.h"

RedisInfo::RedisInfo(RedisSession *session)
  : parser_(this),
    session_(session),
    server_(NULL),
    current_write_cmd_(NULL),
    current_read_cmd_(NULL) {
}

RedisInfo::RedisInfo(RedisServer *server)
  : parser_(this),
    session_(NULL),
    server_(server) {
}

RedisCommand* RedisInfo::GetFreeCommand() {
  return new RedisCommand();
}

RedisCommand* RedisInfo::NextWriteCommand() {
  if (current_write_cmd_ != NULL) {
    return current_write_cmd_;
  }

  if (wait_write_cmds_.empty()) {
    return NULL;
  }
  current_write_cmd_ = wait_write_cmds_.front();
  wait_write_cmds_.pop_front();

  // ready to write cmd 
  current_write_cmd_->ReadyWrite(); 
  return current_write_cmd_;
}

RedisCommand* RedisInfo::NextReadCommand() {
  if (current_read_cmd_ != NULL) {
    return current_read_cmd_;
  }

  if (wait_read_cmds_.empty()) {
    return NULL;
  }
  current_read_cmd_ = wait_read_cmds_.front();
  wait_read_cmds_.pop_front();

  return current_read_cmd_;
}
