#include "errcode.h"
#include "log.h"
#include "net.h"
#include "redis_info.h"
#include "redis_session.h"
#include "redis_server.h"
#include "server.h"

RedisSession::RedisSession(int fd, const string& ip, int port, Server *server)
  : Session(fd, Address(ip, port), server),
    info_(this) {
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

void RedisSession::addQueryCommand(RedisCommand *cmd) {
  //waiting_commands_.push_back(cmd);
  RedisServer *server = CreateServer(Address("127.0.0.1", 6379), this);
  server->addQueryCommand(cmd);
}

void RedisSession::addResponseCommand(RedisCommand *cmd) {
  info_.AddWaitWriteCmd(cmd);
}

int RedisSession::handleRead() {
  int ret;

  ret = TcpRead(fd_, query_buf_);
  if (ret < 0) {
    return ret;
  }

  // TODO: limit client process cmd number
  
  while (query_buf_->ReadableLength() > 0) {
    Infof("query from %s %s", address_.String(), query_buf_->Start());
    RedisCommand *cmd = info_.GetParser()->Parse(query_buf_, NULL);
    if (cmd->GetReady()) {
      addQueryCommand(cmd);
    } else if (cmd->Error()) {
      delete cmd;
      return false;
    }
    if (query_buf_->ReadableLength() == 0) {
      query_buf_ = new Buffer(kQueryBufferLen); 
    }
  }
  return kOk;
}

int RedisSession::handleWrite() {
  return kOk;
}
