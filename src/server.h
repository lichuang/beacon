#ifndef __SERVER_H__
#define __SERVER_H__

#include "config.h"
#include "const.h"
#include "event.h"

struct Config;
class  Engine;
class  Session;

typedef map<int, Session*> SessionMap;

class Server : public IEventHandler {
public:
  Server(Config *conf);
  ~Server();

  int Run(Config* conf);
  virtual int Handle(int mask);

  void FreeSession(int fd);
private:
  void Listen();

private:
  int fd_;
  Config *config_;
  Engine *engine_;
  char errstr_[kNetErrorLen];
  SessionMap session_map_;  
};

#endif // __SERVER_H__
