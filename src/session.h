#ifndef __SESSION_H__
#define __SESSION_H__

#include <string>
#include "const.h"
#include "buffer.h"
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
      server_(server)
      query_buf_(new Buffer(kQueryBufferLen)) {}
  virtual ~Session() {
    delete query_buf_;
  }

  virtual int Handle(int mask) = 0;

protected:
  int fd_;
  string ip_;
  int port_;
  Server* server_;
  Buffer* query_buf_;
};

#endif// __SESSION_H__
