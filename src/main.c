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
    car_t cars[MAX_NUM_CARS];
    int car_numbers[MAX_NUM_CARS] = {1, 11, 44, 63, 16, 55, 4, 81, 14, 18, 10, 31, 23, 2, 22, 3, 77, 24, 20, 27, 1000};
    initialize_cars(cars, car_numbers);

    printf("===== Début de la session: %s =====\n\n", session_file);
    create_directories_from_csv_values("data/liste_circuits.csv", "Course", "Ville");

    int special_weekend = is_special_weekend(ville);
    if (strcmp(session_type, "essai") == 0) { // essai libre (1 pour wk spé et 3 pour wk normal)
        printf("Simulation des essais libres pour un %s.\n", special_weekend ? "week-end spécial" : "week-end normal");
        int total_laps = estimate_max_laps(DUREE_ESSAI, 3*MIN_TIME);
        simulate_sess(cars, MAX_NUM_CARS - 1, DUREE_ESSAI, total_laps, "essai");
        save_session_results(cars, MAX_NUM_CARS - 1, session_file, "w");
    } else if (strcmp(session_type, "shootout") == 0) { // shootout (uniquement pdt wk spé)
        printf("Simulation du Sprint Shootout)\n");
        simulate_qualification(cars, session_num, ville, session_file, session_type);
    } else if (strcmp(session_type, "sprint") == 0) { // sprint (uniquement pdt wk spé)
        printf("Simulation du Sprint\n");
        simulate_course(cars, ville, session_type, session_file);
    } else if (strcmp(session_type, "qualif") == 0) { // qualif pour la course principale
        printf("Simulation des qualifications pour un %s.\n", special_weekend ? "week-end spécial" : "week-end normal");
        simulate_qualification(cars, session_num, ville, session_file, session_type);
    } else if (strcmp(session_type, "course") == 0) { // course principale
        printf("Simulation de la course principale.\n");
        simulate_course(cars, ville, session_type, session_file);
    }
    process_session_files(session_num, ville, session_type); // création fichier resume_<type>.csv
    free(ville);
    free(session_type);
    return 0;
}