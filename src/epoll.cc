#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "epoll.h"
#include "errcode.h"
#include "engine.h"
#include "log.h"

Epoll::Epoll(Engine *engine)
  : IDispatcher(engine, "epoll") {
}

Epoll::~Epoll() {
  free(events_);
  close(efd_);
}

int Epoll::Init(int setsize) {
  efd_ = epoll_create(1024);
  if (efd_ == -1) {
    return kError;
  }
  events_ = (struct epoll_event*)malloc(setsize * sizeof(struct epoll_event));
  return kOk;
}

int Epoll::Add(int fd, int mask) {
  struct epoll_event ee = {0};
  Event *event = engine_->GetEvent(fd);

  int op = event->mask_ == kEventNone ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;
  
  ee.events = 0;
  mask |= event->mask_;
  if (mask & kEventRead)  ee.events |= EPOLLIN;
  if (mask & kEventWrite) ee.events |= EPOLLOUT;
  ee.data.fd = fd;
  if (epoll_ctl(efd_, op, fd, &ee) == -1)  {
    Errorf("epoll_ctl on %d fail: %s", fd, strerror(errno));
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
  if (mask & kEventRead)  ee.events |= EPOLLIN;
  if (mask & kEventWrite) ee.events |= EPOLLOUT;
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

  ret = epoll_wait(efd_, events_, engine_->GetSetSize(),
    tvp ? (tvp->tv_sec*1000 + tvp->tv_usec/1000) : -1);
  if (ret > 0) {
    numevents = ret;
    for (i = 0; i < numevents; ++i) {
      mask = kEventNone;
      ev = &(events_[i]);

      if (ev->events & EPOLLIN)   { mask |= kEventRead; Debugf("fd %d readable", ev->data.fd);}
      if (ev->events & EPOLLOUT)  mask |= kEventWrite;
      if (ev->events & EPOLLERR)  mask |= kEventWrite;
      if (ev->events & EPOLLHUP)  mask |= kEventWrite;
      engine_->FireEvent(ev->data.fd, mask);
    }
  }

  return numevents;
}
