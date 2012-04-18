/*
  Copyright (C) MOXA Inc. All rights reserved.
  This software is distributed under the terms of the
  MOXA License.  See the file COPYING-MOXA for details.
*/

/*---------------------------------------------------------------------------*/
/**
  @file		serial.h
  @brief	Serial API header file

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

#ifndef SERIAL_H
#define SERIAL_H

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <asm/ioctls.h>

#include "moxadevice.h"

#define PORT1					0
#define PORT2					1
#define PORT3					2
#define PORT4					3
#define PORT5					4
#define PORT6					5
#define PORT7					6
#define PORT8					7

#define MAX_PORT_NUM				8

#define NO_FLOW_CONTROL				0
#define HW_FLOW_CONTROL				1
#define SW_FLOW_CONTROL				2

#define SerialRead				SerialBlockRead

#define	SERIAL_OK				0
#define SERIAL_ERROR_FD				-1	///< Could not find the fd in the map, device not opened
#define SERIAL_ERROR_OPEN			-2	///< Could not open the port or port has been opened
#define SERIAL_PARAMETER_ERROR			-3	///< Not available parameter

#define	CMSPAR					010000000000	///< mark or space (stick) parity

int	SerialOpen( int port);
int	SerialWrite( int port, char* str, int len);
int	SerialNonBlockRead( int port, char* buf, int len);
int	SerialBlockRead( int port, char* buf, int len);
int	SerialClose( int port);
int	SerialDataInInputQueue( int port);
int	SerialDataInOutputQueue( int port);
int	SerialFlowControl( int port, int control);
int	SerialSetSpeed( int port, unsigned int speed);
int	SerialSetMode( int port, unsigned int mode);
int	SerialGetMode( int port);
int	SerialSetParam( int port, int parity, int databits, int stopbit);

int	FindFD( int port);

#endif
