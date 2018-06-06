#ifndef __NET_H__
#define __NET_H__

#include <stdio.h>
#include <string>
#include "const.h"

using namespace std;

class Buffer;

struct Address {
  string ip_;
  int    port_;
  string str_;

  Address(const string& ip, int port)
    : ip_(ip),
      port_(port) {
    char buf[kAddressLen];
    snprintf(buf, kAddressLen, "%s:%d", ip.c_str(), port);
    str_ = buf;
  }

  const char* String() const {
    return str_.c_str();
  }
};

int CreateTcpServer(int port, const char* addr, int backlog, char* err);
int TcpAccept(int sfd, string *cip, int *port, char* err);
int SetNonBlock(int fd, char *err);
int TcpRead(int fd, Buffer* buf);

#endif // __NET_H__
