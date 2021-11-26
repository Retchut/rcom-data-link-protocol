CC = gcc
CFLAGS = -c -Wall

all: ll.o
	$(CC) ll.o write.c -o write
	$(CC) ll.o recv.c -o recv

ll.o: ll.c
	$(CC) $(CFLAGS) ll.c -o ll.o
	
clean:
	-rm -f *.o recv write