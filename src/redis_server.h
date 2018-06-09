#ifndef __REDIS_SERVER_H__
#define __REDIS_SERVER_H__

#include <list>
#include <string>
#include "event.h"
#include "net.h"
#include "redis_session.h"

using namespace std;

class RedisCommand;
class Engine;

class RedisServer : public Session {
public:
  RedisServer(const Address&, RedisSession *);
  ~RedisServer();

  bool Connect();
  void addQueryCommand(RedisCommand* cmd);

  virtual int Handle(int mask);

  Buffer* QueryBuffer() {
    return query_buf_;
  }

private:
  int handleWrite();
  int handleRead();

private:
  list<RedisCommand*> query_cmds_;
  list<RedisCommand*> waiting_cmds_;
  RedisCommand *current_cmd_;
  RedisSession *session_;
  Buffer *query_buf_;
  int status_;
};

RedisServer* CreateServer(const Address& address, RedisSession *session);
#endif // __REDIS_SERVER_H__
