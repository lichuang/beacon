#ifndef __SESSION_H__
#define __SESSION_H__

#include <string>
#include "event.h"

using namespace std;

class Session;
class Server;

class SessionFactory {
public:
  SessionFactory() {}
  virtual ~SessionFactory() {}

  virtual Session* CreateSession(int fd, const string& ip, int port, Server *server) = 0;
};

class Session : public IEventHandler {
public:
  Session(int fd, const string& ip, int port, Server *server)
    : fd_(fd),
      ip_(ip),
      port_(port),
      server_(server) {}
  virtual ~Session() {}

  virtual int Handle(int mask) = 0;

protected:
  int fd_;
  string ip_;
  int port_;
  Server *server_;
};

#endif// __SESSION_H__
