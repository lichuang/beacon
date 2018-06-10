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
    session_(session),
    status_(kDisconnected),
    query_buf_(new Buffer(kQueryBufferLen)),
    info_(this) { 
}

RedisServer::~RedisServer() {
  delete query_buf_;
}

void RedisServer::addQueryCommand(RedisCommand* cmd) {
  info_.AddWaitWriteCmd(cmd);   
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
  RedisCommand *cmd;

  if (status_ == kConnecting) {
    status_ = kConnected;
  }
  if (status_ != kConnected) {
    Errorf("connect to %s error", address_.String());
    return kError;
  }

  cmd = info_.NextWriteCommand();

  while (cmd != NULL) {
    BufferPos *current = cmd->NextBufferPos();
    while (current != NULL && current->buffer_ != NULL) {
      errno_t ret = TcpSend(fd_, current->buffer_);
      if (ret < 0) {
        return kError;
      }
      if (ret == kAgain) {
        return kOk;
      }
      current = cmd->NextBufferPos();
    }

    // end of write current cmd,send next cmd
    info_.AddWaitReadCmd(cmd);
    info_.ResetWriteCommand();
    cmd = info_.NextWriteCommand();
  }

  return kOk;
}

int RedisServer::handleRead() {
  RedisCommand *cmd;

  errno_t ret = TcpRead(fd_, query_buf_);
  if (ret < 0) {
    return kError;
  }
  Infof("response from %s %s", address_.String(), query_buf_->Start());

  cmd = info_.NextReadCommand();
  while (cmd != NULL && query_buf_->hasUnprocessedData()) {
    cmd = info_.GetParser()->Parse(query_buf_, cmd);

    if (cmd->GetReady()) {
      session_->AddResponseCommand(cmd);
      info_.ResetReadCommand();
      cmd = info_.NextReadCommand();
    } else if (cmd->Error()){
      return kError;
    }
    if (query_buf_->ReadableLength() == 0) {
      query_buf_ = new Buffer(kQueryBufferLen); 
    }
  }

  return kOk;
}

void RedisServer::serverEof() {
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
