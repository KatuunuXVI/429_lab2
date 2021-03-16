CC	 	= gcc
LD	 	= gcc
CFLAGS	 	= -Wall -g

LDFLAGS	 	= 
DEFS 	 	=

all:	sendfile recvfile

sendfile: sendfile.c
	$(CC) $(DEFS) $(CFLAGS) $(LIB) -o sendfile sendfile.c

recvfile: recvfile.c
	$(CC) $(DEFS) $(CFLAGS) $(LIB) -o recvfile recvfile.c

oldsend: oldsend.c
	$(CC) $(DEFS) $(CFLAGS) $(LIB) -o oldsend oldsend.c


clean:
	rm -f *.o
	rm -f *~
	rm -f core.*.*
	rm -f sendfile
	rm -f recvfile
	