CC = gcc
CFLAGS = -Wall -g -Iinclude
SRC_DIR = src
OBJ_DIR = build
BIN_DIR = bin
DATA_DIR = data/fichiers
SUBFOLDER ?= 1_Sakhir

all: $(BIN_DIR)/main

$(BIN_DIR)/main: $(OBJ_DIR)/main.o $(OBJ_DIR)/car.o $(OBJ_DIR)/utils.o $(OBJ_DIR)/display.o $(OBJ_DIR)/file_manager.o $(OBJ_DIR)/simulate.o $(OBJ_DIR)/algorithm.o
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ_DIR)/main.o: $(SRC_DIR)/main.c include/car.h include/utils.h include/display.h include/file_manager.h include/algorithm.h include/simulate.h
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

$(OBJ_DIR)/simulate.o: $(SRC_DIR)/simulate.c include/simulate.h
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/algorithm.o: $(SRC_DIR)/algorithm.c include/algorithm.h
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

## commandes de bases

clean_run: clean run_sessions # supprimer tous les dossiers et lancer une simulation classique

clean_run_spe : clean run_sessions_special # supprimer tous les dossier et lance une simulation de sprint

run_sessions: $(BIN_DIR)/main # lance une simulation. Sakhir par défaut. Pour simuler une autre journée, ajouter SUBFOLDER=x_ville
	./$(BIN_DIR)/main $(DATA_DIR)/$(SUBFOLDER)/essai_1.csv
	./$(BIN_DIR)/main $(DATA_DIR)/$(SUBFOLDER)/essai_2.csv
	./$(BIN_DIR)/main $(DATA_DIR)/$(SUBFOLDER)/essai_3.csv
	./$(BIN_DIR)/main $(DATA_DIR)/$(SUBFOLDER)/qualif_1.csv
	./$(BIN_DIR)/main $(DATA_DIR)/$(SUBFOLDER)/qualif_2.csv
	./$(BIN_DIR)/main $(DATA_DIR)/$(SUBFOLDER)/qualif_3.csv
	./$(BIN_DIR)/main $(DATA_DIR)/$(SUBFOLDER)/course_1.csv

run_sessions_special: $(BIN_DIR)/main # lance une simulation pour un week de sprint. Même principe pour SUBFOLDER que run_session
	./$(BIN_DIR)/main $(DATA_DIR)/$(SUBFOLDER)/essai_1.csv
	./$(BIN_DIR)/main $(DATA_DIR)/$(SUBFOLDER)/shootout_1.csv
	./$(BIN_DIR)/main $(DATA_DIR)/$(SUBFOLDER)/shootout_2.csv
	./$(BIN_DIR)/main $(DATA_DIR)/$(SUBFOLDER)/shootout_3.csv
	./$(BIN_DIR)/main $(DATA_DIR)/$(SUBFOLDER)/sprint_1.csv
	./$(BIN_DIR)/main $(DATA_DIR)/$(SUBFOLDER)/qualif_1.csv
	./$(BIN_DIR)/main $(DATA_DIR)/$(SUBFOLDER)/qualif_2.csv
	./$(BIN_DIR)/main $(DATA_DIR)/$(SUBFOLDER)/qualif_3.csv
	./$(BIN_DIR)/main $(DATA_DIR)/$(SUBFOLDER)/course_1.csv

## commandes spécifiques pour les tests

clean:
	rm -rf $(OBJ_DIR)/*.o $(BIN_DIR)/main $(DATA_DIR)/*/
	rm -f data/gestion_points.csv

## commandes pour lancer le championnat
run_championnat: clean 
	$(MAKE) run_sessions
	$(MAKE) run_sessions SUBFOLDER=2_Djeddah
	$(MAKE) run_sessions SUBFOLDER=3_Melbourne
	$(MAKE) run_sessions_special SUBFOLDER=4_Bakou
	$(MAKE) run_sessions SUBFOLDER=5_Miami
	$(MAKE) run_sessions SUBFOLDER=6_Monte-Carlo
	$(MAKE) run_sessions SUBFOLDER=7_Barcelone
	$(MAKE) run_sessions SUBFOLDER=8_Montréal
	$(MAKE) run_sessions_special SUBFOLDER=9_Spielberg
	$(MAKE) run_sessions SUBFOLDER=10_Silverstone
	$(MAKE) run_sessions SUBFOLDER=11_Budapest
	$(MAKE) run_sessions_special SUBFOLDER=12_Spa-Francorchamps
	$(MAKE) run_sessions SUBFOLDER=13_Zandvoort
	$(MAKE) run_sessions SUBFOLDER=14_Monza
	$(MAKE) run_sessions SUBFOLDER=15_Marina_Bay
	$(MAKE) run_sessions SUBFOLDER=16_Suzuka
	$(MAKE) run_sessions_special SUBFOLDER=17_Losail
	$(MAKE) run_sessions_special SUBFOLDER=18_Austin
	$(MAKE) run_sessions SUBFOLDER=19_Mexico
	$(MAKE) run_sessions_special SUBFOLDER=20_Sao_Paulo
	$(MAKE) run_sessions SUBFOLDER=21_Las_Vegas
	$(MAKE) run_sessions SUBFOLDER=22_Yas_Marina