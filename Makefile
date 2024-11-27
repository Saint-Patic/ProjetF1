CC = gcc
CFLAGS = -Wall -g -Iinclude
SRC_DIR = src
OBJ_DIR = build
BIN_DIR = bin
DATA_DIR = data/fichiers
SUBFOLDER ?= 1_Sakhir
SUBFOLDER_2 ?= 2_Djeddah

all: $(BIN_DIR)/main

$(BIN_DIR)/main: $(OBJ_DIR)/main.o $(OBJ_DIR)/car.o $(OBJ_DIR)/utils.o $(OBJ_DIR)/display.o $(OBJ_DIR)/file_manager.o
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ_DIR)/main.o: $(SRC_DIR)/main.c include/car.h include/utils.h include/display.h include/file_manager.h
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/car.o: $(SRC_DIR)/car.c include/car.h include/utils.h
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/utils.o: $(SRC_DIR)/utils.c include/utils.h
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/display.o: $(SRC_DIR)/display.c include/display.h
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/file_manager.o: $(SRC_DIR)/file_manager.c include/file_manager.h
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

first_session: clean $(BIN_DIR)/main
	./$(BIN_DIR)/main $(DATA_DIR)/$(SUBFOLDER)/essai_1.csv

seconde_session: $(BIN_DIR)/main
	./$(BIN_DIR)/main $(DATA_DIR)/$(SUBFOLDER)/essai_2.csv

djeddah: $(BIN_DIR)/main
	./$(BIN_DIR)/main $(DATA_DIR)/$(SUBFOLDER_2)/essai_1.csv

djeddah_complet: clean $(BIN_DIR)/main
	./$(BIN_DIR)/main $(DATA_DIR)/$(SUBFOLDER)/essai_1.csv
	./$(BIN_DIR)/main $(DATA_DIR)/$(SUBFOLDER)/essai_2.csv
	./$(BIN_DIR)/main $(DATA_DIR)/$(SUBFOLDER)/essai_3.csv
	./$(BIN_DIR)/main $(DATA_DIR)/$(SUBFOLDER)/qualif_1.csv
	./$(BIN_DIR)/main $(DATA_DIR)/$(SUBFOLDER)/qualif_2.csv
	./$(BIN_DIR)/main $(DATA_DIR)/$(SUBFOLDER)/qualif_3.csv
	./$(BIN_DIR)/main $(DATA_DIR)/$(SUBFOLDER)/course_1.csv
	./$(BIN_DIR)/main $(DATA_DIR)/$(SUBFOLDER_2)/essai_1.csv

run_sessions: $(BIN_DIR)/main
	./$(BIN_DIR)/main $(DATA_DIR)/$(SUBFOLDER)/essai_1.csv
	./$(BIN_DIR)/main $(DATA_DIR)/$(SUBFOLDER)/essai_2.csv
	./$(BIN_DIR)/main $(DATA_DIR)/$(SUBFOLDER)/essai_3.csv
	./$(BIN_DIR)/main $(DATA_DIR)/$(SUBFOLDER)/qualif_1.csv
	./$(BIN_DIR)/main $(DATA_DIR)/$(SUBFOLDER)/qualif_2.csv
	./$(BIN_DIR)/main $(DATA_DIR)/$(SUBFOLDER)/qualif_3.csv
	./$(BIN_DIR)/main $(DATA_DIR)/$(SUBFOLDER)/course_1.csv

run_sessions_special: $(BIN_DIR)/main
	./$(BIN_DIR)/main $(DATA_DIR)/$(SUBFOLDER)/essai_1.csv
	./$(BIN_DIR)/main $(DATA_DIR)/$(SUBFOLDER)/shootout_1.csv
	./$(BIN_DIR)/main $(DATA_DIR)/$(SUBFOLDER)/shootout_2.csv
	./$(BIN_DIR)/main $(DATA_DIR)/$(SUBFOLDER)/shootout_3.csv
	./$(BIN_DIR)/main $(DATA_DIR)/$(SUBFOLDER)/sprint_1.csv
	./$(BIN_DIR)/main $(DATA_DIR)/$(SUBFOLDER)/qualif_1.csv
	./$(BIN_DIR)/main $(DATA_DIR)/$(SUBFOLDER)/qualif_2.csv
	./$(BIN_DIR)/main $(DATA_DIR)/$(SUBFOLDER)/qualif_3.csv
	./$(BIN_DIR)/main $(DATA_DIR)/$(SUBFOLDER)/course_1.csv

clean:
	rm -rf $(OBJ_DIR)/*.o $(BIN_DIR)/main $(DATA_DIR)/*/
	rm -f data/gestion_points.csv

clean_dirs:
	rm -rf $(DATA_DIR)/*/

clean_files:
	rm -f $(DATA_DIR)/fichiers/*

clean_o:
	rm -rf $(OBJ_DIR) $(BIN_DIR)/main
