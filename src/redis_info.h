#ifndef __REDIS_INFO_H__
#define __REDIS_INFO_H__

#include <list>
#include "redis_parser.h"

using namespace std;

class RedisSession;
class RedisServer;

class RedisInfo {
public:
  RedisInfo(RedisSession *session);
  RedisInfo(RedisServer *server);

  RedisCommand* GetFreeCommand();

  RedisParser* GetParser() {
    return &parser_;
  }

  void AddWaitReadCmd(RedisCommand *cmd) {
    wait_read_cmds_.push_back(cmd);
  }

  void AddWaitWriteCmd(RedisCommand *cmd) {
    wait_write_cmds_.push_back(cmd);
  }

  void ResetWriteCommand() {
    current_write_cmd_ = NULL;
  }
  void ResetReadCommand() {
    current_read_cmd_ = NULL;
  }

  RedisCommand* NextWriteCommand();
  RedisCommand* NextReadCommand();
private:  
  RedisParser parser_;
  RedisSession *session_;
  RedisServer  *server_;
  RedisCommand *current_cmd_;

  list<RedisCommand*> wait_read_cmds_;
  list<RedisCommand*> wait_write_cmds_;
  RedisCommand *current_write_cmd_;
  RedisCommand *current_read_cmd_;
};

#endif // __REDIS_INFO_H__
