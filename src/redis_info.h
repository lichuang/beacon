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

  RedisCommand* getFreeCommand();

  RedisCommand* Parse(Buffer *, int );

private:  
  RedisParser parser_;
  RedisSession *session_;
  RedisServer  *server_;
  list<RedisCommand*> free_commands_;
};

#endif // __REDIS_INFO_H__
