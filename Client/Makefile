CC = gcc
CFLAGS = -Wall

all: CL drive

CL: CL.c
	$(CC) $(CFLAGS) CL.c -o CL

drive: drive.c
	$(CC) $(CFLAGS) drive.c -o drive

clean:
	rm -f CL drive
