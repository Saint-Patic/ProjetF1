#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include "car.h"
#include "display.h"
#include "file_manager.h"
#include "utils.h"

#define NUM_CARS 20
#define MIN_TIME 25
#define MAX_TIME 45
#define SESSION_DISTANCE 300

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <session_filename>\n", argv[0]);
        printf("Format paramètre attendu: fichiers/<x_ville>/<type>_<numéro>.csv\n");
        return 1;
    }

    char *session_file = argv[1];
    char *ville = malloc(50 * sizeof(char));
    char *session_type = malloc(20 * sizeof(char));
    int session_num;

    if (!verifier_parametres(session_file, ville, session_type, &session_num)) {
        return 1;
    }

    srand(time(NULL));
    int session_duration = 3600;
    car_t cars[NUM_CARS];
    int car_numbers[NUM_CARS] = {1, 11, 44, 63, 16, 55, 4, 81, 14, 18, 10, 31, 23, 2, 22, 3, 77, 24, 20, 27};

    for (int i = 0; i < NUM_CARS; i++) {
        cars[i].car_number = car_numbers[i];
        cars[i].best_lap_time = 0;
        cars[i].temps_rouler = 0;
        cars[i].pit_stop = 0;
        cars[i].pit_stop_nb = 1;
        cars[i].out = 0;
        for (int j = 0; j < NUM_SECTORS; j++) {
            cars[i].sector_times[j] = 0;
            cars[i].best_sector_times[j] = 0;
        }
    }

    printf("===== Début de la session: %s =====\n\n", session_file);
    create_directories_from_csv_values("liste_circuits.csv", "Course", "Ville");
    if (strcmp(session_type, "qualif") == 0) {
        simulate_qualification(cars, session_num, ville, MIN_TIME, MAX_TIME, NUM_CARS);
    } else if (strcmp(session_type, "essai") == 0) {
        int total_laps = estimate_max_laps(session_duration, (float)3 * MIN_TIME) + 1;
        simulate_sess(cars, NUM_CARS, MIN_TIME, MAX_TIME, session_duration, total_laps);
        save_session_results(cars, NUM_CARS, session_file, "w");
    } else if (strcmp(session_type, "course") == 0) {
        int nb_resultats;
        char **circuit_distance = recuperer_colonne_csv("liste_circuits.csv", "taille (km)", &nb_resultats);
        int total_laps = estimate_max_laps(SESSION_DISTANCE, atof(circuit_distance[atoi(ville) - 1])) + 1;
        simulate_course(SESSION_DISTANCE, MIN_TIME, MAX_TIME, total_laps);
    }

    process_session_files(session_num, ville, session_type);
    free(ville);
    free(session_type);
    return 0;
}
