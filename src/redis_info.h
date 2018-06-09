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

  Buffer* QueryBuffer();
  void    addWaitingCommand(RedisCommand *);
  RedisCommand* getFreeCommand();

  bool hasUnprocessedQueryData();

  bool Parse();

private:  
  RedisParser parser_;
  RedisSession *session_;
  RedisServer  *server_;
  list<RedisCommand*> free_commands_;
};

#endif // __REDIS_INFO_H__
