#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include "../include/car.h"
#include "../include/utils.h"
#include "../include/display.h"
#include "../include/file_manager.h"
#include <sys/ipc.h>
#include <sys/sem.h>

#define SEM_KEY 1234

sem_t sem; // Define the semaphore
extern sem_t sem; // Declare the semaphore

void generate_sector_times(car_t *car, int min_time, int max_time) {
    float lap_time = 0;
    for (int i = 0; i < NUM_SECTORS; i++) {
        car->sector_times[i] = random_float(min_time, max_time);
        lap_time += car->sector_times[i];

        if (car->best_sector_times[i] == 0 || car->sector_times[i] < car->best_sector_times[i]) {
            car->best_sector_times[i] = car->sector_times[i];
        }
        if (rand() % 100 < 20 && i == NUM_SECTORS - 1) { // 20% chance of pit stop
            car->pit_stop_duration = random_float(MIN_PIT_STOP_DURATION, MAX_PIT_STOP_DURATION);
            car->pit_stop = 1;
        }
    }

    if (car->best_lap_time == 0 || lap_time < car->best_lap_time) {
        car->best_lap_time = lap_time;
    }
    car->temps_rouler += lap_time;
}

void simulate_pit_stop(car_t *car, int min_time, int max_time, char *session_type) {
    // Pit stop obligatoire dans le secteur 3
    float pit_stop_time = random_float(min_time, max_time);
    car->temps_rouler += pit_stop_time;
    car->pit_stop_nb++;
    car->pit_stop = 0; // Une fois effectué, désactive l'indicateur
}

int compare_cars(const void *a, const void *b) {
    car_t *carA = (car_t *)a;
    car_t *carB = (car_t *)b;
    return (carA->best_lap_time > carB->best_lap_time) ? 1 : -1;
}

void reset_out_status_and_temps_rouler(car_t cars[], int num_cars) {
    for (int i = 0; i < num_cars; i++) {
        cars[i].out = 0;
        cars[i].temps_rouler = 0;
    }
}

void simulate_sess(car_t cars[], int num_cars, int session_duration, int total_laps, char *session_type) {
    for (int lap = 0; lap < total_laps; lap++) {
        int active_cars = num_cars;

        for (int i = 0; i < num_cars; i++) {
            if (cars[i].out) {
                active_cars--;
                continue;
            }

            if (cars[i].pit_stop) {
                simulate_pit_stop(&cars[i], MIN_PIT_STOP_DURATION, MAX_PIT_STOP_DURATION, session_type);
                continue;
            }

            generate_sector_times(&cars[i], MIN_TIME, MAX_TIME);

            // Gestion des pannes
            if (rand() % 100 < 1) { // 1% de panne
                cars[i].out = 1;
                cars[i].pit_stop = 0;
                active_cars--;
            }
            if (cars[i].temps_rouler > session_duration) return;
        }

        if (active_cars == 0) break;

        // system("clear");
        printf("Tour %d:\n", lap + 1);
        display_practice_results(cars, num_cars);
        usleep(10000); // sleep for 0.2 seconds
    }

    // En course : Vérifiez que chaque voiture a effectué au moins un pit stop
    if (strcmp(session_type, "course") == 0) {
        for (int i = 0; i < num_cars; i++) {
            if (!cars[i].out && cars[i].pit_stop_nb == 0) {
                printf("La voiture %d n'a pas respecté l'arrêt obligatoire.\n", cars[i].car_number);
                cars[i].out = 1;
            }
        }
    }

    display_practice_results(cars, num_cars);
}

void simulate_qualification(car_t cars[], int session_num, const char *ville, int sprint_mode, char *filename) {
    int num_cars_in_stage = ternaire_moins_criminel(session_num, 20, 15, 10, sprint_mode);
    int eliminated_cars_count = ternaire_moins_criminel(session_num, 5, 5, 0, sprint_mode);
    int session_duration = ternaire_moins_criminel(session_num, DUREE_QUALIF_1, DUREE_QUALIF_2, DUREE_QUALIF_3, sprint_mode);

    // Create a semaphore set with one semaphore
    int sem_id = semget(SEM_KEY, 1, IPC_CREAT | 0666);
    if (sem_id == -1) {
        perror("semget failed");
        exit(EXIT_FAILURE);
    }

    // Initialize the semaphore to 1
    semctl(sem_id, 0, SETVAL, 1);

    // Semaphore operations
    struct sembuf sem_op;

    // Lock the semaphore (P operation)
    sem_op.sem_num = 0;
    sem_op.sem_op = -1;
    sem_op.sem_flg = 0;
    if (semop(sem_id, &sem_op, 1) == -1) {
        perror("semop lock failed");
        exit(EXIT_FAILURE);
    }

    // Critical section
    char *classement_file = malloc(100 * sizeof(char));
    snprintf(classement_file, 100, "data/fichiers/%s/classement.csv", ville);
    if (session_num > 1) {
        load_eliminated_cars(classement_file, cars, NUM_CARS);
    }

    car_t eligible_cars[num_cars_in_stage];
    int eligible_index = 0;

    for (int i = 0; i < NUM_CARS; i++) {
        if (!cars[i].out && eligible_index < num_cars_in_stage) {
            eligible_cars[eligible_index++] = cars[i];
        }
    }

    int total_laps = estimate_max_laps(session_duration, (float)3 * MIN_TIME) + 1;
    simulate_sess(eligible_cars, num_cars_in_stage, session_duration, total_laps, "qualif");

    qsort(eligible_cars, num_cars_in_stage, sizeof(car_t), compare_cars);

    save_session_results(eligible_cars, num_cars_in_stage, filename, "a");
    save_eliminated_cars(eligible_cars, num_cars_in_stage, eliminated_cars_count, session_num, cars, NUM_CARS, ville);
    free(classement_file);

    // Unlock the semaphore (V operation)
    sem_op.sem_op = 1;
    if (semop(sem_id, &sem_op, 1) == -1) {
        perror("semop unlock failed");
        exit(EXIT_FAILURE);
    }

    // Remove the semaphore set
    if (semctl(sem_id, 0, IPC_RMID) == -1) {
        perror("semctl remove failed");
        exit(EXIT_FAILURE);
    }
}

void simulate_course(int distance, int total_laps) {
    int shm_fd = shm_open("/cars_shm", O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, sizeof(car_t) * NUM_CARS);
    car_t *cars = mmap(0, sizeof(car_t) * NUM_CARS, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    int car_numbers[NUM_CARS];
    for (int i = 0; i < NUM_CARS; i++) {
        car_numbers[i] = i + 1; // Assuming car numbers are 1, 2, 3, ...
    }

    // Initialize cars array using the function from utils.c
    initialize_cars(cars, car_numbers, NUM_CARS);

    simulate_sess(cars, NUM_CARS, 999999, total_laps, "course");

    munmap(cars, sizeof(car_t) * NUM_CARS);
    shm_unlink("/cars_shm");
}



