#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include "car.h"
#include "display.h"
#include "file_manager.h"

#define NUM_CARS 20
#define MIN_TIME 25
#define MAX_TIME 45



int main(int argc, char *argv[]) {
    // check s'il y a des paramètres 
    if (argc != 2) {
        printf("Usage: %s <session_filename>\n", argv[0]);
        printf("Format paramètre attendu:  fichier_enregistree/session_<numéro>.csv\n");
        return 1;
    }

    char *session_file = argv[1];
    int session_num;

    // check si le format de fichier est correct
    if (sscanf(session_file, "fichier_enregistree/session_%d.csv", &session_num) != 1) {
        printf("Nom de fichier invalide. Utilisez le format fichier_enregistree/session_<numéro>.csv\n");
        return 1;
    }

    // crée la chaine prev_session_file avec le numéro précédent de celui mis en paramètre
    char prev_session_file[50];
    sprintf(prev_session_file, "fichier_enregistree/session_%d.csv", session_num - 1);

    // fichier exite ? ne simule rien : simule la session
    if (file_exists(session_file)) {
        printf("Le fichier %s existe déjà. La session a déjà été exécutée.\n", session_file);
        return 0;
    }


    // course précédente existe ? simule ? simule rien
    if (session_num > 1 && !file_exists(prev_session_file)) {
        printf("La session précédente %s n'existe pas. Exécution annulée.\n", prev_session_file);
        return 0;
    }

    srand(time(NULL));
    int session_duration = 3600;
    struct CarTime cars[NUM_CARS];
    int car_numbers[NUM_CARS] = {1, 11, 44, 63, 16, 55, 4, 81, 14, 18, 10, 31, 23, 2, 22, 3, 77, 24, 20, 27};

    // initialision des voitures
    for (int i = 0; i < NUM_CARS; i++) {
        cars[i].car_number = car_numbers[i];
        cars[i].best_lap_time = 0;
        cars[i].temps_rouler = 0;
        cars[i].pit_stop = 0;
        cars[i].out = 0;
        for (int j = 0; j < NUM_SECTORS; j++) {
            cars[i].sector_times[j] = 0;
            cars[i].best_sector_times[j] = 0;
        }
    }


    // simulation 
    printf("===== Début de la session de pratique : %s =====\n\n", session_file);

    simulate_practice(cars, NUM_CARS, MIN_TIME, MAX_TIME, session_duration);
    save_session_results(cars, NUM_CARS, session_file);
    printf("Les résultats de la session ont été enregistrés dans %s\n", session_file);

    return 0;
}
