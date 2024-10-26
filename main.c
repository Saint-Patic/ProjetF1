#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "car.h"
#include "display.h"
#include "file_manager.h"

#define NUM_CARS 20
#define MIN_TIME 25    // Temps minimal pour un secteur
#define MAX_TIME 45    // Temps maximal pour un secteur

int main() {
    srand(time(NULL));

    const char *output_file = "fichier_enregistree/meilleurs_temps.csv";
    struct CarTime cars[NUM_CARS];
    int car_numbers[NUM_CARS] = {1, 11, 44, 63, 16, 55, 4, 81, 14, 18, 10, 31, 23, 2, 22, 3, 77, 24, 20, 27};
    char *session_files[] = {
        "fichier_enregistree/session_P1.csv",
        "fichier_enregistree/session_P2.csv",
        "fichier_enregistree/session_P3.csv"
    };
    int session_count = sizeof(session_files) / sizeof(session_files[0]);

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

    printf("===== Début du week-end de Grand Prix =====\n\n");

    for (int session = 0; session < session_count; session++) {
        printf("\n=== Début de la session %d ===\n", session + 1);

        simulate_practice_session(cars, NUM_CARS, MIN_TIME, MAX_TIME);
        save_session_results(cars, NUM_CARS, session_files[session]);
        printf("Les résultats de la session %d ont été enregistrés dans %s\n", session + 1, session_files[session]);

        if (session < session_count - 1) {
            printf("Appuyez sur Entrée pour passer à la session suivante...");
            getchar();
        }
    }

    printf("\n===== Résultats finaux du week-end =====\n");
    display_overall_best_times(cars, NUM_CARS);

    combine_session_results(session_files, session_count, output_file);

    printf("\nFin du week-end de Grand Prix avec les meilleurs temps enregistrés.\n");

    return 0;
}
