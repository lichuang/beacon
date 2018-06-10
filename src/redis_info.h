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
  list<RedisCommand*>* GetWaitReadCmds() {
    return &wait_read_cmds_;
  }

  void AddWaitWriteCmd(RedisCommand *cmd) {
    wait_write_cmds_.push_back(cmd);
  }
  list<RedisCommand*>* GetWaitWriteCmds() {
    return &wait_write_cmds_;
  }

private:  
  RedisParser parser_;
  RedisSession *session_;
  RedisServer  *server_;
  RedisCommand *current_cmd_;

  list<RedisCommand*> wait_read_cmds_;
  list<RedisCommand*> wait_write_cmds_;
};

#endif // __REDIS_INFO_H__
