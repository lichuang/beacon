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
      addr_buf_(Buffer(kClientAddressSize)),
      server_(server),
      query_buf_(new Buffer(kQueryBufferLen)) {
    snprintf(addr_buf_.Start(), addr_buf_.WritableLength(), "%s:%d", ip.c_str(), port);      
    addr_buf_.AdvanceWrite(kClientAddressSize);
  }
  virtual ~Session() {
    delete query_buf_;
  }

  virtual int Handle(int mask) = 0;

  int Fd() { return fd_; }
  const char* String() { return addr_buf_.Start(); }

protected:
  int fd_;
  Buffer addr_buf_;
  Server* server_;
  Buffer* query_buf_;
};

#endif// __SESSION_H__
