#include <string.h>
#include <unistd.h>
#include "config.h"
#include "common.h"
#include "engine.h"
#include "log.h"
#include "net.h"
#include "server.h"
#include "session.h"

Server::Server(Config *conf, Engine *engine)
  : config_(conf),
    engine_(engine) {
}

Server::~Server() {
}

int Server::Run() {
  int ret;

  Listen();
  ret = engine_->CreateEvent(fd_, kEventRead, this);
  if (ret < 0) {
    Fatalf("create listen event error:%d", ret);
  }

  engine_->Main();

  return kOk;
}

int Server::Handle(int mask) {
  int max = kMaxAcceptPerCall;
  int fd, cport;
  Session *session;
  string cip;

  UNUSED(mask);

  while (max--) {
    fd = TcpAccept(fd_, &cip, &cport, &errstr_);
    if (fd < 0) {
      if (errno != EWOULDBLOCK) {
        Errorf("accepting client connection: %s", errstr_.c_str());
        return kOk;
      }
      Infof("accpted %s:%d", cip.c_str(), cport);
      session = config_->factory_->CreateSession(fd, cip, port);
      if (session == NULL) {
        Errorf("new session fail, close connection from %s:%d", cip.c_str(), cport);
        close(fd);
        return;
      }
      engine_->CreateEvent(fd, kEventRead, session);
      session_map_[fd] = session;
    }
  }
  return kOk;
}

void Server::Listen() {
  fd_ = CreateTcpServer(port, NULL, config_->backlog_, &errstr_);
  if (fd_ < 0) {
    Fatalf("CreateTcpServer fail:%s", strerr.c_str());
  }
  SetNonBlock(fd_);
}
