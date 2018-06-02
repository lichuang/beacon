#ifndef __DISPATCHER_H__
#define __DISPATCHER_H__

#include <sys/time.h>
#include <string>
#include "typedef.h"

using namespace std;
class Engine;

class IDispatcher {
public:
  IDispatcher(Engine *engine, const string& name)
    : engine_(engine),
      name_(name) {
  }
  virtual ~IDispatcher() {}

  const string& String() { return name_; }

  virtual int Init(int setsize)         = 0;
  virtual int Add(int fd, int mask)     = 0;
  virtual int Del(int fd, int delmask)  = 0;
  virtual int Poll(struct timeval *tvp) = 0;

protected:
  Engine *engine_;
  string name_;
};

#endif // __DISPATCHER_H__
