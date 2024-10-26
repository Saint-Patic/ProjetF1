CC = gcc
CFLAGS = -Wall -g

all: main

main: main.o car.o utils.o display.o file_manager.o
	$(CC) $(CFLAGS) -o main main.o car.o utils.o display.o file_manager.o

main.o: main.c car.h utils.h display.h file_manager.h
	$(CC) $(CFLAGS) -c main.c

car.o: car.c car.h utils.h
	$(CC) $(CFLAGS) -c car.c

utils.o: utils.c utils.h
	$(CC) $(CFLAGS) -c utils.c

display.o: display.c display.h
	$(CC) $(CFLAGS) -c display.c

file_manager.o: file_manager.c file_manager.h
	$(CC) $(CFLAGS) -c file_manager.c

clean:
	rm -f *.o main
