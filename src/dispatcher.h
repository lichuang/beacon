#ifndef __DISPATCHER_H__
#define __DISPATCHER_H__

#include <sys/time.h>
#include <string>
#include "typedef.h"

class Engine;

class IDispatcher {
public:
  IDispatcher(Engine *engine, const string& name)
    : engine_(engine),
      name_(name) {
  }
  virtual ~IDispatcher() {}

  const string& String() { return name_; }

  virtual bool Init()                   = 0;
  virtual int Init()                    = 0;
  vitual int Add(int fd, int mask)      = 0;
  virtual int Del(int fd, int delmask)  = 0;
  virtual int Poll(struct timeval *tvp);

protected:
  string name_;
  Engine *engine_;
};

#endif // __DISPATCHER_H__
