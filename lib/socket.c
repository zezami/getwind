/*
  Copyright (C) MOXA Inc. All rights reserved.
  This software is distributed under the terms of the
  MOXA License.  See the file COPYING-MOXA for details.
*/

/*---------------------------------------------------------------------------*/
/**
  @file		socket.c
  @brief	Socket API define file

  TCP socket utility functions, it provides simple functions that helps
  to build TCP client/server.


  History:
  Date		Author			Comment
  08-01-2005	AceLan Kao.		Create it.

  @author AceLan Kao.(acelan_kao@moxa.com.tw)
 */
/*---------------------------------------------------------------------------*/

#include "socket.h"

/*---------------------------------------------------------------------------*/
/**
  @brief	initialize TCP server
  @param	port		port number for socket
  @param	serverfd	server socket fd
  @return	return server socked fd for success, on error return error code
 */
/*---------------------------------------------------------------------------*/
int	TCPServerInit( int port, int *serverfd)
{
	struct sockaddr_in dest;

	/// create socket , same as client
	*serverfd = socket(PF_INET, SOCK_STREAM, 0);

	/// initialize structure dest
	bzero((void*)&dest, sizeof(dest));
	dest.sin_family = PF_INET;
	dest.sin_port = htons( port);

	dest.sin_addr.s_addr = INADDR_ANY;

	/// Assign a port number to socket
	bind( *serverfd, (struct sockaddr*)&dest, sizeof(dest));

	return *serverfd;
}

/*---------------------------------------------------------------------------*/
/**
  @brief	wait client connect
  @param	serverfd	server socket fd
  @param	clientfd	client socket fd
  @param	clientaddr	client address which connect to server
  @return	return client fd, on error return error code
 */
/*---------------------------------------------------------------------------*/
int	TCPServerWaitConnection( int serverfd, int *clientfd, char *clientaddr)
{
	struct sockaddr_in client_addr;
	socklen_t addrlen = sizeof(client_addr);

	/// make it listen to socket
	listen( serverfd, 20);

	/// Wait and Accept connection
	*clientfd = accept(serverfd, (struct sockaddr*)&client_addr, &addrlen);

	strcpy( clientaddr, (const char *)( inet_ntoa( client_addr.sin_addr)));

	return *clientfd;
}

/*---------------------------------------------------------------------------*/
/**
  @brief	wait client connect
  @param	serverfdlist	server socket fd list
  @param	num		number of server fd
  @param	clientfd	client socket fd
  @param	clientaddr	client address which connect to server
  @return	return server fd, on error return error code
 */
/*---------------------------------------------------------------------------*/
int	TCPServerSelect( int* serverfdlist, int num, int *clientfd, char *clientaddr)
{
	struct sockaddr_in client_addr;
	fd_set rfds;
	int i, maxfd, readfds;

	socklen_t addrlen = sizeof(client_addr);

	/// make it listen to socket
	for( i= 0; i< num; i++)
		listen( serverfdlist[ i], 20);

	while( 1)
	{
		FD_ZERO( &rfds);

		for( i= 0, maxfd= -1; i< num; i++)
		{
			FD_SET( serverfdlist[ i], &rfds);
			if( serverfdlist[ i] > maxfd)
				maxfd= serverfdlist[ i];
		}

		if( select( maxfd+ 1, &rfds, NULL, NULL, NULL) < 0)
			continue;
		
		for( i= 0; i< num; i++)
			if( FD_ISSET( serverfdlist[ i], &rfds))
				break;
		if( i == num)
			continue;

		break;
	}

	/// Wait and Accept connection
	*clientfd = accept( serverfdlist[ i], (struct sockaddr*)&client_addr, &addrlen);

	strcpy( clientaddr, (const char *)( inet_ntoa( client_addr.sin_addr)));

	/// Bind port error
	if( *clientfd < 0)
		return -1;
	
	return serverfdlist[ i];
}

/*---------------------------------------------------------------------------*/
/**
  @brief	initialize TCP client
  @param	clientfd	client socket fd
  @return	return client socked fd for success, on error return error code
 */
/*---------------------------------------------------------------------------*/
int	TCPClientInit( int *clientfd)
{
	*clientfd = socket(PF_INET, SOCK_STREAM, 0);

	return *clientfd;
}

/*---------------------------------------------------------------------------*/
/**
  @brief	connect to TCP server
  @param	clientfd	client socket fd
  @param	addr		server address
  @param	port		server port number
  @return	return zero for success, on error -1 is returned
 */
/*---------------------------------------------------------------------------*/
int	TCPClientConnect( const int clientfd, const char *addr, int port)
{
	struct sockaddr_in dest;

	/// initialize value in dest
	bzero(&dest, sizeof(dest));
	dest.sin_family = PF_INET;
	dest.sin_port = htons( port);
	inet_aton( addr, &dest.sin_addr);

	/// Connecting to server
	return connect(clientfd, (struct sockaddr*)&dest, sizeof(dest));
}

/*---------------------------------------------------------------------------*/
/**
  @brief	non-block read from TCP socket
  @param	clientfd	socket fd
  @param	buf		input buffer
  @param	size		buffer size
  @return	the length of read data
 */
/*---------------------------------------------------------------------------*/
int	TCPNonBlockRead( int clientfd, char* buf, int size)
{
	int opts;
	opts = fcntl(clientfd, F_GETFL);
	opts = (opts | O_NONBLOCK);
	fcntl(clientfd, F_SETFL, opts);

	return recv( clientfd, buf, size, 0);
}

/*---------------------------------------------------------------------------*/
/**
  @brief	block read from TCP socket
  @param	clientfd	socket fd
  @param	buf		input buffer
  @param	size		buffer size
  @return	the length of read data
 */
/*---------------------------------------------------------------------------*/
int	TCPBlockRead( int clientfd, char* buf, int size)
{
	int opts;
	opts = fcntl(clientfd, F_GETFL);
	opts = (opts & ~O_NONBLOCK);
	fcntl(clientfd, F_SETFL, opts);

	return recv( clientfd, buf, size, 0);
}

/*---------------------------------------------------------------------------*/
/**
  @brief	write to TCP socket
  @param	clientfd	socket fd
  @param	buf		output buffer
  @param	size		output string length
  @return	the length of the actual written data, -1: disconnected
 */
/*---------------------------------------------------------------------------*/
int	TCPWrite( int clientfd, char* buf, int size)
{
	int len= 0;
	len= send( clientfd, buf, size, MSG_NOSIGNAL);

	return len;
}

/*---------------------------------------------------------------------------*/
/**
  @brief	close the client socket
  @param	sockfd		client socket fd
  @return	none
 */
/*---------------------------------------------------------------------------*/
void	TCPClientClose( int sockfd)
{
	close( sockfd);
}

/*---------------------------------------------------------------------------*/
/**
  @brief	close the server socket
  @param	sockfd		server socket fd
  @return	none
 */
/*---------------------------------------------------------------------------*/
void	TCPServerClose( int sockfd)
{
	close( sockfd);
}

