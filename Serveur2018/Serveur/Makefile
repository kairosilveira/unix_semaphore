# Fichier Makefile

CC = gcc
CFLAGS = -g -Wall -ansi -D _GNU_SOURCE 
CFLAG2 = -c -Wall -ansi -D _GNU_SOURCE 
OBJETS = SV_main.o SV_msg.o SV_mem.o SV_connect.o SV_init.o SV_data.o SV_sem.o

all: sv SV

sv : $(OBJETS) SV.c
	$(CC) $(CFLAGS) $(OBJETS) -o sv_zz
	$(CC) $(CFLAGS) SV.c -o SV

SV_main.o : SV_main.c SV_include
	$(CC) $(CFLAG2) SV_main.c -o SV_main.o

SV_msg.o : SV_msg.c SV_include SV_msg.h
	$(CC) $(CFLAG2) SV_msg.c -o SV_msg.o

SV_mem.o : SV_mem.c SV_include SV_mem.h
	$(CC) $(CFLAG2) SV_mem.c -o SV_mem.o

SV_connect.o : SV_connect.c SV_include SV_connect.h
	$(CC) $(CFLAG2) SV_connect.c -o SV_connect.o

SV_init.o : SV_init.c SV_include SV_init.h
	$(CC) $(CFLAG2) SV_init.c -o SV_init.o

SV_data.o : SV_data.c SV_include SV_data.h
	$(CC) $(CFLAG2) SV_data.c -o SV_data.o

SV_sem.o : SV_sem.c SV_include SV_sem.h
	$(CC) $(CFLAG2) SV_sem.c -o SV_sem.o

clean:
	@rm -f *~ *.o sv_zz SV 




