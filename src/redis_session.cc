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

RedisCommand* RedisSession::getFreeCommand() {
  RedisCommand *cmd;

  if (free_commands_.empty()) {
    cmd = new RedisCommand();
  } else {
    cmd = free_commands_.front();
    free_commands_.pop_front();
  }

  return cmd;
}

void RedisSession::addWaitingCommand(RedisCommand *cmd) {
  waiting_commands_.push_back(cmd);
}

int RedisSession::handleRead() {
  int ret;

  ret = TcpRead(fd_, query_buf_);

  if (ret < 0) {
    return ret;
  }

  Infof("query from %s %s", address_.String(), query_buf_->Start());
  return kOk;
}

int RedisSession::handleWrite() {
  return kOk;
}
