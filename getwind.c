/*
  Copyright (C) MOXA Inc. All rights reserved.
  This software is distributed under the terms of the
  MOXA License.  See the file COPYING-MOXA for details.
*/

/*---------------------------------------------------------------------------*/
/**
  @file		getwind.c
  @brief	connect to serial port of ultimeter 100 wind station and get wind data

  History:
  Date		Author			Comment
  2011-05-11	Johan Bernhardtson	First version

  @author Johan Bernhardtson (johan@deepcore.se)
 */
/*---------------------------------------------------------------------------*/

#include <stdio.h>
#include "serial.h"

#define SPEED_OFFSET 0
#define DIRECTION_OFFSET 4
#define TEMPERATURE_OFFSET 8
#define AVGSPEED_OFFSET 44

#define POST_URL "http://lillis.plakat.se/newData.aspx"
#define POST_USER "johan"
#define POST_PASSWORD "blaj"

int get_value(unsigned char* buf, int offset)
{
	int i, ret=0;
	for(i=0; i<4; i++) {
		if(buf[offset+i] > 0x39) {
			printf("%x", buf[offset+i]-55);
			ret = ret | (buf[offset+i] - 55) << ((3-i)*4);
		}
		else {
			printf("%x", buf[offset+i]-48);
			ret = ret | (buf[offset+i] - 48) << ((3-i)*4);
		}
	}
	printf(" ret = %d\n", ret);
	return ret;	
}

void get_data_header(int fd)
{
	unsigned char buf[1];
	do {
                SerialBlockRead(fd, buf, 1);
        } while(buf[0] != '!');
	return;
}

int main()
{
	unsigned char buf[255];
	int i, len,ret, fd=PORT3, direction;
	float speed, temperature, avgspeed;
	
	printf("Opening serial port...");
	ret = SerialOpen(fd);
	if(ret < 0) {
		printf("Error: SerialOpen returned: %d\n", ret);
		SerialClose(fd);
		return -1;
	}
	printf("Done\n");	

	printf("Setting port speed...");
	ret = SerialSetSpeed(fd, 2400);
	if(ret < 0) {
		printf("Error: SerialSetSpeed returned: %d\n", ret);
		SerialClose(fd);
		return -1;
	}
	printf("Done\n");

	printf("Setting port parameters...");
	ret = SerialSetParam(fd, 0, 8, 1);
	if(ret < 0) {
		printf("Error: SerialSetParam returned: %d\n", ret);
		SerialClose(fd);
		return -1;	
	}
	printf("Done\n");	

	printf("Setting port flow control...");
	ret = SerialFlowControl(fd, NO_FLOW_CONTROL);
	if(ret < 0) {
		printf("Error: SerialFlowControl returned: %d\n", ret);
		SerialClose(fd);
		return -1;
	}
	printf("Done\n");

	printf("Setting data logger mode...");
	ret = SerialWrite(fd, ">I\r", 3); // data logger mode
	if(ret < 0) {
		printf("Error: SerialWrite returned: %d\n", ret);
		SerialClose(fd);
		return -1;
	}
	printf("Done\n");
	

	printf("Waiting for header...");
	get_data_header(fd);
	get_data_header(fd);
	printf("Found\n");

	printf("Waiting for data...");
	ret = SerialBlockRead(fd, buf, 50); // read data
	if(ret < 0) {
		printf("Error: SerialBlockRead returned: %d\n", ret);
		SerialClose(fd);
		return -1;
	}

	printf("Found\n");		
       	for(i=0;i<48;i++) {
                printf("%c\n", buf[i]);
	}
	
	if((buf[48] != 13) && (buf[49] != 10)) {
		printf("Error: End of data incorrect!\n");
		SerialClose(fd);
		return -1;
	}	

	//printf("end= %d %d\n", buf[48], buf[49]);

	ret = SerialWrite(fd, ">\r", 2);
	SerialClose(fd);

	speed = (float)get_value(buf, SPEED_OFFSET);
	speed = speed / 10;
	speed = speed / 3.6; //(speed * 1.60934) / 3.6;
	direction = get_value(buf, DIRECTION_OFFSET) & 0x00FFFFFF; // highest byte may be FF sometimes
	direction = (int)((360.0/255.0) * direction);
	temperature = (float)get_value(buf, TEMPERATURE_OFFSET);
	temperature = temperature / 10;
	temperature = (temperature - 32) * 5 / 9;
	avgspeed = (float)get_value(buf, AVGSPEED_OFFSET);
	avgspeed = avgspeed / 10;
	avgspeed = avgspeed / 3.6;

	printf("speed=%.1f, direction=%d, temperature=%.1f, avg.speed=%.1f\n", speed, direction, temperature, avgspeed);
	sprintf(buf, "wget --spider \"%s?speed=%.1f&dir=%d&temp=%.1f&avgspeed=%.1f\"\n", POST_URL, speed, direction, temperature, avgspeed);

	system(buf);
	
	return 0;
}
