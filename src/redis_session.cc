#include "errcode.h"
#include "log.h"
#include "net.h"
#include "redis_session.h"
#include "redis_parser.h"
#include "redis_server.h"
#include "server.h"

RedisSession::RedisSession(int fd, const string& ip, int port, Server *server)
  : Session(fd, Address(ip, port), server), parser_(this) {
}

RedisSession::~RedisSession() {
}

int RedisSession::Handle(int mask) {
  Debugf("%s mask %d", String(), mask);
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
  //waiting_commands_.push_back(cmd);
  RedisServer *server = CreateServer(Address("127.0.0.1", 6379), this);
  server->addQueryCommand(cmd);
}

int RedisSession::handleRead() {
  int ret;

  ret = TcpRead(fd_, query_buf_);

  if (ret < 0) {
    return ret;
  }

  Infof("query from %s %s", address_.String(), query_buf_->Start());
  parser_.Parse();
  return kOk;
}

int RedisSession::handleWrite() {
  return kOk;
}
