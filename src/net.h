#ifndef __NET_H__
#define __NET_H__

#include <stdio.h>
#include <string>
#include "const.h"
#include "errcode.h"

using namespace std;

class Buffer;
struct BufferPos;

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

  Address& operator=(const Address &address) {
    if (&address != this) {
      this->ip_ = address.ip_;
      this->port_ = address.port_;
    }

    return *this;
  }

  bool operator< (const Address &address) {
    if (address.ip_ != ip_) {
      return ip_ < address.ip_;
    }

    return port_ < address.port_;
  }
};

int CreateTcpServer(int port, const char* addr, int backlog, char* err);
int CreateTcpSocket();
int TcpAccept(int sfd, string *cip, int *port, char* err);
int SetNonBlock(int fd, char *err);
errno_t TcpRead(int fd, Buffer* buf);
int TcpSend(int fd, BufferPos* bufpos);
int TcpConnect(int fd, const char *addr, int port);

#endif // __NET_H__
