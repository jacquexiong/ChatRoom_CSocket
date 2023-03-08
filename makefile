#
#
#  Makefile for client and server examples
#
#

# Edit the following line to reflect the current project number
PROJNUM = 1

HDR = speak.h speakd.h
SRC = speak.c speakd.c unixclient.c unixserver.c

OBJ = unixclient.o unixserver.o speakd.o speak.o
GRD = makefile ${SRC} ${HDR}

CC=gcc
CFLAGS = -g0

# Create all files

all:	speak speakd

speak:	unixclient.o speak.o
	$(CC)	$(CFLAGS) unixclient.o speak.o -o speak

speakd:	unixserver.o speakd.o
	$(CC)	$(CFLAGS) unixserver.o speakd.o -o speakd

unixclient.o:	unixclient.c speak.h
	$(CC)	$(CFLAGS) -c unixclient.c
	
speak.o:	speak.c speak.h
	$(CC)	$(CFLAGS) -c speak.c

unixserver.o:	unixserver.c speakd.h
	$(CC)	$(CFLAGS) -c unixserver.c

speakd.o:	speakd.c speakd.h
	$(CC)	$(CFLAGS) -c speakd.c

#
# Clean up script
#
clean:
	/bin/rm -f *.o speak speakd

