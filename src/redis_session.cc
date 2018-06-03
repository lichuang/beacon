#include "redis_session.h"

Session* RedisSessionFactory::CreateSession(int fd, const string& ip, int port, Server *server) {
  return new RedisSession(fd, ip, port, server);
}

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
    server_->FreeSession(fd_);
    return ret;
  }

  if (mask & kEventWrite) {
    ret = handleWrite();
  }
  if (ret != kOk) {
    server_->FreeSession(fd_);
    return ret;
  }

  return kOk;
}

int RedisSession::handleRead() {
  
  return kOk;
}

int RedisSession::handleWrite() {
  return kOk;
}
