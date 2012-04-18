/*
  Copyright (C) MOXA Inc. All rights reserved.
  This software is distributed under the terms of the
  MOXA License.  See the file COPYING-MOXA for details.
*/

/*---------------------------------------------------------------------------*/
/**
  @file		serial.c
  @brief	Serial API define file

  Serial utility functions, it helps programmer easily operating serial port.
  It provides mostly standard functions we usually use, and SerialSetMode()
  is the private function can be use in UC box.

  History:
  Date		Author			Comment
  08-01-2005	AceLan Kao.		Create it.
  10-06-2005	AceLan Kao.		Add SerialDataInOutputQueue()
  10-06-2005	AceLan Kao.		Modify SerialOpen() prototype

  @author AceLan Kao.(acelan_kao@moxa.com.tw)
 */
/*---------------------------------------------------------------------------*/

#include "serial.h"

static struct	termios oldtio[ MAX_PORT_NUM], newtio[ MAX_PORT_NUM];
static int	fd_map[ MAX_PORT_NUM]={ -1, -1, -1, -1, -1, -1, -1, -1};///< -1 means SERIAL_ERROR_FD

/*---------------------------------------------------------------------------*/
/**
  @brief	use port number to find out the fd of the port
  @param	port	port number
  @return	fd which is opened by using SerialOpen()
 */
/*---------------------------------------------------------------------------*/
int	FindFD( int port)
{
	if( fd_map[ port] == -1)
		return SERIAL_ERROR_FD;

	return fd_map[ port];
}

/*---------------------------------------------------------------------------*/
/**
  @brief	open serial port
  @param	port		port number
  @return	return fd for success, on error return error code
 */
/*---------------------------------------------------------------------------*/
int	SerialOpen( int port)
{
	int fd;
	char device[ 80];
	struct termios tio;

	if( fd_map[ port] != -1)		///< port already opened
		return SERIAL_ERROR_OPEN;

	sprintf( device, "/dev/ttyM%d", port);
	fd = open( device, O_RDWR|O_NOCTTY);
	if( fd <0)
		return SERIAL_ERROR_OPEN;

	fd_map[ port]= fd;

	bzero( &tio, sizeof(tio));		///< clear struct for new port settings

	tio.c_iflag = 0;
	tio.c_oflag = 0;
	tio.c_cflag = B9600|CS8|CREAD|CLOCAL;
	tio.c_lflag = 0;

	tio.c_cc[ VTIME] = 0;			///< inter-character timer unused
	tio.c_cc[ VMIN] = 1;			///< blocking read until 1 character arrives

	tcgetattr( fd, &(oldtio[ port]));	///< save current serial port settings
	newtio[ port]= tio;

	tcflush( fd, TCIFLUSH);
	tcsetattr( fd, TCSANOW, &newtio[ port]);

	return fd;
}

/*---------------------------------------------------------------------------*/
/**
  @brief	write to serial port
  @param	port		port number
  @param	str		string to write
  @param	len		length of str
  @return	return length of str for success, on error return error code
 */
/*---------------------------------------------------------------------------*/
int	SerialWrite( int port, char* str, int len)
{
	int fd= FindFD( port);

	if( fd < 0)			///< error
		return fd;

	return write( fd, str, len);
}

/*---------------------------------------------------------------------------*/
/**
  @brief	non-block read from serial port
  @param	port		port number
  @param	buf		input buffer
  @param	len		buffer length
  @return	return length of read str for success,
  		on error return error code
 */
/*---------------------------------------------------------------------------*/
int	SerialNonBlockRead( int port, char* buf, int len)
{
	int res= 0;
	int bytes= 0;
	int fd= FindFD( port);

	if( fd < 0)			///< error
		return fd;

	fcntl( fd, F_SETFL, FNDELAY);
	res = read( fd, buf, len);
	return res;
}

/*---------------------------------------------------------------------------*/
/**
  @brief	block read from serial port
  @param	port		port number
  @param	buf		input buffer
  @param	len		buffer length
  @return	return length of read str for success,
  		on error return error code
 */
/*---------------------------------------------------------------------------*/
int	SerialBlockRead( int port, char* buf, int len)
{
	int res= 0;
	int bytes= 0;
	int fd= FindFD( port);

	if( fd < 0)			///< error
		return fd;

	fcntl( fd, F_SETFL, 0);
	res = read( fd, buf, len);
	return res;
}

/*---------------------------------------------------------------------------*/
/**
  @brief	close serial port
  @param	port		port number
  @return	return SERIAL_OK for success, on error return error code
 */
/*---------------------------------------------------------------------------*/
int	SerialClose( int port)
{
	int fd= FindFD( port);

	if( fd < 0)			///< error
		return fd;

	tcsetattr( fd, TCSANOW, &(oldtio[ port]));
	close( fd);

	fd_map[ port]= -1;

	return SERIAL_OK;
}

/*---------------------------------------------------------------------------*/
/**
  @brief	test how much data in input queue
  @param	port		port number
  @return	return number of data to be read for success,
  		on error return error code
 */
/*---------------------------------------------------------------------------*/
int	SerialDataInInputQueue( int port)
{
	int bytes= 0;
	int fd= FindFD( port);

	if( fd < 0)			///< error
		return fd;

	ioctl( fd, FIONREAD, &bytes);
	return bytes;
}

/*---------------------------------------------------------------------------*/
/**
  @brief	test how much data in output queue
  @param	port		port number
  @return	return number of data to be write for success,
  		on error return error code
 */
/*---------------------------------------------------------------------------*/
int	SerialDataInOutputQueue( int port)
{
	int bytes= 0;
	int fd= FindFD( port);

	if( fd < 0)			///< error
		return fd;

	ioctl( fd, TIOCOUTQ, &bytes);
	return bytes;
}
/*---------------------------------------------------------------------------*/
/**
  @brief	set flow control
  @param	port		port number
  @param	control		NO_FLOW_CONTROL/HW_FLOW_CONTROL/SW_FLOW_CONTROL
  @return	return SERIAL_OK for success, on error return error code
 */
/*---------------------------------------------------------------------------*/
int	SerialFlowControl( int port, int control)
{
	int fd= FindFD( port);

	if( fd < 0)			///< error
		return fd;

	if( control == NO_FLOW_CONTROL)
	{
		newtio[ port].c_cflag &= ~CRTSCTS;
		newtio[ port].c_iflag &= ~(IXON | IXOFF | IXANY);
	}
	else if( control == HW_FLOW_CONTROL)
		newtio[ port].c_cflag |= CRTSCTS;
	else if( control == SW_FLOW_CONTROL)
		newtio[ port].c_iflag |= (IXON | IXOFF | IXANY);
	else
		return SERIAL_PARAMETER_ERROR;

	tcflush( fd, TCIFLUSH);
	tcsetattr( fd, TCSANOW, &newtio[ port]);

	return SERIAL_OK;
}

/*---------------------------------------------------------------------------*/
/**
  @brief	set serial speed and make changes now
  @param	port		port number
  @param	speed		unsigned integer for new speed
  @return	return SERIAL_OK for success, on error return error code
 */
/*---------------------------------------------------------------------------*/
int	SerialSetSpeed( int port, unsigned int speed)
{
	int i, table_size= 23;
	int speed_table1[]={ 0, 50, 75, 110, 134, 150, 200, 300,
			     600, 1200, 1800, 2400, 4800, 9600,
			     19200, 38400, 57600, 115200, 230400,
			     460800, 500000, 576000, 921600};
	int speed_table2[]={ B0, B50, B75, B110, B134, B150, B200, B300,
			     B600, B1200, B1800, B2400, B4800, B9600,
			     B19200, B38400, B57600, B115200, B230400,
			     B460800, B500000, B576000, B921600};
	int fd= FindFD( port);

	if( fd < 0)			///< error
		return fd;

	for( i= 1; i< table_size; i++)	///< i start from 1, bellow 50 will be set to B0
		if( speed_table1[ i] >= speed)
			break;

	cfsetispeed( &newtio[ port], speed_table2[ i]);
	cfsetospeed( &newtio[ port], speed_table2[ i]);
	tcsetattr( fd, TCSANOW, &newtio[ port]);

	return SERIAL_OK;
}

/*---------------------------------------------------------------------------*/
/**
  @brief	set serial port mode for RS232/RS422/RS485
  @param	port		port number
  @param	mode		serial port mode
  		{RS232_MODE/RS485_2WIRE_MODE/RS422_MODE/RS485_4WIRE_MODE}
  @return	return SERIAL_OK for success, on error return error code
 */
/*---------------------------------------------------------------------------*/
int     SerialSetMode( int port, unsigned int mode)
{
        char device[ 80];
	int ret= 0, fd= FindFD( port);
	if( fd < 0)			///< error
	{
		sprintf( device, "/dev/ttyM%d", port);
		fd = open( device, O_RDWR|O_NOCTTY);
		if( fd <0)
	                return SERIAL_ERROR_OPEN;
	}

	ret= ioctl( fd, MOXA_SET_OP_MODE, &mode);
	if( FindFD( port) < 0)
		close( fd);

	return ret;
}

/*---------------------------------------------------------------------------*/
/**
  @brief	get serial port mode
  @param	port		port number
  @return	serial port mode
  		{RS232_MODE/RS485_2WIRE_MODE/RS422_MODE/RS485_4WIRE_MODE}
 */
/*---------------------------------------------------------------------------*/
int     SerialGetMode( int port)
{
        char device[ 80];
	int mode, ret= 0, fd= FindFD( port);

	if( fd < 0)			///< error
	{
		sprintf( device, "/dev/ttyM%d", port);
		fd = open( device, O_RDWR|O_NOCTTY);
		if( fd <0)
	                return SERIAL_ERROR_OPEN;
	}

	ret= ioctl( fd, MOXA_GET_OP_MODE, &mode);
	if( FindFD( port) < 0)
		close( fd);

	if( ret < 0)
		return ret;
	return mode;
}

/*---------------------------------------------------------------------------*/
/**
  @brief	set serial port parameter
  @param	port		port number
  @param	parity		parity check, 0: none, 1: odd, 2: even, 3: space, 4: mark
  @param	databits	data bits
  @param	stopbit		stop bit
  @return	return fd for success, on error return error code
 */
/*---------------------------------------------------------------------------*/
int	SerialSetParam( int port, int parity, int databits, int stopbit)
{
	int fd= FindFD( port);
	if( fd < 0)			///< error
		return fd;

	if( parity == 0)
	{
		newtio[ port].c_cflag &= ~PARENB;
		newtio[ port].c_iflag &= ~INPCK;
	}
	else if( parity == 1)
	{
		newtio[ port].c_cflag |= PARENB;
		newtio[ port].c_cflag |= PARODD;
		newtio[ port].c_iflag |= INPCK;
	}
	else if( parity == 2)
	{
		newtio[ port].c_cflag |= PARENB;
		newtio[ port].c_cflag &= ~PARODD;
	}
	else if( parity == 3)
	{
		newtio[ port].c_cflag &= ~PARENB;
		newtio[ port].c_cflag &= ~CSTOPB;
	}
	else if( parity == 4)
	{
		newtio[ port].c_cflag |= CSTOPB;
		newtio[ port].c_cflag &= ~PARENB;
		newtio[ port].c_iflag &= ~INPCK;
	}

	if( databits == 5)
		newtio[ port].c_cflag |= CS5;
	else if( databits == 6)
		newtio[ port].c_cflag |= CS6;
	else if( databits == 7)
		newtio[ port].c_cflag |= CS7;
	else if( databits == 8)
		newtio[ port].c_cflag |= CS8;

	if( stopbit == 1)
		newtio[ port].c_cflag &= ~CSTOPB;
	else if( stopbit == 2)
		newtio[ port].c_cflag |= CSTOPB;

	tcflush( fd, TCIFLUSH);
	tcsetattr( fd, TCSANOW, &newtio[ port]);

	return SERIAL_OK;
}
