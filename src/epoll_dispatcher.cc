#include <sys/time.h>
#include "epoll_dispatcher.h"
#include "errcode.h"
#include "engine.h"

Epoll::Epoll(Engine *engine)
  : IDispatcher(engine, "epoll") {
}

Epoll::~Epoll() {
  delete [] events_;
  close(efd_);
}

int Epoll::Init(int setsize) {
  efd_ = epoll_create(1024);
  if (efd_ == -1) {
    return kError;
  }
  events_ = new struct epoll_event[setsize];
  set_size_ = setsize;
  return kOk;
}

int Epoll::Add(int fd, int mask) {
  struct epoll_event ee = {0};
  Event *event = engine_->GetEvent(fd);

  int op = event->mask_ == kEventNone ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;
  
  ee.events = 0;
  mask |= event->mask_;
  if (mask & kEventRead) ee.events = EPOLLIN;
  if (mask & kEventRead) ee.events = EPOLLOUT;
  ee.data.fd = fd;
  if (epoll_ctl(epd_, op, fd, &ee) == -1)  {
    return kError;
  }
  return kOk;
}

int Epoll::Del(int fd, int delmask) {
  struct epoll_event ee = {0};
  Event *event = engine_->GetEvent(fd);
  int mask = event->mask_ & (~delmask);
  int ret;

  ee.events = 0;
  mask |= event->mask_;
  if (mask & kEventRead) ee.events = EPOLLIN;
  if (mask & kEventRead) ee.events = EPOLLOUT;
  ee.data.fd = fd;
  if (mask != kEventNone) {
    ret = epoll_ctl(efd_, EPOLL_CTL_MOD, fd, &ee);
  } else {
    ret = epoll_ctl(efd_, EPOLL_CTL_DEL, fd, &ee);
  }
  if (ret < 0) {
    return kError;
  }
  return kOk;
}

int Epoll::Poll(struct timeval *tvp) {
  int numevents = 0, ret;
  int i, mask;
  struct epoll_event *ev;

  ret = epoll_wait(efd_, events_, set_size_,
    tvp ? (tvp->tv_sec*1000 + tvp->tv_usec/1000) : -1);
  if (ret > 0) {
    numevents = ret;
    for (i = 0; i < numevents; ++i) {
      mask = 0;
      ev = events_[i];

      if (ev->events & EPOLLIN)   mask |= kEventRead;
      if (ev->events & EPOLLOUT)  mask |= kEventWrite;
      if (ev->events & EPOLLERR)  mask |= kEventWrite;
      if (ev->events & EPOLLHUP)  mask |= kEventWrite;
      engine_->FireEvent(fd, mask);
    }
  }

  return numevents;
}
