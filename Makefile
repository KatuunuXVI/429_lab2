CC	 	= gcc
LD	 	= gcc
CFLAGS	 	= -Wall -g

LDFLAGS	 	= 
DEFS 	 	=

all:	sendfile recvfile

source/sendfile: sendfile.c
	$(CC) $(DEFS) $(CFLAGS) $(LIB) -o source/sendfile sendfile.c

dest/recvfile: recvfile.c
	$(CC) $(DEFS) $(CFLAGS) $(LIB) -o dest/recvfile recvfile.c


clean:
	rm -f *.o
	rm -f *~
	rm -f core.*.*
	rm -f source/sendfile
	rm -f recvfile
	