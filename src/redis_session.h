#ifndef __REDIS_SESSION_H__
#define __REDIS_SESSION_H__

#include "session.h"

class RedisSession;

class RedisSessionFactory : public SessionFactory {
public:
  RedisSessionFactory() {}
  virtual ~RedisSessionFactory() {}

  virtual Session* CreateSession(int fd, const string& ip, int port, Server *server);
};

class RedisSession : public Session {
public:
  RedisSession(int fd, const string& ip, int port, Server *server);
  virtual ~RedisSession();

  virtual int Handle(int mask) = 0;
private:
  int handleRead();
  int handleWrite();
};

#endif // __REDIS_SESSION_H__
