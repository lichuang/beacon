#ifndef __EPOLL_DISPATCHER_H__
#define __EPOLL_DISPATCHER_H__

#include <sys/epoll.h>
#include "dispatcher.h"

class Epoll : public IDispatcher {
public:
  Epoll();
  virtual ~Epoll();

  virtual int Init(int setsize);
  vitual int Add(int fd, int mask);
  virtual int Del(int fd, int delmask);
  virtual int Poll(struct timeval *tvp);

private:
  int efd_;
  struct epoll_event *events_;
  int set_size_;
};

#endif // __EPOLL_DISPATCHER_H__
