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
    current_write_cmd_(NULL),
    current_read_cmd_(NULL),
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

void RedisServer::nextWriteCommand() {
  if (current_write_cmd_ != NULL) {
    return;
  }

  list<RedisCommand*> *wait_cmds = info_.GetWaitWriteCmds();
  if (wait_cmds->empty()) {
    return;
  }
  current_write_cmd_ = wait_cmds->front();
  wait_cmds->pop_front();

  // ready to write cmd 
  current_write_cmd_->ReadyWrite(); 
}

int RedisServer::handleWrite() {
  if (status_ == kConnecting) {
    status_ = kConnected;
  }
  if (status_ != kConnected) {
    Errorf("connect to %s error", address_.String());
    return kError;
  }

  nextWriteCommand();

  while (current_write_cmd_ != NULL) {
    BufferPos *current = current_write_cmd_->NextBufferPos();
    while (current != NULL && current->buffer_ != NULL) {
      errno_t ret = TcpSend(fd_, current->buffer_);
      if (ret < 0) {
        return kError;
      }
      if (ret == kAgain) {
        return kOk;
      }
      current = current_write_cmd_->NextBufferPos();
    }

    // end of write current cmd,send next cmd
    info_.AddWaitReadCmd(current_write_cmd_);
    current_write_cmd_ = NULL;
    nextWriteCommand();
  }

  return kOk;
}

int RedisServer::handleRead() {
  TcpRead(fd_, query_buf_);
  Infof("response from %s %s", address_.String(), query_buf_->Start());
  /*
  while (!waiting_cmds_.empty()) {
    RedisCommand *cmd = waiting_cmds_.front();
    // end of write current cmd,send next cmd
    info_.AddWaitReadCmd(current_write_cmd_);
    current_write_cmd_ = NULL;
    nextWriteCommand();
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
