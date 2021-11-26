CC = gcc
CFLAGS = -c -Wall

all: ll.o
	$(CC) ll.o send.c -o send
	$(CC) ll.o recv.c -o recv

ll.o: ll.c
	$(CC) $(CFLAGS) ll.c -o ll.o
	
clean:
	-rm -f *.o recv write
