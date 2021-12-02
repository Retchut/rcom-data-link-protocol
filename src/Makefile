CC = gcc
CFLAGS = -c -Wall

all: ll.o send.o read.o state.o config.o utils.o
	$(CC) ll.o send.o read.o state.o config.o utils.o main.c -o main

ll.o: ll.c
	$(CC) $(CFLAGS) ll.c -o ll.o

appl.o: appl.c
	$(CC) $(CFLAGS) appl.c -o appl.o

send.o: send.c
	$(CC) $(CFLAGS) send.c -o send.o

read.o: read.c
	$(CC) $(CFLAGS) read.c -o read.o

state.o: state.c
	$(CC) $(CFLAGS) state.c -o state.o

config.o: config.c
	$(CC) $(CFLAGS) config.c -o config.o

utils.o: utils.c
	$(CC) $(CFLAGS) utils.c -o utils.o

clean: # -rm -f *.o recv write
	-rm -f *.o recv write main
