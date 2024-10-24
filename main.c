#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define NUM_SECTORS 3  // Nombre de secteurs par tour
#define NUM_CARS 3     // Nombre de voitures

struct CarTime {
    int car_number;
    float sector_times[NUM_SECTORS];  // Temps pour chaque secteur
    float best_lap_time;  // Meilleur temps au tour
    int pit_stop;         // 0 si sur la piste, 1 si au stand
    int out;              // 0 si encore en course, 1 si hors course
    int points;           // Points obtenus
};

void simulate_f1(struct CarTime *car, int min_time, int max_time) {
    car->best_lap_time = 0;
    srand(time(NULL) + car->car_number);  // Initialisation unique pour chaque voiture

    printf("Voiture %d commence la course...\n", car->car_number);
    for (int i = 0; i < NUM_SECTORS; i++) {
        car->sector_times[i] = (rand() % (max_time - min_time + 1)) + min_time + ((float)rand() / RAND_MAX);  // Temps aléatoire avec décimales
        car->best_lap_time += car->sector_times[i];  // Calcul du meilleur temps total
        printf("Secteur %d (Voiture %d) : %.2f secondes\n", i + 1, car->car_number, car->sector_times[i]);
    }
    printf("Voiture %d a terminé avec un meilleur temps de %.2f secondes.\n", car->car_number, car->best_lap_time);
}

int main() {
    struct CarTime cars[NUM_CARS];  // Tableau de voitures
    int min_time = 25;  // Temps minimum en secondes
    int max_time = 45; // Temps maximum en secondes

    for (int i = 0; i < NUM_CARS; i++) {
        cars[i].car_number = i + 1;
        cars[i].pit_stop = 0;
        cars[i].out = 0;
        cars[i].points = 0;

        int pid = fork();

        if (pid == 0) {
            // Processus enfant : simulation de la Formule 1
            simulate_f1(&cars[i], min_time, max_time);  // Simule une voiture
            exit(0);  // Fin du processus enfant
        } else if (pid > 0) {
            // Processus parent continue à créer d'autres voitures
            continue;
        } else {
            // Erreur dans la création du processus
            perror("Erreur lors de la création du processus");
            return 1;
        }
    }

    // Le processus parent attend que toutes les voitures finissent
    for (int i = 0; i < NUM_CARS; i++) {
        int status;
        wait(&status);  // Attend la fin de chaque processus enfant
    }

    printf("Toutes les voitures ont terminé la course.\n");

    return 0;
}
