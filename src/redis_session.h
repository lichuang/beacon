#ifndef __REDIS_SESSION_H__
#define __REDIS_SESSION_H__

#include <list>
#include "net.h"
#include "redis_command.h"
#include "session.h"
#include "redis_info.h"

class Server;

class RedisSession : public Session {
public:
  RedisSession(int fd, const string& ip, int port, Server *server);
  virtual ~RedisSession();

  virtual int Handle(int mask);

  void addWaitingCommand(RedisCommand *);

private:
  int handleRead();
  int handleWrite();

private:
  RedisInfo info_;
};

class RedisSessionFactory : public SessionFactory {
public:
  RedisSessionFactory() {}
  virtual ~RedisSessionFactory() {}

  virtual Session* CreateSession(int fd, const string& ip, int port, Server *server) {
    return new RedisSession(fd, ip, port, server);
  }
};

#endif // __REDIS_SESSION_H__
