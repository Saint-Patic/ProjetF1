CC = gcc
CFLAGS = -Wall -g

# Variable pour spécifier le sous-dossier (par défaut 1_Sakhir)
SUBFOLDER ?= 1_Sakhir

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

run_sessions: main
    ./main fichiers/$(SUBFOLDER)/essai_1.csv
    ./main fichiers/$(SUBFOLDER)/essai_2.csv
    ./main fichiers/$(SUBFOLDER)/essai_3.csv
    ./main fichiers/$(SUBFOLDER)/qualif_1.csv
    ./main fichiers/$(SUBFOLDER)/qualif_2.csv
    ./main fichiers/$(SUBFOLDER)/qualif_3.csv

clean:
    rm -f *.o main
    rm -rf fichiers/*/

clean_dirs:
    rm -rf fichiers/*/

clean_files:
    rm -f fichiers/*

clean_o:
    rm -f *.o main