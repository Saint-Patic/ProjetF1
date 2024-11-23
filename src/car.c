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
#include <sys/ipc.h>
#include <sys/sem.h>
#include "../include/car.h"
#include "../include/utils.h"
#include "../include/display.h"
#include "../include/file_manager.h"

sem_t sem; // Define the semaphore
extern sem_t sem; // Declare the semaphore



void initialize_cars(car_t cars[], int car_numbers[], int num_cars) {
    for (int i = 0; i < num_cars; i++) {
        cars[i].car_number = car_numbers[i];
        cars[i].best_lap_time = 0.0;
        cars[i].temps_rouler = 0.0;
        cars[i].pit_stop = 0;
        cars[i].pit_stop_nb = 0;
        cars[i].out = 0;
        for (int j = 0; j < NUM_SECTORS; j++) {
            cars[i].sector_times[j] = 0.0;
            cars[i].best_sector_times[j] = 0.0;
        }
    }
}


/**
 * @brief Génère les temps par secteur pour une voiture et met à jour ses meilleurs temps.
 * 
 * @param car Pointeur vers la structure de la voiture.
 * @param min_time Temps minimum possible pour un secteur.
 * @param max_time Temps maximum possible pour un secteur.
 */
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

/**
 * @brief Simule un arrêt au stand pour une voiture.
 * 
 * @param car Pointeur vers la structure de la voiture.
 * @param min_time Temps minimum pour un arrêt au stand.
 * @param max_time Temps maximum pour un arrêt au stand.
 * @param session_type Type de session en cours (course, essai, etc.).
 */
void simulate_pit_stop(car_t *car, int min_time, int max_time, char *session_type) {
    float pit_stop_time = random_float(min_time, max_time);
    car->temps_rouler += pit_stop_time;
    car->pit_stop_nb++;
    car->pit_stop = 0; // Une fois effectué, désactive l'indicateur
}

/**
 * @brief Compare deux voitures en fonction de leur meilleur temps au tour.
 * 
 * @param a Pointeur vers la première voiture.
 * @param b Pointeur vers la deuxième voiture.
 * @return 1 si la première voiture a un temps plus lent, -1 sinon.
 */
int compare_cars(const void *a, const void *b) {
    car_t *carA = (car_t *)a;
    car_t *carB = (car_t *)b;
    return (carA->best_lap_time > carB->best_lap_time) ? 1 : -1;
}

/**
 * @brief Réinitialise le statut "out" et le temps de roulage pour un tableau de voitures.
 * 
 * @param cars Tableau de voitures.
 * @param num_cars Nombre de voitures dans le tableau.
 */
void reset_out_status_and_temps_rouler(car_t cars[], int num_cars) {
    for (int i = 0; i < num_cars; i++) {
        cars[i].out = 0;
        cars[i].temps_rouler = 0;
    }
}

/**
 * @brief Simule une session (essais, qualifications, course) pour un ensemble de voitures.
 * 
 * @param cars Tableau de voitures.
 * @param num_cars Nombre de voitures dans le tableau.
 * @param session_duration Durée maximale de la session.
 * @param total_laps Nombre total de tours prévus.
 * @param session_type Type de session ("course", "essai", etc.).
 */
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

            if (rand() % 100 < 1) { // 1% de panne
                cars[i].out = 1;
                cars[i].pit_stop = 0;
                active_cars--;
            }
            if (cars[i].temps_rouler > session_duration) return;
        }

        if (active_cars == 0) break;

        system("clear");
        printf("Tour %d:\n", lap + 1);
        display_practice_results(cars, num_cars);
        usleep(10000); // sleep for 0.2 seconds
    }

    if (strcmp(session_type, "course") == 0) {
        for (int i = 0; i < num_cars; i++) {
            if (!cars[i].out && cars[i].pit_stop_nb == 0) {
                printf("La voiture %d n'a pas respecté l'arrêt obligatoire.\n", cars[i].car_number);
                cars[i].out = 1;
            }
        }
    }
}

/**
 * @brief Simule une session de qualifications.
 * 
 * @param cars Tableau de voitures.
 * @param session_num Numéro de la session actuelle.
 * @param ville Nom de la ville où se déroule l'événement.
 * @param sprint_mode Mode sprint activé ou non.
 * @param filename Nom du fichier où sauvegarder les résultats.
 */
void simulate_qualification(car_t cars[], int session_num, const char *ville, int sprint_mode, char *filename, char *session_type) {
    int num_cars_in_stage = ternaire_moins_criminel(session_num, 20, 15, 10);
    int eliminated_cars_count = ternaire_moins_criminel(session_num, 5, 5, 0);
    int session_duration = ternaire_moins_criminel(session_num, DUREE_QUALIF_1, DUREE_QUALIF_2, DUREE_QUALIF_3);  
    int sem_id = semget(SEM_KEY, 1, IPC_CREAT | 0666);
    if (sem_id == -1) {
        perror("semget failed");
        exit(EXIT_FAILURE);
    }

    semctl(sem_id, 0, SETVAL, 1);

    struct sembuf sem_op;

    sem_op.sem_num = 0;
    sem_op.sem_op = -1;
    sem_op.sem_flg = 0;
    if (semop(sem_id, &sem_op, 1) == -1) {
        perror("semop lock failed");
        exit(EXIT_FAILURE);
    }

    char *classement_file = malloc(100 * sizeof(char));
    if (sprint_mode) {
        int session_duration = ternaire_moins_criminel(session_num, 720, 600, 480);
        snprintf(classement_file, 100, "data/fichiers/%s/classement_shootout.csv", ville);
    } else {
        snprintf(classement_file, 100, "data/fichiers/%s/classement.csv", ville); 
    }

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
    simulate_sess(eligible_cars, num_cars_in_stage, session_duration, total_laps, session_type);

    qsort(eligible_cars, num_cars_in_stage, sizeof(car_t), compare_cars);

    save_session_results(eligible_cars, num_cars_in_stage, filename, "a");
    save_eliminated_cars(eligible_cars, num_cars_in_stage, eliminated_cars_count, session_num, cars, NUM_CARS, ville, classement_file);
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


/**
 * @brief Simule une session de course.
 * 
 * @param distance Distance de la course en km.
 * @param total_laps Nombre total de tours prévus.
 * @param ville Nom de la ville où se déroule l'événement.
 */
void simulate_course(int special_weekend, int session_num, const char *ville) {
    int shm_fd = shm_open("/cars_shm", O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, sizeof(car_t) * NUM_CARS);
    car_t *cars = mmap(0, sizeof(car_t) * NUM_CARS, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    int car_numbers[NUM_CARS];
    int distance_course = session_num == 1 && special_weekend ? SPRINT_DISTANCE : SESSION_DISTANCE;
    int total_laps = calculate_total_laps(ville, distance_course);

    // Ensure the correct path for classement.csv
    char classement_file_path[100];
    snprintf(classement_file_path, sizeof(classement_file_path), "data/fichiers/%s/classement.csv", ville);

    // Read starting grid from classement.csv
    read_starting_grid(classement_file_path, car_numbers, NUM_CARS);

    // Display the starting grid
    display_starting_grid(car_numbers, NUM_CARS);

    // Initialize cars array using the function from utils.c
    initialize_cars(cars, car_numbers, NUM_CARS);

    //simulate_sess(cars, NUM_CARS, 999999, total_laps, "course");

    munmap(cars, sizeof(car_t) * NUM_CARS);
    shm_unlink("/cars_shm");
}



