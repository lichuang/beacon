#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <string>

using namespace std;

class SessionFactory;

struct Config {
  int port_;

  int backlog_;

  SessionFactory *factory_;
};

#endif // __CONFIG_H__
