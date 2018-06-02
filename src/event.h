#ifndef __event_h__
#define __event_h__

#include <stdio.h>
#include "typedef.h"

const int kEventNone  = 0;
const int kEventRead  = 1;
const int kEventWrite = 2;

struct IEventHandler {
public:
  IEventHandler() {}
  virtual ~IEventHandler() {}

  virtual int Handle(int mask) = 0;
};

struct Event {
public:
  Event()
    : mask_(kEventNone),
      handler_(NULL),
      data_(NULL) {}

public:
  int mask_;
  IEventHandler *handler_;
};

struct ITimerHandler {
public:
  ITimerHandler() {}
  virtual ~ITimerHandler() {}

  virtual int Handle() = 0;
};

struct TimeEvent {
public:
  TimeEvent(int id, msec_t ms, ITimerHandler* handler)
    : id_(id),
      msec_(ms),
      handler_(handler),
      next_(NULL) {}

  ~TimeEvent() {}

public:
  int id_;
  msec_t msec_;
  ITimerHandler *handler_;
  // link same msec time events in a list
  TimeEvent *next_;
};

#endif // __event_h__
