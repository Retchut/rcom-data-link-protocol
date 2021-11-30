CC = gcc
CFLAGS = -c -Wall

all: ll.o appl.o
	$(CC) ll.o appl.o send.c -o send
	$(CC) ll.o appl.o recv.c -o recv

ll.o: ll.c
	$(CC) $(CFLAGS) ll.c -o ll.o

appl.o: appl.c
	$(CC) $(CFLAGS) appl.c -o appl.o
	
clean:
	-rm -f *.o recv write
