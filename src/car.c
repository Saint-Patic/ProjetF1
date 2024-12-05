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
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include "../include/car.h"
#include "../include/utils.h"
#include "../include/display.h"
#include "../include/file_manager.h"

sem_t sem; // Define the semaphore
extern sem_t sem; // Declare the semaphore



void initialize_cars(car_t cars[], int car_numbers[]) {
    for (int i = 0; i < MAX_NUM_CARS; i++) {
        cars[i].car_number = car_numbers[i]; // 21e voiture fictive
        cars[i].best_lap_time = 0.0;
        cars[i].temps_rouler = 0.0;
        cars[i].pit_stop = 0;
        cars[i].pit_stop_nb = 0;
        cars[i].out = (i == MAX_NUM_CARS - 1) ? 1 : 0; // 21e voiture inactive
        cars[i].current_lap = 0.0;
        for (int j = 0; j < NUM_SECTORS; j++) {
            cars[i].sector_times[j] = 0.0;
            cars[i].best_sector_times[j] = 0.0;
            cars[i].best_cars_sector[j] = -1;
        }
        cars[i].nb_points = 0;
        cars[i].best_cars_tour = -1;
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
    car->current_lap = 0;
    for (int i = 0; i < NUM_SECTORS; i++) { // génération des 3 secteurs pour 1 voiture
        car->sector_times[i] = random_float(min_time, max_time);
        car->current_lap += car->sector_times[i];

        // Mise à jour des meilleurs temps pour les secteurs
        if (car->best_sector_times[i] == 0 || car->sector_times[i] < car->best_sector_times[i]) {
            car->best_sector_times[i] = car->sector_times[i];
        }

        // Probabilité de faire un pit stop
        if (rand() % 100 < 15 && i == NUM_SECTORS - 1) { // chance of pit stop
            car->pit_stop_duration = random_float(MIN_PIT_STOP_DURATION, MAX_PIT_STOP_DURATION);
            car->pit_stop = 1;
        }
    }
 


    // Mise à jour du meilleur temps pour la voiture
    if (car->best_lap_time == 0 || car->current_lap < car->best_lap_time) {
        car->best_lap_time = car->current_lap;
    }
    car->temps_rouler += car->current_lap;
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
    car->current_lap += pit_stop_time;
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


int compare_tour_cars(const void *a, const void *b) {
    car_t *carA = (car_t *)a;
    car_t *carB = (car_t *)b;
    return (carA->temps_rouler > carB->temps_rouler) ? 1 : -1;
}

/**
 * @brief Enregistre les meilleurs temps des 3 secteurs et du circuit en général dans une voiture imiganaire
 * @param cars Tableau de voitures
 * @param num_cars Nombre de voitures dans la course
 */
void find_overall_best_times(car_t cars[], int num_cars) {
    // Parcourt les x premières voitures
    for (int i = 0; i < num_cars; i++) { // comparaison pour le tour complet 
        if (cars[num_cars].best_lap_time == 0 || cars[i].best_lap_time < cars[num_cars].best_lap_time) {
            cars[num_cars].best_lap_time = cars[i].best_lap_time;
            cars[num_cars].best_cars_tour = cars[i].car_number;
        }
        for (int j = 0; j < NUM_SECTORS; j++) { // comparaison pour les secteurs
            if (cars[num_cars].best_sector_times[j] == 0 || 
                cars[i].best_sector_times[j] < cars[num_cars].best_sector_times[j]) {
                cars[num_cars].best_sector_times[j] = cars[i].best_sector_times[j];
                cars[num_cars].best_cars_sector[j] = cars[i].car_number;
            }
        }
    }
}


void gestion_points(car_t cars[], const char *input_file, const char *output_file, const char *type_session) {
    FILE *file_in = fopen(input_file, "r");
    if (!file_in) {
        perror("Erreur lors de l'ouverture du fichier d'entrée");
        exit(EXIT_FAILURE);
    }

    char line[256];
    int car_count = 0;

    // Lire l'en-tête et ignorer
    fgets(line, sizeof(line), file_in);

    // Lire les données des voitures
    while (fgets(line, sizeof(line), file_in)) {
        sscanf(line, "%d,%f,%f,%f,%f",
               &cars[car_count].car_number,
               &cars[car_count].best_lap_time,
               &cars[car_count].sector_times[0],
               &cars[car_count].sector_times[1],
               &cars[car_count].sector_times[2]);
        cars[car_count].nb_points = 0;
        car_count++;
    }
    fclose(file_in);

    // Déterminer les points attribuables
    int points_distribution[MAX_NUM_CARS - 1] = {0};
    if (strcmp(type_session, "course") == 0) {
        int course_points[] = POINTS_COURSE;
        memcpy(points_distribution, course_points, sizeof(course_points));
    } else if (strcmp(type_session, "sprint") == 0) {
        int sprint_points[] = POINTS_SPRINT;
        memcpy(points_distribution, sprint_points, sizeof(sprint_points));
    }

    // Attribuer les points selon le classement
    for (int i = 0; i < MAX_NUM_CARS - 1; i++) {
        cars[i].nb_points = points_distribution[i];
    }

    // Ajouter un point bonus pour le meilleur temps au tour
    int best_lap_index = 0;
    for (int i = 1; i < MAX_NUM_CARS - 1; i++) {
        if (cars[i].best_lap_time < cars[best_lap_index].best_lap_time) {
            best_lap_index = i;
        }
    }
    cars[best_lap_index].nb_points += 1; // Ajouter un point bonus pour le meilleur temps au tour
    // Écrire les résultats dans le fichier de sortie
    FILE *file_out = fopen(output_file, "w");
    if (!file_out) {
        perror("Erreur lors de l'ouverture du fichier de sortie");
        exit(EXIT_FAILURE);
    }
    fprintf(file_out, "Car Number,Points\n");
    for (int i = 0; i < MAX_NUM_CARS - 1; i++) {
        fprintf(file_out, "%d,%d\n", cars[i].car_number, cars[i].nb_points);
    }
    fclose(file_out);
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
    // Initialize semaphore
    int sem_id = semget(SEM_KEY, 1, IPC_CREAT | 0666);
    if (sem_id == -1) {
        perror("semget failed");
        exit(EXIT_FAILURE);
    }

    semctl(sem_id, 0, SETVAL, 1);

    struct sembuf sem_op;

    // Simule un tour de la session
    for (int lap = 0; lap < total_laps; lap++) {
        int active_cars = num_cars;
        // fais tourner les voitures pour le nieme tour de la session
        for (int i = 0; i < num_cars; i++) {
            if (i == MAX_NUM_CARS - 1) continue;
            // Lock the semaphore (P operation)
            sem_op.sem_num = 0;
            sem_op.sem_op = -1;
            sem_op.sem_flg = 0;
            if (semop(sem_id, &sem_op, 1) == -1) {
                perror("semop lock failed");
                exit(EXIT_FAILURE);
            }

            // Si la voiture est sortie, passe à la suivante
            if (cars[i].out) {
                active_cars--;
                // Unlock the semaphore (V operation)
                sem_op.sem_op = 1;
                if (semop(sem_id, &sem_op, 1) == -1) {
                    perror("semop unlock failed");
                    exit(EXIT_FAILURE);
                }
                continue;
            }

            // Simule un arrêt au stand si nécessaire
            if (cars[i].pit_stop) {
                simulate_pit_stop(&cars[i], MIN_PIT_STOP_DURATION, MAX_PIT_STOP_DURATION, session_type);
                // Unlock the semaphore (V operation)
                sem_op.sem_op = 1;
                if (semop(sem_id, &sem_op, 1) == -1) {
                    perror("semop unlock failed");
                    exit(EXIT_FAILURE);
                }
                continue;
            }

            // Génère les temps de roulage pour le secteur et les meilleurs temps
            generate_sector_times(&cars[i], MIN_TIME, MAX_TIME);
            find_overall_best_times(cars, num_cars);

            // Simule une panne si le tirage aléatoire est inférieur à 1%   
            if (rand() % 500 < 1) { // 1% de panne
                cars[i].out = 1;
                cars[i].pit_stop = 0;
                active_cars--;
            }

            // Unlock the semaphore (V operation)
            sem_op.sem_op = 1;
            if (semop(sem_id, &sem_op, 1) == -1) {
                perror("semop unlock failed");
                exit(EXIT_FAILURE);
            }

            // porte de sortie pour les sessions d'essais et de qualifications
            if (strcmp(session_type, "essai") == 0 || strcmp(session_type, "qualif") == 0) {
                if (cars[i].temps_rouler > session_duration) return;
            }

            // Switch positions if current lap time is greater
            if (strcmp(session_type, "course") == 0 || strcmp(session_type, "sprint") == 0) {
                if (i > 0 && cars[i].current_lap < cars[i-1].current_lap) {
                    car_t temp = cars[i];
                    cars[i] = cars[i-1];
                    cars[i-1] = temp;
                }
            }
        }

        // si toutes les voitures sont out, ne sert à rien de simuler la suite
        if (active_cars == 0) break;

        // Affiche les résultats du tour
        system("clear");
        // printf("Tour %d:\n", lap + 1);
        display_practice_results(cars, num_cars, session_type);
        display_overall_best_times(cars, num_cars, session_type);
        usleep(10000); // sleep for 0.2 seconds
    }

    // Pour les courses et sprints, les voitures sont obligés de faire au moins un pit-stop => Si aucun pit-stop : elimine
    if (strcmp(session_type, "course") == 0 || strcmp(session_type, "sprint") == 0) {
        for (int i = 0; i < num_cars; i++) {
            if (!cars[i].out && cars[i].pit_stop_nb == 0) {
                printf("La voiture %d n'a pas respecté l'arrêt obligatoire.\n", cars[i].car_number);
                cars[i].out = 1;
            }
        }
    }

    // Remove the semaphore set
    if (semctl(sem_id, 0, IPC_RMID) == -1) {
        perror("semctl remove failed");
        exit(EXIT_FAILURE);
    }
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
 * @param total_laps Nombre total de tours prévus.
 * @param ville Nom de la ville où se déroule l'événement.
 */
void simulate_course(car_t cars[], int special_weekend, int session_num, const char *ville, char *session_type, char *session_file) {

    int distance_course;
    const char *points_file = "data/gestion_points.csv";
    // Créer le fichier de points s'il n'existe pas
    creer_fichier_points(points_file);

    // Identifier la voiture avec le meilleur tour

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


    int total_laps = calculate_total_laps(ville, distance_course);
    char *output_file = malloc(150 * sizeof(char));
    snprintf(output_file, 150, "data/fichiers/%s/gestion_points.csv", ville);

    // // Read starting grid from classement.csv
    // read_starting_grid(classement_file_path, car_numbers, MAX_NUM_CARS - 1);

    // // Display the starting grid
    // display_starting_grid(MAX_NUM_CARS - 1, MAX_NUM_CARS);

    // // Start the race
    // printf("La course commence !\n");
    // sleep(1); // Simulate the start delay

    simulate_sess(cars, MAX_NUM_CARS - 1, 999999, total_laps, session_type);
    save_session_results(cars, MAX_NUM_CARS - 1 , session_file, "w");
    gestion_points(cars, session_file, output_file, session_type);
    
    free(classement_file_path);
    free(output_file);
}