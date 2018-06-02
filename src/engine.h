#ifndef __ENGING_H__
#define __ENGING_H__

#include <list>
#include <vector>
#include <map>
#include "bitmap.h"
#include "event.h"
#include "typedef.h"

using namespace std;

typedef map<msec_t, TimeEvent*>::iterator TimeEventsIter;
class IDispatcher;

class Engine {
public:
  Engine(int setsize);
  ~Engine();

  Event* GetEvent(int fd) { return &(events_[fd]); }
  int GetSetSize() { return setsize_; }

  void Stop();

  int CreateEvent(int fd, int mask, IEventHandler *handler);
  int DeleteEvent(int fd, int mask);
  void FireEvent(int fd, int mask);

  int CreateTimeEvent(msec_t ms, ITimerHandler* handler);

  void Main();

private:
  msec_t nextTimeout();
  void processTimeEvents();

private:
  int setsize_;
  bool stop_;
  int maxfd_;
  vector<Event> events_;
  list<Event*> fired_;

  Bitmap timeid_bits_;
  msec_t current_ms_;
  map<msec_t, TimeEvent*> time_events_;
  IDispatcher *dispatcher_;
};

#endif  // __ENGING_H__
