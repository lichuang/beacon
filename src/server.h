#ifndef __SERVER_H__
#define __SERVER_H__

#include <map>
#include "config.h"
#include "const.h"
#include "event.h"

using namespace std;
struct Config;
class  Engine;
class  Session;

typedef map<int, Session*> SessionMap;

class Server : public IEventHandler {
public:
  Server(Config *conf, Engine*);
  ~Server();

  int Run();
  virtual int Handle(int mask);

  void FreeSession(Session *);
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
