#ifndef __NET_H__
#define __NET_H__

#include <string>

using namespace std;

int CreateTcpServer(int port, const char* addr, int backlog, char* err);
int TcpAccept(int sfd, string *cip, int *port, char* err);
int SetNonBlock(int fd, char *err);

#endif // __NET_H__
