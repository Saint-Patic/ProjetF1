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
#include "../include/algorithm.h"
// Variable globale pour stocker le type de session
extern char *current_session_type;
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
        cars[i].nb_tours = 0;
    }
}


/**
 * @brief Génère les temps par secteur pour une voiture et met à jour ses meilleurs temps.
 * 
 * @param car Pointeur vers la structure de la voiture.
 * @param min_time Temps minimum possible pour un secteur.
 * @param max_time Temps maximum possible pour un secteur.session_type
 */
void update_best_times(car_t *car, int sector_index) {
    if (car->best_sector_times[sector_index] == 0 || car->sector_times[sector_index] < car->best_sector_times[sector_index]) {
        car->best_sector_times[sector_index] = car->sector_times[sector_index];
    }

}
void generate_sector_times(car_t *car, int min_time, int max_time, char *session_type) {
    car->current_lap = 0;
    for (int i = 0; i < NUM_SECTORS; i++) {
        car->sector_times[i] = random_float(min_time, max_time);
        car->current_lap += car->sector_times[i];
        update_best_times(car, i);

        int proba_pit_stop =  strcmp(session_type, "course") || strcmp(session_type, "sprint") ? 15 : 1;
        if (rand() % 100 < proba_pit_stop && i == NUM_SECTORS - 1) {
            car->pit_stop_duration = random_float(MIN_PIT_STOP_DURATION, MAX_PIT_STOP_DURATION);
            car->pit_stop = 1;
        }
    }
    if (car->best_lap_time == 0 || car->current_lap < car->best_lap_time) {
        car->best_lap_time = car->current_lap;
    }
    car->temps_rouler += car->current_lap;
    car->nb_tours += 1;
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
void gestion_points(car_t cars[], const char *input_file, const char *output_file, const char *type_session) {
    FILE *file_in = fopen(input_file, "r");
    if (!file_in) {
        perror("Erreur lors de l'ouverture du fichier d'entrée");
        exit(EXIT_FAILURE);
    }

    char line[1024];
    int car_count = 0;

    // Lire l'en-tête et ignorer
    fgets(line, sizeof(line), file_in);
    // Lire les données des voitures
    while (fgets(line, sizeof(line), file_in)) {
        if (strncmp(line, "Best Sector Times", 17) == 0 || car_count >= MAX_NUM_CARS - 1) {
            break; // Arrêter la lecture des données
        }
        sscanf(line, "%d,%f,%f,%f,%f",
               &cars[car_count].car_number,
               &cars[car_count].best_lap_time,
               &cars[car_count].sector_times[0],
               &cars[car_count].sector_times[1],
               &cars[car_count].sector_times[2]);
        cars[car_count++].nb_points = 0;

    }
    fclose(file_in);

    // Déterminer les points attribuables
    int points_distribution[MAX_NUM_CARS - 1] = {0};
    int max_points;
    if (strcmp(type_session, "course") == 0) {
        max_points = 10;
        int course_points[] = POINTS_COURSE;
        memcpy(points_distribution, course_points, sizeof(course_points));
    } else if (strcmp(type_session, "sprint") == 0) {
        max_points = 8;
        int sprint_points[] = POINTS_SPRINT;
        memcpy(points_distribution, sprint_points, sizeof(sprint_points));
    }

    for (int i = 0; i < car_count; i++) {
        cars[i].nb_points = points_distribution[i];
    }

    int best_lap_index = 0;
    for (int i = 1; i < car_count; i++) {
        if (cars[i].best_lap_time < cars[best_lap_index].best_lap_time && i < max_points) {
            best_lap_index = i;
        }
    }
    cars[best_lap_index].nb_points += 1;

    FILE *file_out = fopen(output_file, "r");
    if (file_out) {
        fgets(line, sizeof(line), file_out);

        while (fgets(line, sizeof(line), file_out)) {
            int car_number, existing_points;
            sscanf(line, "%d,%d", &car_number, &existing_points);

            for (int i = 0; i < car_count; i++) {
                if (cars[i].car_number == car_number) {
                    cars[i].nb_points += existing_points;
                    break;
                }
            }
        }
        fclose(file_out);
    }

    for (int i = 0; i < car_count - 1; i++) {
        for (int j = i + 1; j < car_count; j++) {
            if (cars[i].nb_points < cars[j].nb_points) {
                car_t temp = cars[i];
                cars[i] = cars[j];
                cars[j] = temp;
            }
        }
    }

    file_out = fopen(output_file, "w");
    if (!file_out) {
        perror("Erreur lors de l'ouverture du fichier de sortie");
        exit(EXIT_FAILURE);
    }
    fprintf(file_out, "Car Number,Points\n");
    for (int i = 0; i < car_count; i++) {
        fprintf(file_out, "%d,%d\n", cars[i].car_number, cars[i].nb_points);
    }
    fclose(file_out);

    display_points(cars, car_count);
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
void handle_pit_stop(car_t *car, int lap, int total_laps, char *session_type) {
    float forced_pit_stop = 3/4;
    if (lap >= (total_laps * forced_pit_stop) && car->pit_stop_nb == 0 && strcmp(session_type, "course") == 0) {
        simulate_pit_stop(car, MIN_PIT_STOP_DURATION, MAX_PIT_STOP_DURATION, session_type);
    }
    if (car->pit_stop) {
        simulate_pit_stop(car, MIN_PIT_STOP_DURATION, MAX_PIT_STOP_DURATION, session_type);
    } else {
        generate_sector_times(car, MIN_TIME, MAX_TIME, session_type);
        if (rand() % 500 < 1) car->out = 1; // 1/500 de chance de panne
    }
}

int compare_function(const void *a, const void *b) {
    car_t *car_a = (car_t *)a;
    car_t *car_b = (car_t *)b;
    // Vérifie si c'est une course ou un sprint
    if (strcmp(current_session_type, "course") == 0 || strcmp(current_session_type, "sprint") == 0) {
        // Place les voitures "out" en dernier
        if (car_a->out && car_b->out) return 0; // Les deux sont "out", ordre inchangé
        if (car_a->out) return 1;               // "car_a" est "out", elle passe après
        if (car_b->out) return -1;              // "car_b" est "out", elle passe après

        // Si aucune voiture n'est "out", comparer par nb_tours
        if (car_a->nb_tours != car_b->nb_tours) {
            return (car_b->nb_tours - car_a->nb_tours); // Plus de tours = mieux classée
        }

        // Si le nombre de tours est égal, comparer par temps_rouler
        return (car_a->temps_rouler > car_b->temps_rouler) - (car_a->temps_rouler < car_b->temps_rouler);
    } else {
        // Si ce n'est pas une course ou un sprint, comparer par best_lap_time
        return (car_a->best_lap_time > car_b->best_lap_time) - (car_a->best_lap_time < car_b->best_lap_time);
    }
}