#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "config.h"
#include "common.h"
#include "errcode.h"
#include "engine.h"
#include "event.h"
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

  Infof("Server listening at 0.0.0:%d", config_->port_);
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
    fd = TcpAccept(fd_, &cip, &cport, errstr_);
    if (fd < 0) {
      if (errno != EWOULDBLOCK) {
        Errorf("accepting client connection: %s", errstr_);
      }
      return kOk;
    }
    SetNonBlock(fd, NULL);
    session = config_->factory_->CreateSession(fd, cip, cport, this);
    if (session == NULL) {
      Errorf("new session fail, close connection from %s:%d", cip.c_str(), cport);
      close(fd);
      return kOk;
    }
    Infof("accept %s for fd %d", session->String(), fd);
    engine_->CreateEvent(fd, kEventRead, session);
    session_map_[fd] = session;
  }
  return kOk;
}

void Server::Listen() {
  fd_ = CreateTcpServer(config_->port_, NULL, config_->backlog_, errstr_);
  if (fd_ < 0) {
    Fatalf("CreateTcpServer fail:%s", errstr_);
  }
  SetNonBlock(fd_, errstr_);
}

void Server::FreeSession(Session *session) {
  int fd = session->Fd();
  Event *event = engine_->GetEvent(fd);

  if (event == NULL) {
    Errorf("no event for fd %d", fd);
    return;
  }

  Debugf("close connection from %s", session->String());

  engine_->DeleteEvent(fd, event->mask_);
  session_map_.erase(fd);
  delete session;
  close(fd);
}
