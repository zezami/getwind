/*
  Copyright (C) MOXA Inc. All rights reserved.
  This software is distributed under the terms of the
  MOXA License.  See the file COPYING-MOXA for details.
*/

/******************************************************************************
File Name : buzzer.c

Description :
	The program demo how to code buzzer program on UC.

Usage :
	1.Compile this file and execute on UC7000 series.
	2.Choose the number of menu, you can hear a beep sound.

History :
	Versoin		Author		Date		Comment
	1.0		Hank		01-15-2004	Wrote it
*******************************************************************************/

#include	<stdio.h>
#include	<stdlib.h>
#include	<fcntl.h>
#include	<errno.h>
#include	<termios.h>
#include	<time.h>
#include	<sys/kd.h>   // for Buzzer Function

static int buzzer_fd= 0;

int BuzzerInit()
{
	buzzer_fd= open("/dev/console", O_RDWR);
	return buzzer_fd;
}

int BuzzerSound( int time)
{
	if( buzzer_fd <= 0)
		if( BuzzerInit() < 0)
			return -1;

	ioctl( buzzer_fd, KDMKTONE, (time<<16) | 100);
}

