#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>

#include "../include/car.h"
#include "../include/display.h"
#include "../include/file_manager.h"
#include "../include/utils.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <session_filename>\n", argv[0]);
        printf("Format paramètre attendu: fichiers/<x_ville>/<type>_<numéro>.csv\n");
        return 1;
    }

    char *session_file = argv[1];
    char *ville = malloc(50 * sizeof(char));
    char *session_type = malloc(20 * sizeof(char));
    int session_num, directory_num;

    if (!verifier_parametres(session_file, ville, session_type, &session_num, &directory_num)) {
        free(ville);
        free(session_type);
        return 1;
    }

    srand(time(NULL));

    car_t cars[NUM_CARS];
    int car_numbers[NUM_CARS] = {1, 11, 44, 63, 16, 55, 4, 81, 14, 18, 10, 31, 23, 2, 22, 3, 77, 24, 20, 27};
    initialize_cars(cars, car_numbers, NUM_CARS);

    printf("===== Début de la session: %s =====\n\n", session_file);
    create_directories_from_csv_values("data/liste_circuits.csv", "Course", "Ville");

    int special_weekend = is_special_weekend(ville);

    if (strcmp(session_type, "essai") == 0) {
        printf("Simulation des essais libres pour un %s.\n", special_weekend ? "week-end spécial" : "week-end normal");
        simulate_sess(cars, NUM_CARS, 3600, 20, "essai");
        save_session_results(cars, NUM_CARS, session_file, "w");
    } else if (strcmp(session_type, "qualif") == 0 || strcmp(session_type, "shootout") == 0) {
        if (special_weekend && session_num == 1) {
            printf("Simulation du Sprint Shootout (qualifications spéciales).\n");
        } else if (!special_weekend || session_num > 1) {
            printf("Simulation des qualifications pour la course principale.\n");
            simulate_qualification(cars, session_num, ville, special_weekend, session_file);
        }
    } else if (strcmp(session_type, "course") == 0) {
        int distance_course = session_num == 1 && special_weekend ? SPRINT_DISTANCE : SESSION_DISTANCE;
        int total_laps = calculate_total_laps(ville, distance_course);
        if (session_num == 1 && special_weekend) {
            printf("Simulation de la course sprint.\n");
        } else if (session_num > 1 || !special_weekend) {
            printf("Simulation de la course principale.\n");
        }
        simulate_course(distance_course, total_laps, ville);
    }

    process_session_files(session_num, ville, session_type);
    free(ville);
    free(session_type);
    return 0;
}