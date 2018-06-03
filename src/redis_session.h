#ifndef __REDIS_SESSION_H__
#define __REDIS_SESSION_H__

#include "session.h"

class RedisSession;

class RedisSession : public Session {
public:
  RedisSession(int fd, const string& ip, int port, Server *server);
  virtual ~RedisSession();

  virtual int Handle(int mask);
private:
  int handleRead();
  int handleWrite();
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
