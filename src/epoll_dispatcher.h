#ifndef __EPOLL_DISPATCHER_H__
#define __EPOLL_DISPATCHER_H__

#include <sys/epoll.h>
#include "dispatcher.h"

class Epoll : public IDispatcher {
public:
  Epoll(Engine *engine);
  virtual ~Epoll();

  virtual int Init(int setsize);
  virtual int Add(int fd, int mask);
  virtual int Del(int fd, int delmask);
  virtual int Poll(struct timeval *tvp);

private:
  int efd_;
  struct epoll_event *events_;
};

#endif // __EPOLL_DISPATCHER_H__
