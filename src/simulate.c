#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <pthread.h>
#include <fcntl.h>
#include <float.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h> // Include this header for shared memory functions
#include "../include/car.h"
#include "../include/utils.h"
#include "../include/display.h"
#include "../include/file_manager.h"
#include "../include/simulate.h"


int pause_course = 1000000;
int pause_autre = 10000; 


/**
 * @brief Simule un arrêt au stand pour une voiture.g
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
    car->current_lap += pit_stop_time;
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
    int shm_id;
    car_t *shared_cars;

    // Crée et attache le segment de mémoire partagée
    shm_id = shmget(SHM_KEY, sizeof(car_t) * MAX_NUM_CARS, IPC_CREAT | 0666);
    if (shm_id < 0) {
        perror("Erreur lors de la création de la mémoire partagée");
        exit(EXIT_FAILURE);
    }

    shared_cars = (car_t *)shmat(shm_id, NULL, 0);
    if (shared_cars == (car_t *)-1) {
        perror("Erreur d'attachement mémoire");
        exit(EXIT_FAILURE);
    }

    // Copie initiale des voitures dans la mémoire partagée
    memcpy(shared_cars, cars, sizeof(car_t) * num_cars);

    for (int lap = 0; lap < total_laps; lap++) {
        //printf("Lap %d/%d\n", lap + 1, total_laps);
        for (int i = 0; i < num_cars; i++) {
            if (shared_cars[i].out) continue;

            pid_t pid = fork();
            if (pid < 0) {
                perror("Erreur lors du fork");
                exit(EXIT_FAILURE);
            } else if (pid == 0) {
                // Processus enfant : gérer un seul tour pour une voiture
                srand(time(NULL) ^ getpid()); // Initialisation aléatoire unique pour chaque processus

                // Entrée en section critique
                enter_critical_section(i);

                // Si on est au 3/4 de la course et que la voiture n'a pas encore fait de pit stop, forcer un arrêt
                if (lap >= (total_laps * 3 / 4) && shared_cars[i].pit_stop_nb == 0) {
                    //printf("Forcing pit stop for car %d at lap %d (3/4 of the race).\n", shared_cars[i].car_number, lap + 1);
                    simulate_pit_stop(&shared_cars[i], MIN_PIT_STOP_DURATION, MAX_PIT_STOP_DURATION, session_type);
                }

                if (shared_cars[i].pit_stop) {
                    simulate_pit_stop(&shared_cars[i], MIN_PIT_STOP_DURATION, MAX_PIT_STOP_DURATION, session_type);
                } else {
                    generate_sector_times(&shared_cars[i], MIN_TIME, MAX_TIME);
                    if (rand() % 500 < 1) shared_cars[i].out = 1; // 1% de chance de panne
                }

                // Sortie de section critique
                exit_critical_section(i);

                shmdt(shared_cars); // Détache la mémoire partagée
                exit(0);  // Le processus enfant se termine après un tour
            }
        }

        // Attendre que tous les processus enfants finissent leur tour
        for (int i = 0; i < num_cars; i++) {
            wait(NULL);
        }

        // Mise à jour des voitures après ce tour
        memcpy(cars, shared_cars, sizeof(car_t) * num_cars);

        system("clear");
        find_overall_best_times(cars, num_cars);
        display_practice_results(cars, num_cars, session_type);
        display_overall_best_times(cars, num_cars, session_type);
        strcmp(session_type, "course") == 0 ? usleep(pause_course) : usleep(pause_autre);
    }

    // Détache et libère la mémoire partagée
    shmdt(shared_cars);
    shmctl(shm_id, IPC_RMID, NULL);

    // Calcul des meilleurs temps globaux à la fin
}


/**
 * @brief Simule une session de qualifications.
 * 
 * @param cars Tableau de voitures.
 * @param session_num Numéro de la session actuelle.
 * @param ville Nom de la ville où se déroule l'événement.
 * @param filename Nom du fichier où sauvegarder les résultats.
 */
void simulate_qualification(car_t cars[], int session_num, const char *ville, char *filename, char *session_type) {
    int num_cars_in_stage = ternaire_moins_criminel(session_num, 20, 15, 10); // nbr de voitures qui roulent
    int eliminated_cars_count = ternaire_moins_criminel(session_num, 5, 5, 0); // nbr de voitures éliminées à la fin de la simul
    int session_duration;  // durée de la session
    // choix du fichier pour save les résultats 
    char *classement_file = malloc(100 * sizeof(char));
    if (strcmp(session_type, "shootout") == 0) {
        session_duration = ternaire_moins_criminel(session_num, 720, 600, 480);
        snprintf(classement_file, 100, "data/fichiers/%s/classement_shootout.csv", ville); // si wk spé
    } else {
        session_duration = ternaire_moins_criminel(session_num, DUREE_QUALIF_1, DUREE_QUALIF_2, DUREE_QUALIF_3); 
        snprintf(classement_file, 100, "data/fichiers/%s/classement.csv", ville); // si wk normal
    }

    // chargement des voitures éliminées si la session n'est pas la première
    if (session_num > 1) {
        load_eliminated_cars(classement_file, cars, MAX_NUM_CARS - 1);
    }
    // ##### init voiture pouvant participer à la qualif #####
    car_t eligible_cars[num_cars_in_stage + 1];
    int eligible_index = 0;
    for (int i = 0; i < MAX_NUM_CARS - 1; i++) {
        if (!cars[i].out && eligible_index < num_cars_in_stage) {
            eligible_cars[eligible_index++] = cars[i];
        }
    }
    eligible_cars[num_cars_in_stage] = cars[MAX_NUM_CARS - 1];
    // ##### init voiture pouvant participer à la qualif #####
    int total_laps = estimate_max_laps(session_duration, (float)3 * MIN_TIME) + 1;
    simulate_sess(eligible_cars, num_cars_in_stage, session_duration, total_laps, session_type);
    qsort(eligible_cars, num_cars_in_stage, sizeof(car_t), compare_cars);
    save_session_results(eligible_cars, num_cars_in_stage, filename, "a");
    save_eliminated_cars(eligible_cars, num_cars_in_stage, eliminated_cars_count, session_num, cars, MAX_NUM_CARS - 1, ville, classement_file);
    free(classement_file);
}


/**
 * @brief Simule une session de course.
 * 
 * @param distance Distance de la course en km.
 * @param ville Nom de la ville où se déroule l'événement.
 */
void simulate_course(car_t cars[], const char *ville, char *session_type, char *session_file, int  car_numbers[]) {

    int distance_course;
    const char *points_file = "data/gestion_points.csv";

    // Ensure the correct path for classement.csv
    char *classement_file_path = malloc(150 * sizeof(char));
    if (classement_file_path == NULL) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }

    if (strcmp(session_type, "sprint") == 0) {
        distance_course = SPRINT_DISTANCE;
        snprintf(classement_file_path, 150, "data/fichiers/%s/classement_shootout.csv", ville);
    } else {
        distance_course = SESSION_DISTANCE;  
        snprintf(classement_file_path, 150, "data/fichiers/%s/classement.csv", ville); 
    }
    // Read starting grid from classement.csv
    read_starting_grid(classement_file_path, MAX_NUM_CARS - 1, cars, car_numbers);
    // Display the starting grid
    display_starting_grid(car_numbers, MAX_NUM_CARS - 1);


    // Start the race
    printf("La course commence !\n");
    sleep(1); // Simulate the start delay

    int total_laps = calculate_total_laps(ville, distance_course);
    simulate_sess(cars, MAX_NUM_CARS - 1, 999999, total_laps, session_type);
    save_session_results(cars, MAX_NUM_CARS - 1 , session_file, "w");
    gestion_points(cars, session_file, points_file, session_type);
    display_points(cars, MAX_NUM_CARS - 1);
    free(classement_file_path);
}