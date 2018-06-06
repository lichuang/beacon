#ifndef __SESSION_H__
#define __SESSION_H__

#include <list>
#include <string>
#include "const.h"
#include "buffer.h"
#include "event.h"
#include "net.h"

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
      address_(ip, port),
      server_(server),
      query_buf_(new Buffer(kQueryBufferLen)) {}
  virtual ~Session() {
    delete query_buf_;
  }

  virtual int Handle(int mask) = 0;

  int Fd() { return fd_; }
  const char* String() { return address_.String(); }
  
  Buffer* QueryBuffer() {
    return query_buf_;
  }

  bool hasUnprocessedQueryData() {
    if (!query_buf_) {
      return false;
    }

    return query_buf_->ReadableLength() > 0;
  }

protected:
  int fd_;
  Address address_;
  Server* server_;
  Buffer* query_buf_;
  list<Buffer*> free_buf_;
  list<Buffer*> processing_buf_;
};

#endif// __SESSION_H__
