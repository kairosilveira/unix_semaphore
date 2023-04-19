CC = gcc
CFLAGS = -Wall

all: CL driver

CL: CL.c
	$(CC) $(CFLAGS) CL.c -o CL

drive: drive.c
	$(CC) $(CFLAGS) driver.c -o driver

clean:
	rm -f CL driver
