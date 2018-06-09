#include "redis_command.h"
#include "redis_server.h"
#include "redis_session.h"
#include "const.h"
#include "engine.h"
#include "errcode.h"
#include "log.h"
#include "net.h"

RedisServer::RedisServer(const Address& address, RedisSession *session)
  : Session(-1, address, session->GetServer()),
    current_cmd_(NULL),
    session_(session),
    status_(kDisconnected),
    query_buf_(new Buffer(kQueryBufferLen)){ 
}

RedisServer::~RedisServer() {
  delete query_buf_;
}

void RedisServer::addQueryCommand(RedisCommand* cmd) {
  query_cmds_.push_back(cmd);   
}

bool RedisServer::Connect() {
  fd_ = CreateTcpSocket();
  if (fd_ < 0) {
    return false;
  }
  SetNonBlock(fd_, NULL);
  int ret = TcpConnect(fd_, address_.ip_.c_str(), address_.port_);
  switch (ret) {
  case kOk:
    status_ = kConnected;
    break;
  case kError:
    status_ = kDisconnected;
    break;
  case kInProgress:
    status_ = kConnecting;
    break;
  }
  if (status_ != kDisconnected) {
    if (session_->GetEngine()->AddEvent(fd_, kEventRead | kEventWrite, this) != kOk) {
      Errorf("add server %s event error", address_.String());
    }
  }
  return status_ != kDisconnected;
}

int RedisServer::Handle(int mask) {
  if ((mask & kEventWrite) && handleWrite() != kOk) {
    return kError;
  }

  if ((mask & kEventRead) && handleRead() != kOk) {
    return kError;
  }

  return kOk;
}

int RedisServer::handleWrite() {
  if (status_ == kConnecting) {
    status_ = kConnected;
  }
  if (status_ != kConnected) {
    Errorf("connect to %s error", address_.String());
    return kError;
  }

  if (query_cmds_.empty() && current_cmd_ == NULL) {
    return kOk;
  }
  if (current_cmd_ == NULL) {
    current_cmd_ = query_cmds_.front();
    query_cmds_.pop_front();
  }
  
  while (current_cmd_ != NULL) {
    BufferPos *current = current_cmd_->NextBufferPos();
    while (current != NULL && current->buffer_ != NULL) {
      int ret = TcpSend(fd_, current);
      if (ret < 0) {
        return kError;
      }
      if (current->Done()) {
        current = current_cmd_->NextBufferPos();
      } else {
        return kOk;
      }
    }
    waiting_cmds_.push_back(current_cmd_);
    current_cmd_ = NULL;
    if (query_cmds_.empty()) {
      break;
    }
    current_cmd_ = query_cmds_.front();
    query_cmds_.pop_front();
  }

  return kOk;
}

int RedisServer::handleRead() {
  TcpRead(fd_, query_buf_);
  Infof("response from %s %s", address_.String(), query_buf_->Start());
  /*
  while (!waiting_cmds_.empty()) {
    RedisCommand *cmd = waiting_cmds_.front(); 
  }
  */
  return kOk;
}

RedisServer* CreateServer(const Address& address, RedisSession *session) {
  RedisServer *server = new RedisServer(address, session);
  if (!server) {
    return NULL;
  }
  if (!server->Connect()) {
    delete server;
    return NULL;
  }

  return server;
}
