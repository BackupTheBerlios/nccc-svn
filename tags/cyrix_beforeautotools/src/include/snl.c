/*
	The SNL (Simple Network Layer) provides a neat C API for network programming.
	Copyright (C) 2001, 2002 Clemens Kirchgatterer <clemens@thf.ath.cx>

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <errno.h>       // errno, EINTR
#include <stdio.h>       // sprintf()
#include <fcntl.h>       // F_GETFL, F_SETFL, fcntl()
#include <unistd.h>      // close(), read(), write(), getpid()
#include <stdlib.h>      // malloc(), free()
#include <pthread.h>     // pthread_*()
#include <sys/types.h>   // pid_t
#include <sys/socket.h>  // socket(), bind(), listen(), accept(), shutdown()
#include <sys/un.h>      // struct sockaddr_un
#include <sys/uio.h>     // writev()
#include <netdb.h>       // gethostbyname()
#include <netinet/tcp.h> // TCP_NODELAY
#include <netinet/in.h>  // struct sockaddr_in
#include <arpa/inet.h>   // htons(), htonl(), ntohl()

#include "snl.h"

#define SA  struct sockaddr
#define SAI struct sockaddr_in
#define SAU struct sockaddr_un

typedef struct ListenParam {
	unsigned short port;          // port to listen on
	char *name;                   // snl name for UNIX DOMAIN SOCKET
	void *data;                   // pointer to user specific data
	SNL_LISTEN_CALLBACK(*cb);     // function to call after a connecting
} ListenParam;

typedef struct ReceiveParam {
	int  fd;                      // filedescriptor to read from
	void *data;                   // pointer to user specific data
	SNL_READ_CALLBACK(*cb);       // function to call after reading
} ReceiveParam;

static void *
thread_read(void *param)
{
	int remaining, received, err = SNL_ERR_OK, buf_len = 64;
	unsigned long int data_len;
	ReceiveParam *rp;
	char *buf, *ptr;

	rp = (ReceiveParam *)param;
	// allocate buffer for received data
	if (!(buf = malloc(buf_len))) {
		err = SNL_ERR_BUFFER;
		goto close_socket;
	}
	// we repeat until the connection has been closed
	while (TRUE) {
		// read length of next datagram
		ptr = (char *)&data_len;
		remaining = sizeof (data_len);
		while (remaining) {
			received = read(rp->fd, ptr, remaining);
			if (received == 0) {
				err = SNL_ERR_CLOSED;
				goto close_socket;
			} else if (received < 0) {
				if (errno == EINTR) continue;
				err = SNL_ERR_READ;
				goto close_socket;
			} else {
				ptr += received;
				remaining -= received;
			}
		}
		// convert it back to host byte order
		data_len = ntohl(data_len);
		// increase buffer size if nessecery
		if (data_len > buf_len) {
			if (!(buf = realloc(buf, data_len))) {
				err = SNL_ERR_BUFFER;
				goto close_socket;
			}
			buf_len = data_len;
		}
		// read datagram
		ptr = (char *)buf;
		remaining = data_len;
		while (remaining) {
			received = read(rp->fd, ptr, remaining);
			if (received == 0) {
				err = SNL_ERR_CLOSED;
				goto close_socket;
			} else if (received < 0) {
				if (errno == EINTR) continue; 
				err = SNL_ERR_READ;
				goto close_socket;
			} else {
				ptr += received;
				remaining -= received;
			}
		}
		rp->cb(rp->fd, err, (void *)buf, data_len, rp->data);
	}
	close_socket:
	shutdown(rp->fd, SHUT_RDWR);
	close(rp->fd);
	if (buf) free(buf);
	rp->cb(rp->fd, err, NULL, 0, rp->data);
	return (NULL);
}

static void *
thread_listen(void *param)
{
	int err, flags, listen_fd, client_fd, sa_len, ph_len;
	void *server, *client;
	ListenParam *lp;

	lp = (ListenParam *)param;
	sa_len = lp->port ? sizeof (SAI) : sizeof (SAU);
	server = malloc(sa_len);
	memset(server, 0, sa_len);
	client = malloc(sa_len);
	memset(client, 0, sa_len);
	if ((listen_fd = socket(lp->port ? AF_INET : AF_UNIX, SOCK_STREAM, 0)) < 0) {
		err = SNL_ERR_OPEN;
		goto close_socket;
	}
	if ((flags = fcntl(listen_fd, F_GETFL, 0)) >= 0) {
		flags |= SO_REUSEADDR;
		fcntl(listen_fd, F_SETFL, flags);
	}
	if (lp->port) {
		((SAI *)server)->sin_family = AF_INET;
		((SAI *)server)->sin_port = htons((int)lp->port);
		((SAI *)server)->sin_addr.s_addr = htonl(INADDR_ANY);
	} else {
		((SAU *)server)->sun_family = AF_UNIX;
		ph_len = sizeof (((SAU *)server)->sun_path)-1;
		strncpy(((SAU *)server)->sun_path, lp->name, ph_len);
		unlink(lp->name);
	}
	if (bind(listen_fd, (SA *)server, sa_len)) {
		err = SNL_ERR_BIND;
		goto close_socket;
	}
	if (listen(listen_fd, 3)) { // backlog = 3 XXX config via snl_init() ?
		err = SNL_ERR_LISTEN;
		goto close_socket;
	}
	while (TRUE) {
		err = SNL_ERR_OK;
		if ((client_fd = accept(listen_fd, (SA *)client, &sa_len)) < 0) {
			if (errno == EINTR) continue;
			err = SNL_ERR_ACCEPT;
		}
		lp->cb(lp->port, err, client_fd, lp->data);
	}
	close_socket:
	if (!lp->port) unlink(lp->name);
	if (lp->name)  free(lp->name);
	if (server)    free(server);
	if (client)    free(client);
	lp->cb(lp->port, err, 0, lp->data);
	return (NULL);
}

int
snl_read(int fd, SNL_READ_CALLBACK(*cb), void *data)
{
	pthread_t thr_id;
	ReceiveParam *rp;

	if (!(rp = (ReceiveParam *)malloc(sizeof (ReceiveParam)))) {
		return (SNL_ERR_BUFFER);
	}
	rp->fd   = fd;
	rp->cb   = cb;
	rp->data = data;
	if (pthread_create(&thr_id, NULL, &thread_read, (void *)rp)) {
		return (SNL_ERR_READ);
	}
	if (pthread_detach(thr_id)) {
		return (SNL_ERR_READ);
	}
	return (SNL_ERR_OK);
}

int
snl_write(int fd, void *buf, unsigned long int len)
{
	struct iovec vec[2];

	vec[1].iov_base = buf;
	vec[1].iov_len  = len;
	// convert packet length to net byte order
	len = htonl(len);
	vec[0].iov_base = &len;
	vec[0].iov_len  = sizeof (len);
	write_data:
	if (writev(fd, &vec[0], 2) == -1) {
		if (errno == EINTR) goto write_data;
		return (SNL_ERR_WRITE);
	}
	return (SNL_ERR_OK);
}

int
snl_listen(const char *name, unsigned short port, SNL_LISTEN_CALLBACK(*cb), void *data)
{
	pthread_t thr_id;
	ListenParam *lp;

	if (!(lp = (ListenParam *)malloc(sizeof (ListenParam)))) {
		return (SNL_ERR_BUFFER);
	}
	lp->port = port;
	lp->name = (char *)name;
	lp->cb   = cb;
	lp->data = data;
	if (pthread_create(&thr_id, NULL, &thread_listen, (void *)lp)) {
		return (SNL_ERR_LISTEN);
	}
	if (pthread_detach(thr_id)) {
		return (SNL_ERR_LISTEN);
	}
	return (SNL_ERR_OK);
}

int
snl_connect(const char *name, unsigned short port)
{
	int fd, flags, sa_len, ph_len;
	struct hostent *host = NULL;
	char addr[INET_ADDRSTRLEN];
	void *server;

	sa_len = port ? sizeof (SAI) : sizeof (SAU);
	server = malloc(sa_len);
	memset(server, 0, sa_len);
	if ((fd = socket(port ? AF_INET : AF_UNIX, SOCK_STREAM, 0)) < 0) {
		return (SNL_ERR_OPEN);
	}
	if ((flags = fcntl(fd, F_GETFL, 0)) >= 0) {
		flags |= TCP_NODELAY;
		fcntl(fd, F_SETFL, flags);
	}
	if (port) {
		((SAI *)server)->sin_family = AF_INET;
		((SAI *)server)->sin_port   = htons((int)port);
		if (!(host = gethostbyname(name))) {
			return (SNL_ERR_ADDRESS);
		}
		if (!inet_ntop(AF_INET, host->h_addr, addr, sizeof (addr))) {
			return (SNL_ERR_CONNECT);
		}
		if (!inet_pton(AF_INET, addr, &((SAI *)server)->sin_addr)) {
			return (SNL_ERR_CONNECT);
		}
	} else {
		((SAU *)server)->sun_family = AF_UNIX;
		ph_len = sizeof (((SAU *)server)->sun_path)-1;
		strncpy(((SAU *)server)->sun_path, name, ph_len);
	}
	if (connect(fd, (SA *)server, sa_len)) {
		close(fd);
		return (SNL_ERR_CONNECT);
	}
	return (fd);
}

#undef SA
#undef SAI
#undef SAU
