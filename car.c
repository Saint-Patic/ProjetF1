#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // Pour sleep()
#include <string.h> // Pour system()
#include "car.h"
#include "utils.h" 

void generate_sector_times(struct CarTime *car, int min_time, int max_time) {
    car->best_lap_time = 0;
    // static int counter = 0;
    for (int i = 0; i < NUM_SECTORS; i++) {
        car->sector_times[i] = random_float(min_time, max_time);
        car->best_lap_time += car->sector_times[i];

        // Mettre à jour le meilleur temps de secteur
        if (car->best_sector_times[i] == 0 || car->sector_times[i] < car->best_sector_times[i]) {
            car->best_sector_times[i] = car->sector_times[i];
        }
        // printf("Génération des temps de secteur: %d\n", counter++);
    }

    // Mettre à jour le meilleur temps de tour
    if (car->best_lap_time < car->best_lap_time || car->best_lap_time == 0) {
        car->best_lap_time = car->best_lap_time; // Mettre à jour avec le nouveau meilleur temps
    }
}

void simulate_practice_session(struct CarTime cars[], int num_cars, int min_time, int max_time) {
    int total_laps = MAX_LAPS; // Total number of laps
    for (int lap = 0; lap < total_laps; lap++) {
        for (int i = 0; i < num_cars; i++) {
            if (cars[i].out) {
                continue; // Ignore out of race cars
            }

            // Gestion des pit stops
            if (cars[i].pit_stop) {
                // Décrémenter la durée du pit stop d'un temps aléatoire
                int pit_stop_time = random_float(min_time, max_time); // Temps de pit stop aléatoire
                if (cars[i].pit_stop_duration > pit_stop_time) {
                    cars[i].pit_stop_duration -= pit_stop_time; // Décrémenter le temps de pit stop
                    continue; // Ignorez la voiture pendant le pit stop
                } else {
                    cars[i].pit_stop = 0; // La voiture sort du pit stop
                }
            }

            // Génération des temps de secteur pour les voitures en course
            generate_sector_times(&cars[i], min_time, max_time);

            // Incrémentez le temps de roulage
            cars[i].temps_rouler += (float)max_time; // Ajouter le temps de ce tour

            // Vérifiez si la voiture a dépassé 1 heure (3600 secondes)
            if (cars[i].temps_rouler >= 3600) {
                for (int j = 0; j < num_cars; j++) {
                    cars[j].out = 1; // Toutes les voitures s'arrêtent
                }
                return; // Sortir de la simulation
            }

            // Événements aléatoires pour pit ou out
            if (rand() % 100 < 10) {
                cars[i].pit_stop_duration = random_float(MIN_PIT_STOP_DURATION, MAX_PIT_STOP_DURATION);
                cars[i].pit_stop = 1; // 10% de chance d'entrer en pit stop
            } else if (rand() % 100 < 5) {
                cars[i].out = 1; // 5% de chance de sortir de la course
            }
        }
    

        // Rafraîchir l'affichage à chaque tour
        system("clear"); // Efface l'écran
        printf("Tour %d:\n", lap + 1);
        display_practice_results(cars, num_cars);
        sleep(1); // Attendre 1 seconde avant de rafraîchir l'affichage
    }
}   

void display_practice_results(struct CarTime cars[], int num_cars) {
    qsort(cars, num_cars, sizeof(struct CarTime), compare_cars);
    
    printf("=================================================================\n");
    printf("|  #   |  Secteur 1  |  Secteur 2  |  Secteur 3  |   Tour   | Diff |\n");
    printf("=================================================================\n");

    float prev_time = 0;
    for (int i = 0; i < num_cars; i++) {
        if (i == 0) prev_time = cars[i].best_lap_time;

        float diff = cars[i].best_lap_time - prev_time;
        printf("| %3d  |   %7.2f   |   %7.2f   |   %7.2f   |  %7.2f  |  %+5.2f |\n",
               cars[i].car_number,
               cars[i].sector_times[0],
               cars[i].sector_times[1],
               cars[i].sector_times[2],
               cars[i].best_lap_time,
               i == 0 ? 0.00 : diff);

        if (cars[i].pit_stop) {
            printf(" (P)\n");
        } else if (cars[i].out) {
            printf(" (Out)\n");
        }
        prev_time = cars[i].best_lap_time;
    }
    printf("=================================================================\n");
}

// Comparer les voitures par meilleur temps de tour
int compare_cars(const void *a, const void *b) {
    struct CarTime *carA = (struct CarTime *)a;
    struct CarTime *carB = (struct CarTime *)b;
    return (carA->best_lap_time > carB->best_lap_time) ? 1 : -1;
}

void display_overall_best_times(struct CarTime cars[], int num_cars) {
    float overall_best_sector_times[NUM_SECTORS] = {0}; // Pour stocker les meilleurs temps de chaque secteur
    float overall_best_lap_time = 0; // Pour stocker le meilleur temps de tour global

    // Initialiser les meilleurs temps avec des valeurs élevées
    for (int i = 0; i < NUM_SECTORS; i++) {
        overall_best_sector_times[i] = 9999.0; // Une valeur arbitraire élevée
    }
    overall_best_lap_time = 9999.0; // Une valeur arbitraire élevée

    // Parcourir toutes les voitures pour déterminer les meilleurs temps
    for (int i = 0; i < num_cars; i++) {
        for (int j = 0; j < NUM_SECTORS; j++) {
            if (cars[i].best_sector_times[j] > 0 && cars[i].best_sector_times[j] < overall_best_sector_times[j]) {
                overall_best_sector_times[j] = cars[i].best_sector_times[j];
            }
        }

        if (cars[i].best_lap_time > 0 && cars[i].best_lap_time < overall_best_lap_time) {
            overall_best_lap_time = cars[i].best_lap_time;
        }
    }

    // Affichage des meilleurs temps
    printf("\nMeilleurs temps de la période d'essai :\n");
    printf("=======================================================\n");
    for (int i = 0; i < NUM_SECTORS; i++) {
        printf("Meilleur temps secteur %d : %.2f secondes\n", i + 1, overall_best_sector_times[i]);
    }
    printf("Meilleur temps de tour global : %.2f secondes\n", overall_best_lap_time);
    printf("=======================================================\n");
}
