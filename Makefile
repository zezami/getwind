CC=arm-linux-gcc
LIBRARY=../library
CFLAGS=-I$(LIBRARY)
CXXFLAGS=
OBJS1=getwind.o serial.o socket.o

all:	getwind

getwind: $(OBJS1)
	$(CC) $(OBJS1) -o $@

serial.o: $(LIBRARY)/serial.c
	$(CC) -c $(LIBRARY)/serial.c

socket.o: $(LIBRARY)/socket.c
	$(CC) -c $(LIBRARY)/socket.c

clean:
	rm *.o getwind > /dev/null 2>&1

