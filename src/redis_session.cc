#include "errcode.h"
#include "log.h"
#include "net.h"
#include "redis_session.h"
#include "server.h"

RedisSession::RedisSession(int fd, const string& ip, int port, Server *server)
  : Session(fd, ip, port, server) {
}

RedisSession::~RedisSession() {
}

int RedisSession::Handle(int mask) {
  int ret = kOk;

  if (mask & kEventRead) {
    ret = handleRead();
  }
  if (ret != kOk) {
    server_->FreeSession(this);
    return ret;
  }

  if (mask & kEventWrite) {
    ret = handleWrite();
  }
  if (ret != kOk) {
    server_->FreeSession(this);
    return ret;
  }

  return kOk;
}

int RedisSession::handleRead() {
  int ret;

  ret = TcpRead(fd_, query_buf_);

  if (ret < 0) {
    return ret;
  }

  Infof("query from %s %s", String(), query_buf_->Start());
  return kOk;
}

int RedisSession::handleWrite() {
  return kOk;
}
