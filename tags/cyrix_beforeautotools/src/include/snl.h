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

/**
	\file
	\author  Clemens Kirchgatterer <clemens@thf.ath.cx>
	\version V1.1.1
	\date    2002-03-30
	\brief   An user friendly multi threaded network library.

	This is a very user friendly library to hide all complicating things about
	network programming behind a neat api. It starts the threads on incoming
	connections, sends and receives data over a socket and takes care of large
	enough buffers. This way, a buffer overflow is definitly impossible to
	happen, as long as only library functions are used to read from and write
	to the socket filedescriptors.
*/

#ifndef _SNL_H_
#define _SNL_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
	\brief   Macro to shorten the listen callback function prototype.

	The SNL_LISTEN_CALLBACK() macro expands the callback function to its full
	prototype to make the sourcecode more readable.
*/
#define SNL_LISTEN_CALLBACK(CB) void (CB)(unsigned short port, int error, int fd, void *data)

/**
	\brief   Read callback function prototype macro.

	The SNL_READ_CALLBACK() macro expands the callback function to its full
	prototype to make the sourcecode more readable.
*/
#define SNL_READ_CALLBACK(CB) void (CB)(int fd, int error, void *buf, int len, void *data)

#define SNL_ERR_OK       0   ///< no error
#define SNL_ERR_OPEN    -1   ///< could'nt open socket
#define SNL_ERR_LISTEN  -2   ///< error while listening to socket
#define SNL_ERR_CONNECT -3   ///< connecting to socket failed
#define SNL_ERR_BIND    -4   ///< couldn't bind to socket
#define SNL_ERR_ACCEPT  -5   ///< error while accepting connection
#define SNL_ERR_READ    -6   ///< error while reading from socket
#define SNL_ERR_WRITE   -7   ///< failed to write to socket
#define SNL_ERR_CLOSED  -8   ///< peer closed connection
#define SNL_ERR_BUFFER  -9   ///< out of memory (buffer allocation error)
#define SNL_ERR_ADDRESS -10  ///< could not get address from hostname

/**
	\brief   Send a datagram over the socket connection
	\param   fd <int> filedescriptor for this connection
	\param   buf <void *> pointer to the data to send
	\param   len <int> length of that data
	\return  0 on success or a negative error code

	This function is used to send a datagram over the network connection.
	Thereby the library ensures, that the hole datagram will get sent at
	once and that the other side receives all data correctly.
*/
int snl_write(int fd, void *buf, unsigned long int len);

/**
	\brief   Start a seperate thread to handle exact one socket connection
	\param   fd <int> filedescriptor for this connection
	\param   cb <void (*)()> a pointer to the callback function
	\param   data <void *> user specific data
	\return  0 on success or a negative error code

	The read function registers a callback function, that will get called,
	when a complete datagram has been received. It also takes an additional
	parameter, a void pointer, that one can use for its own purpose.
*/
int snl_read(int fd, SNL_READ_CALLBACK(*cb), void *data);

/**
	\brief   Start a thread to listen for incoming connections
	\param   name <const char *> unix socket name for local connections
	\param   port <unsigned short> port number the server should listen on
	\param   cb <void (*)()> a pointer to the callback function
	\param   data <void *> user specific data
	\return  0 on success or a negative error code
	\note    if port is 0, then a UNIX DOMAIN SOCKET is used

	To start the listening thread, this function must be used. Its interface
	is similar to the one of the read function.
*/
int snl_listen(const char *name, unsigned short port, SNL_LISTEN_CALLBACK(*cb), void *data);

/**
	\brief   Connect to a listening socket
	\param   name <const char *> hostname, ip address or unix socket to connect
	\param   port <unsigned short> the port the server is listening on
	\return  a filedescriptor on success or a negative error code
	\note    if port is 0, then a UNIX DOMAIN SOCKET is used

	For connecting to a snl server, one has to call this fuction. The first
	paramter can be a hostname, an ipaddress or a unix socket name, depending
	on the second parameter. If port == 0, then name is interpreted as an
	unix socket file name (i.g. "/tmp/socket").
*/
int snl_connect(const char *name, unsigned short port);

#ifndef NULL
#define NULL ((void *)0) ///< NULL is defined in a somehow save manner
#endif

#ifndef FALSE
#define FALSE (0)        ///< FALSE is equal to 0
#endif

#ifndef TRUE
#define TRUE (!FALSE)    ///< TRUE  is not equal to FALSE
#endif

#ifdef __cplusplus
}
#endif

#endif /* _SNL_H_ */
