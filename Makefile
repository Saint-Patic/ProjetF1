CC = gcc
CFLAGS = -Wall -g

all: main

main: main.o car.o utils.o
	$(CC) $(CFLAGS) -o main main.o car.o utils.o

main.o: main.c car.h utils.h
	$(CC) $(CFLAGS) -c main.c

car.o: car.c car.h utils.h
	$(CC) $(CFLAGS) -c car.c

utils.o: utils.c utils.h
	$(CC) $(CFLAGS) -c utils.c

clean:
	rm -f *.o main
