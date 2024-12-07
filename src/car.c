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

#define SHM_KEY 12345 // Clé pour la mémoire partagée
#define SEM_NAME "/car_sim_semaphore" // Nom du sémaphore

sem_t *sem; // Define the semaphore


// Fonction pour initialiser le sémaphore global
void init_semaphore() {
    sem = sem_open(SEM_NAME, O_CREAT, 0644, 1); // Valeur initiale du sémaphore : 1 (sémaphore binaire)
    if (sem == SEM_FAILED) {
        perror("Erreur lors de l'initialisation du sémaphore");
        exit(EXIT_FAILURE);
    }
}

// Fonction pour détruire le sémaphore à la fin du programme
void destroy_semaphore() {
    sem_close(sem);
    sem_unlink(SEM_NAME);
}



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
        }
        cars[i].nb_points = 0;
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


/**
 * @brief Attribue les points aux voitures en fonction du classement et sauvegarde les résultats.
 *
 * @param cars Tableau de voitures.
 * @param num_cars Nombre de voitures dans la course.
 * @param meilleur_tour_voiture Numéro de la voiture ayant réalisé le meilleur tour.
 * @param is_sprint Indique si la session est un sprint (1) ou une course classique (0).
 * @param filename Chemin du fichier de points.
 */
void gestion_points(const char *classement_filename, const char *points_filename, int meilleur_tour_voiture, int is_sprint) {
    int points_sprint[] = POINTS_SPRINT;
    int points_course[] = POINTS_COURSE;
    int existing_points[MAX_NUM_CARS - 1] = {0};
    int car_numbers[MAX_NUM_CARS - 1] = {0}; // Pour stocker les numéros de voitures depuis classement.csv
    int num_cars = 0;

    // Charger le classement des voitures depuis classement.csv
    FILE *classement_file = fopen(classement_filename, "r");
    if (!classement_file) {
        perror("Erreur lors de l'ouverture du fichier de classement");
        exit(EXIT_FAILURE);
    }

    // Ignorer l'en-tête
    char buffer[256];
    fgets(buffer, sizeof(buffer), classement_file);

    // Lire les numéros de voiture
    while (fscanf(classement_file, "%d,%*f,%*f,%*f,%*f\n", &car_numbers[num_cars]) == 1) {
        num_cars++;
    }
    fclose(classement_file);
    // Charger les points actuels depuis points.csv
    FILE *points_file = fopen(points_filename, "r");
    if (!points_file) {
        perror("Erreur lors de l'ouverture du fichier de points");
        exit(EXIT_FAILURE);
    }

    // Ignorer l'en-tête
    fgets(buffer, sizeof(buffer), points_file);

    // Charger les points existants
    int car_number, points;
    while (fscanf(points_file, "%d,%d\n", &car_number, &points) == 2) {
        if (car_number < 1 || car_number > MAX_NUM_CARS - 1) {
            fprintf(stderr, "Numéro de voiture invalide : %d\n", car_number);
            continue;
        }
        existing_points[car_number - 1] = points;
    }
    fclose(points_file);

    // Attribution des points selon le classement
    int *points_tableau = is_sprint ? points_sprint : points_course;
    int max_places = is_sprint ? 8 : 10;

    for (int i = 0; i < num_cars && i < max_places; i++) {
        existing_points[car_numbers[i] - 1] += points_tableau[i];
    }

    // Point bonus pour le meilleur tour
    if (!is_sprint && meilleur_tour_voiture > 0) {
        for (int i = 0; i < num_cars; i++) {
            if (car_numbers[i] == meilleur_tour_voiture && i < 10) {
                existing_points[meilleur_tour_voiture - 1] += 1;
                break;
            }
        }
    }

    // Sauvegarde des nouveaux points dans points.csv
    points_file = fopen(points_filename, "w");
    if (!points_file) {
        perror("Erreur lors de la sauvegarde des points");
        exit(EXIT_FAILURE);
    }

    fprintf(points_file, "Car Number,Points\n");
    for (int i = 0; i < MAX_NUM_CARS - 1; i++) {
        if (existing_points[i] > 0) {
            fprintf(points_file, "%d,%d\n", i + 1, existing_points[i]);
        }
    }
    fclose(points_file);
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
        }
        for (int j = 0; j < NUM_SECTORS; j++) { // comparaison pour les secteurs
            if (cars[num_cars].best_sector_times[j] == 0 || 
                cars[i].best_sector_times[j] < cars[num_cars].best_sector_times[j]) {
                cars[num_cars].best_sector_times[j] = cars[i].best_sector_times[j];
            }
        }
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
    pid_t pids[num_cars];
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

    // Initialisation du sémaphore
    init_semaphore();

    // Créer un processus pour chaque voiture
    for (int i = 0; i < num_cars; i++) {
        pids[i] = fork();
        if (pids[i] < 0) {
            perror("Erreur lors du fork");
            exit(EXIT_FAILURE);
        } else if (pids[i] == 0) {
            // Processus enfant : Simulation pour une voiture

            srand(time(NULL) ^ getpid());  // Initialisation aléatoire unique pour chaque processus

            for (int lap = 0; lap < total_laps; lap++) {
                if (shared_cars[i].out) break;

                sem_wait(sem); // Verrouillage du sémaphore
                if (shared_cars[i].pit_stop) {
                    simulate_pit_stop(&shared_cars[i], MIN_PIT_STOP_DURATION, MAX_PIT_STOP_DURATION, session_type);
                } else {
                    generate_sector_times(&shared_cars[i], MIN_TIME, MAX_TIME);
                    if (rand() % 500 < 1) shared_cars[i].out = 1; // 1% de chance de panne
                }
                sem_post(sem); // Déverrouillage du sémaphore

                // Fin de session si la durée est dépassée
                if (strcmp(session_type, "essai") == 0 || strcmp(session_type, "qualif") == 0) {
                    if (shared_cars[i].temps_rouler > session_duration) break;
                }
            }

            shmdt(shared_cars); // Détache la mémoire partagée
            exit(0);  // Le processus enfant se termine
        }
    }

    // Attendre la fin de tous les processus enfants
    for (int i = 0; i < num_cars; i++) {
        waitpid(pids[i], NULL, 0);
    }

    // Mise à jour des voitures après simulation
    memcpy(cars, shared_cars, sizeof(car_t) * num_cars);

    // Détache et libère la mémoire partagée
    shmdt(shared_cars);
    shmctl(shm_id, IPC_RMID, NULL);

    // Destruction du sémaphore
    destroy_semaphore();

    // Calcul des meilleurs temps et affichage des résultats
    find_overall_best_times(cars, num_cars);
    //system("clear");
    usleep(2000000);
    display_practice_results(cars, num_cars, session_type);
    display_overall_best_times(cars, num_cars, session_type);
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
void simulate_course(car_t cars[], const char *ville, char *session_type, char *session_file) {

    int distance_course;

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
    char *input_file = malloc(150 * sizeof(char));
    snprintf(input_file, 150, "data/fichiers/%s", ville);

    // // Read starting grid from classement.csv
    // read_starting_grid(classement_file_path, car_numbers, MAX_NUM_CARS - 1);

    // // Display the starting grid
    // display_starting_grid(car_numbers, MAX_NUM_CARS);

    // // Start the race
    // printf("La course commence !\n");
    // sleep(1); // Simulate the start delay

    simulate_sess(cars, MAX_NUM_CARS - 1, 999999, total_laps, session_type);
    save_session_results(cars, MAX_NUM_CARS - 1 , session_file, "w");


    // gestion_points(input_file, points_file, meilleur_tour_voiture, special_weekend);

    free(classement_file_path);
    free(input_file);
}