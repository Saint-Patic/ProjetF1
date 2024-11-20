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
#include "car.h"
#include "utils.h"
#include "display.h"
#include "file_manager.h"

#define NUM_CARS 20 // Define NUM_CARS with an appropriate value

sem_t sem; // Define the semaphore



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

void simulate_pit_stop(car_t *car, int min_time, int max_time) {
    if (car->pit_stop) {
        float pit_stop_time = random_float(min_time, max_time);
        car->pit_stop_nb += 1;
        if (car->pit_stop_duration > pit_stop_time) {
            car->pit_stop_duration -= pit_stop_time;
            car->temps_rouler += pit_stop_time;
        } else {
            car->pit_stop = 0;
        }
    }
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

void *simulate_lap(void *arg) {
    lap_args_t *args = (lap_args_t *)arg;
    car_t *car = args->car;
    int min_time = args->min_time;
    int max_time = args->max_time;

    sem_wait(&sem);
    generate_sector_times(car, min_time, max_time);
    simulate_pit_stop(car, min_time, max_time);
    sem_post(&sem);

    return NULL;
}

void simulate_sess(car_t cars[], int num_cars, int min_time, int max_time, int session_duration, int total_laps) {
    pthread_t threads[num_cars];
    lap_args_t args[num_cars];
    sem_init(&sem, 0, 1); // Initialize the semaphore

    for (int lap = 0; lap < total_laps; lap++) {
        for (int i = 0; i < num_cars; i++) {
            if (cars[i].out) continue;
            args[i].car = &cars[i];
            args[i].min_time = min_time;
            args[i].max_time = max_time;
            pthread_create(&threads[i], NULL, simulate_lap, (void *)&args[i]);
        }
        for (int i = 0; i < num_cars; i++) {
            pthread_join(threads[i], NULL);
        }

        system("clear");
        printf("Tour %d:\n", lap + 1);
        display_practice_results(cars, num_cars);
        sleep(0.2);
    }

    sem_destroy(&sem); // Destroy the semaphore
    reset_out_status_and_temps_rouler(cars, num_cars);
}

void simulate_qualification(car_t cars[], int session_num, const char *ville, int min_time, int max_time, int total_cars) {
    int num_cars_in_stage = ternaire_moins_criminel(session_num, 20, 15, 10);
    int eliminated_cars_count = ternaire_moins_criminel(session_num, 5, 5, 0);
    int session_duration = ternaire_moins_criminel(session_num, 720, 600, 480);

    char classement_file[100];
    snprintf(classement_file, 100, "fichiers/%s/classement.csv", ville);

    load_eliminated_cars(classement_file, cars, total_cars);

    car_t eligible_cars[num_cars_in_stage];
    int eligible_index = 0;

    for (int i = 0; i < total_cars; i++) {
        if (!cars[i].out && eligible_index < num_cars_in_stage) {
            eligible_cars[eligible_index++] = cars[i];
        }
    }

    int total_laps = estimate_max_laps(session_duration, (float)3 * min_time) + 1;
    simulate_sess(eligible_cars, num_cars_in_stage, min_time, max_time, session_duration, total_laps);

    qsort(eligible_cars, num_cars_in_stage, sizeof(car_t), compare_cars);

    char session_file[100];
    snprintf(session_file, 100, "fichiers/%s/qualif_%d.csv", ville, session_num);

    save_session_results(eligible_cars, num_cars_in_stage, session_file, "a");
    save_eliminated_cars(eligible_cars, num_cars_in_stage, eliminated_cars_count, session_num, cars, total_cars, ville);
}

void simulate_course(int distance, int min_time, int max_time, int total_laps) {
    int shm_fd = shm_open("/cars_shm", O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, sizeof(car_t) * NUM_CARS);
    car_t *cars = mmap(0, sizeof(car_t) * NUM_CARS, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    // Initialize cars array
    for (int i = 0; i < NUM_CARS; i++) {
        cars[i].car_number = i + 1;
        cars[i].best_lap_time = 0;
        cars[i].temps_rouler = 0;
        cars[i].pit_stop = 0;
        cars[i].pit_stop_nb = 0;
        cars[i].out = 0;
        for (int j = 0; j < NUM_SECTORS; j++) {
            cars[i].sector_times[j] = 0;
            cars[i].best_sector_times[j] = 0;
        }
    }

    simulate_sess(cars, NUM_CARS, min_time, max_time, 999999, total_laps);

    munmap(cars, sizeof(car_t) * NUM_CARS);
    shm_unlink("/cars_shm");
}

int ternaire_moins_criminel(int session_num, int res1, int res2, int resDefault) {
    switch (session_num) {
        case 1:
            return res1;
        case 2:
            return res2;
        default:
            return resDefault;
    }
}