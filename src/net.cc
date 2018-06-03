#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "errcode.h"
#include "const.h"
#include "net.h"
#include "typedef.h"

static void setNetError(char *err, const char *fmt, ...) {
	va_list ap;

	if (!err) return;
	va_start(ap, fmt);
	vsnprintf(err, kNetErrorLen, fmt, ap);
	va_end(ap);
}

static int setReuseAddr(char *err, int fd) {
	int yes = 1;
	/* Make sure connection-intensive things like the redis benckmark
	 * will be able to close/open sockets a zillion of times */
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
		setNetError(err, "setsockopt SO_REUSEADDR: %s", strerror(errno));
		return kError;
	}
	return kOk;
}

static int netListen(char *err, int s, struct sockaddr *sa, socklen_t len, int backlog) {
	if (bind(s,sa,len) == -1) {
		setNetError(err, "bind: %s", strerror(errno));
		close(s);
		return kError;
	}

	if (listen(s, backlog) == -1) {
		setNetError(err, "listen: %s", strerror(errno));
		close(s);
		return kError;
	}
	return kOk;
}

int CreateTcpServer(int port, const char *addr, int backlog, char *err) {
	int s = -1, rv;
	char _port[6];  /* strlen("65535") */
	struct addrinfo hints, *servinfo, *p;

	snprintf(_port,6,"%d",port);
	memset(&hints,0,sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;    /* No effect if bindaddr != NULL */

	if ((rv = getaddrinfo(addr,_port,&hints,&servinfo)) != 0) {
		setNetError(err, "%s", gai_strerror(rv));
		return kError;
	}
	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((s = socket(p->ai_family,p->ai_socktype,p->ai_protocol)) == -1) {
			continue;
		}
		//if (af == AF_INET6 && anetV6Only(err,s) == ANET_ERR) goto error;
		if (setReuseAddr(err,s) == kError) goto error;
		if (netListen(err,s,p->ai_addr,p->ai_addrlen,backlog) == kError) s = kError;
		goto end;
	}
	if (p == NULL) {
		setNetError(err, "unable to bind socket, errno: %d", errno);
		goto error;
	}

error:
	if (s != -1) close(s);
	s = kError;
end:
	freeaddrinfo(servinfo);
	return s;
}

static int Accept(char *err, int s, struct sockaddr *sa, socklen_t *len) {
	int fd;
	while(true) {
		fd = accept(s,sa,len);
		if (fd == -1) {
			if (errno == EINTR)
				continue;
			else {
				setNetError(err, "accept: %s", strerror(errno));
				return kError;
			}
		}
		break;
	}
	return fd;
}

int TcpAccept(int sfd, string *cip, int *port, char *err) {
	int fd;
	struct sockaddr_storage sa;
	socklen_t salen = sizeof(sa);
	char ip[30];
	if ((fd = Accept(err,sfd,(struct sockaddr*)&sa,&salen)) == -1) {
		return kError;
	}

	if (sa.ss_family == AF_INET) {
		struct sockaddr_in *s = (struct sockaddr_in *)&sa;
		inet_ntop(AF_INET,(void*)&(s->sin_addr),ip,sizeof(ip));
		*port = ntohs(s->sin_port);
		*cip = ip;
	} else {
/*
		struct sockaddr_in6 *s = (struct sockaddr_in6 *)&sa;
		if (ip) inet_ntop(AF_INET6,(void*)&(s->sin6_addr),ip,ip_len);
		if (port) *port = ntohs(s->sin6_port);
*/
	}

	return fd;
}

int setBlock(int fd, bool non_block, char *err) {
	int flags;

	/* Set the socket blocking (if non_block is zero) or non-blocking.
	 * Note that fcntl(2) for F_GETFL and F_SETFL can't be
	 * interrupted by a signal. */
	if ((flags = fcntl(fd, F_GETFL)) == -1) {
		setNetError(err, "fcntl(F_GETFL): %s", strerror(errno));
		return kError;
	}

	if (non_block)
		flags |= O_NONBLOCK;
	else
		flags &= ~O_NONBLOCK;

	if (fcntl(fd, F_SETFL, flags) == -1) {
		setNetError(err, "fcntl(F_SETFL,O_NONBLOCK): %s", strerror(errno));
		return kError;
	}
	return kOk;
}

int SetNonBlock(int fd, char *err) {
	return setBlock(fd, true, err);
}
