#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include "car.h"
#include "utils.h"

#define NUM_CARS 20
#define MIN_TIME 25
#define MAX_TIME 45

void initialize_cars(struct CarTime cars[], int car_numbers[], int num_cars, int max_time) {
    for (int i = 0; i < num_cars; i++) {
        cars[i].car_number = car_numbers[i];
        cars[i].pit_stop = 0;
        cars[i].out = 0;
        cars[i].best_lap_time = max_time * 3; // Initialiser à une valeur élevée
        cars[i].temps_rouler = 0; // Initialiser le temps de roulage
        for (int j = 0; j < NUM_SECTORS; j++) {
            cars[i].best_sector_times[j] = 0; // Initialiser les meilleurs temps de secteur
        }
    }
}

int main() {
    struct CarTime *cars;
    int car_numbers[NUM_CARS] = {1, 11, 44, 63, 16, 55, 4, 81, 14, 18, 10, 31, 23, 2, 22, 3, 77, 24, 20, 27};
    
    // Initialisation aléatoire
    initialize_random();

    // Allocation mémoire partagée
    int shmid = shmget(IPC_PRIVATE, NUM_CARS * sizeof(struct CarTime), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("Erreur mémoire partagée");
        exit(1);
    }
    
    // Attachement de la mémoire partagée
    cars = (struct CarTime *)shmat(shmid, NULL, 0);
    if (cars == (void *)-1) {
        perror("Erreur attachement mémoire partagée");
        exit(1);
    }

    // Initialisation des voitures
    initialize_cars(cars, car_numbers, NUM_CARS, MAX_TIME);

    // Simulation de la séance d'essai
    simulate_practice_session(cars, NUM_CARS, MIN_TIME, MAX_TIME);

    // Affichage des résultats
    printf("\n=== Meilleurs temps de la période d'essai ===\n");
    display_practice_results(cars, NUM_CARS);

    // Afficher les meilleurs temps globaux
    display_overall_best_times(cars, NUM_CARS);

    // Nettoyage
    shmdt(cars);
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}
