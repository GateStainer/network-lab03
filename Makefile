CC=gcc
client: main.o
	$(CC) -o client main.o
main.o: main.c word.h tcp.h
	$(CC) -c main.c

clean:
	rm -f client
	rm -f *.o
	rm -f *~