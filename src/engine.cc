#include <climits>
#include <list>
#include "engine.h"
#include "epoll_dispatcher.h"
#include "errcode.h"
#include "util.h"

Engine::Engine(int setsize)
  : setsize_(setsize),
    stop_(true),
    maxfd_(-1),
    timeid_bits_(INT_MAX),
    current_ms_(0) {
  events_.resize(setsize);
  dispatcher_ = new Epoll(this);
  dispatcher_->Init(setsize);
}

Engine::~Engine() {
  delete dispatcher_;
}

void Engine::Stop() {
  stop_ = true;
}

int Engine::CreateEvent(int fd, int mask, IEventHandler *handler) {
  Event *event;
  int ret;

  if (fd >= setsize_) {
    return kFdOutOfRange;
  }

  ret = dispatcher_->Add(fd, mask);
  if (ret != kOk) {
    return ret;
  }
  event = GetEvent(fd);
  event->handler_ = handler;
  if (fd > maxfd_) {
    maxfd_ = fd;
  }

  return kOk;
}

int Engine::DeleteEvent(int fd, int mask) {
  if (fd >= setsize_) {
    return kFdOutOfRange;
  }
  Event *event = GetEvent(fd);
  if (event->mask_ == kEventNone) {
    return kOk;
  }
  int ret = dispatcher_->Del(fd, mask);
  if (ret != kOk) {
    return ret;
  }
  event->mask_ = event->mask_ & (~mask);
  // update max fd
  if (fd == maxfd_ && event->mask_ == kEventNone) {
    int i;
    for (i = maxfd_ - 1; i >= 0; i--) {
      if (GetEvent(i)->mask_ != kEventNone) break;
    }
    maxfd_ = i;
  }
  return kOk;
}

int Engine::CreateTimeEvent(msec_t ms, ITimerHandler* handler) {
  TimeEventsIter iter;
  int id;
  
  id = timeid_bits_.Next();
  if (id == -1) {
    return kNoTimeId;
  }
  ms += current_ms_;

  iter = time_events_.find(ms);
  if (iter != time_events_.end()) {
    iter->second->next_ = new TimeEvent(id, ms, handler);
  } else {
    time_events_[ms] = new TimeEvent(id, ms, handler);
  }
  return id;
}

msec_t Engine::nextTimeout() {
  if (time_events_.empty()) {
    return 0;
  }
  return time_events_.begin()->second->msec_;
}

void Engine::FireEvent(int fd, int mask) {
  Event *event = GetEvent(fd);
  event->mask_ = mask;
  fired_.push_back(event);
}

void Engine::processTimeEvents() {
  TimeEventsIter iter;
  TimeEvent *event, *tmp, *save;
  list<msec_t> outtimes;
  list<msec_t>::iterator timeiter;

  if (time_events_.empty()) {
    return;
  }
  current_ms_ = GetCurrentMs();
  
  iter = time_events_.begin();
  while (true) {
    event = iter->second;
    if (event->msec_ > current_ms_) {
      break;
    }
    event->handler_->Handle();
    save = event;
    outtimes.push_back(event->msec_);
    while (event->next_) {
      tmp = event->next_;
      tmp->handler_->Handle();
      event = tmp->next_;
      delete tmp;
    }
    delete save;
  }

  for (timeiter = outtimes.begin(); timeiter != outtimes.end(); ++timeiter) {
    time_events_.erase(*timeiter);
  }
}

void Engine::Main() {
  struct timeval tv;
  int ret;
  list<Event*>::iterator iter;

  stop_ = false;
  while (!stop_) {
    current_ms_ = GetCurrentMs();

    // 1. find next timeout msec
    msec_t msec = nextTimeout();

    // 2. poll io events
    if (msec > 0) {
      msec -= current_ms_;
    }
    if (msec < 0) {
      // TODO?
      continue;
    }
    if (msec == 0) {
      tv.tv_sec  = 0;
      tv.tv_usec = 5000;
    } else {
      tv.tv_sec  = msec / 1000;
      tv.tv_usec = (msec % 1000) * 1000;
    }
    dispatcher_->Poll(&tv);

    // 3. fire events
    for (iter = fired_.begin(); iter != fired_.end(); ++iter) {
      ret = (*iter)->handler_->Handle((*iter)->mask_);
      (*iter)->mask_ = kEventNone;
      if (ret != kOk) {
        dispatcher_->Del();
      }
    }

    // 4. process timer events
    processTimeEvents();

    // 5. clear fired events
    fired_.clear();
  }
}
