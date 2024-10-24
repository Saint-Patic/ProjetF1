#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/shm.h>
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
    srand(time(NULL) ^ getpid());  // Utiliser getpid() pour s'assurer que chaque processus enfant a une graine différente

    printf("Voiture %d commence la course...\n", car->car_number);
    for (int i = 0; i < NUM_SECTORS; i++) {
        car->sector_times[i] = (rand() % (max_time - min_time + 1)) + min_time + ((float)rand() / RAND_MAX);  // Temps aléatoire avec décimales
        car->best_lap_time += car->sector_times[i];  // Calcul du meilleur temps total
        printf("Secteur %d (Voiture %d) : %.2f secondes\n", i + 1, car->car_number, car->sector_times[i]);
    }
    printf("Voiture %d a terminé avec un meilleur temps de %.2f secondes.\n", car->car_number, car->best_lap_time);
}

float get_best_sectors_time(struct CarTime cars[], int num_cars, int sector) {
    float best_time = cars[0].sector_times[sector];
    for (int i = 1; i < num_cars; i++) {
        if (cars[i].sector_times[sector] < best_time) {
            best_time = cars[i].sector_times[sector];
        }
    }
    return best_time;
}

float get_best_lap_time(struct CarTime cars[], int num_cars) {
    float best_lap_time = cars[0].best_lap_time;
    for (int i = 1; i < num_cars; i++) {
        if (cars[i].best_lap_time < best_lap_time) {
            best_lap_time = cars[i].best_lap_time;
        }
    }
    return best_lap_time;
}

int main() {
    int shmid;
    struct CarTime *cars;

    // Allocation de mémoire partagée
    shmid = shmget(IPC_PRIVATE, NUM_CARS * sizeof(struct CarTime), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("Erreur lors de la création de la mémoire partagée");
        exit(1);
    }

    // Attachement de la mémoire partagée
    cars = (struct CarTime *)shmat(shmid, NULL, 0);
    if (cars == (void *)-1) {
        perror("Erreur lors de l'attachement de la mémoire partagée");
        exit(1);
    }

    int min_time = 5;  // Temps minimum en secondes
    int max_time = 10; // Temps maximum en secondes

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

    // Affichage des meilleurs temps par secteur
    for (int i = 0; i < NUM_SECTORS; i++) {
        float best_sector_time = get_best_sectors_time(cars, NUM_CARS, i);
        printf("Meilleur temps du secteur %d : %.2f secondes\n", i + 1, best_sector_time);
    }

    // Affichage du meilleur temps au tour
    float best_lap = get_best_lap_time(cars, NUM_CARS);
    printf("Meilleur temps au tour : %.2f secondes\n", best_lap);

    // Détachement de la mémoire partagée
    shmdt(cars);

    // Suppression de la mémoire partagée
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}
