#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <stdio.h>
#include <string>
#include "redis_session.h"

using namespace std;

class SessionFactory;

struct Config {
  int port_;

  int backlog_;

  int setsize_;

  SessionFactory *factory_;

  Config()
    : port_(22345),
      backlog_(1024),
      setsize_(10240),
      factory_(NULL) {}
};

#endif // __CONFIG_H__
