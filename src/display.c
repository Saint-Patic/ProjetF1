#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "../include/car.h"
#include "../include/utils.h"
#include "../include/display.h"  

#define RESET   "\x1b[0m"
#define RED   "\x1b[31m"
#define GREEN   "\x1b[32m" 
#define YELLOW  "\x1b[33m" 
#define BLUE    "\x1b[34m" 
#define MAGENTA "\x1b[35m" 
#define CYAN    "\x1b[36m" 
#define REDBG   "\x1b[41m"
#define GREENBG "\x1b[42m" 
#define YELLOWBG "\x1b[43m" 
#define BLUEBG "\x1b[44m" 
#define MAGENTABG "\x1b[45m" 
#define CYANBG "\x1b[46m"



void display_practice_results(car_t cars[], int num_cars, char *session_type) {
    int compare_function(const void *a, const void *b) {
        car_t *car_a = (car_t *)a;
        car_t *car_b = (car_t *)b;

        // Sinon, tri par temps (temps_rouler ou best_lap_time selon le contexte)
        if (strcmp(session_type, "course") == 0 || strcmp(session_type, "sprint") == 0) {
            // Place les voitures "out" en dernier
            if (car_a->out && car_b->out) return 0; // Les deux sont "out", ordre inchangé
            if (car_a->out) return 1; // "car_a" est "out", elle passe après
            if (car_b->out) return -1; // "car_b" est "out", elle passe après
            return (car_a->temps_rouler > car_b->temps_rouler) - (car_a->temps_rouler < car_b->temps_rouler);
        } else {
            return (car_a->best_lap_time > car_b->best_lap_time) - (car_a->best_lap_time < car_b->best_lap_time);
        }
    }

    // Détermine les colonnes et la méthode de tri selon le type de session
    char *nom_de_colonne;
    if (strcmp(session_type, "course") == 0 || strcmp(session_type, "sprint") == 0) {
        nom_de_colonne = " Temps rouler ";
    } else {
        nom_de_colonne = " Meilleur tour ";
    }

    // Trie les voitures selon la fonction définie
    qsort(cars, num_cars, sizeof(car_t), compare_function);

    // Initialisation d'un buffer dynamique avec une taille initiale
    size_t buffer_size = BUFFER_INCREMENT;
    char *buffer = malloc(buffer_size);
    if (!buffer) {
        perror("ERREUR: problème de mémoire non allouée");
        exit(EXIT_FAILURE);
    }

    // Initialise le buffer
    buffer[0] = '\0';
    size_t current_length = 0;

    // Construit l'en-tête dynamique
    char header[512];
    snprintf(header, sizeof(header),
        GREEN "================================================================================================" RESET "\n"
        CYAN "|" RESET RED "  ##  " RESET CYAN "|" RESET YELLOW "  Secteur 1  " RESET CYAN "|" RESET YELLOW "  Secteur 2  " RESET CYAN "|" RESET YELLOW "  Secteur 3  " RESET CYAN "|" RESET "   Tour Actuel  " CYAN "|" RESET "  %s  " CYAN "|" RESET REDBG "   Diff   " RESET "\n"
        GREEN "================================================================================================" RESET "\n",
        nom_de_colonne);

    size_t header_length = strlen(header);
    if (current_length + header_length >= buffer_size) {
        buffer_size += header_length;
        buffer = realloc(buffer, buffer_size);
        if (!buffer) {
            perror("ERREUR: problème de mémoire non allouée");
            exit(EXIT_FAILURE);
        }
    }
    strcat(buffer, header);
    current_length += header_length;

    // Ajoute les résultats ligne par ligne
    float prev_value = (strcmp(session_type, "course") == 0 || strcmp(session_type, "sprint") == 0)
                           ? cars[0].temps_rouler
                           : cars[0].best_lap_time;

    for (int i = 0; i < num_cars; i++) {
        float current_value = (strcmp(session_type, "course") == 0 || strcmp(session_type, "sprint") == 0)
                                  ? cars[i].temps_rouler
                                  : cars[i].best_lap_time;

        float diff = current_value - prev_value;
        char line[512];

        // Crée la ligne correspondante
        if (cars[i].pit_stop) {
            snprintf(line, sizeof(line),
                CYAN "|" RESET " " RED "%3d" RESET "  " CYAN "|" RESET "  %7.2f    " CYAN "|" RESET "  %7.2f    " CYAN "|" RESET "  %7.2f    " CYAN "|" RESET "    %7.2f     " CYAN "|" RESET "     %7.2f      " CYAN "|" RESET "  %+5.2f    " BLUE " (P)" RESET "\n",
                cars[i].car_number,
                cars[i].sector_times[0],
                cars[i].sector_times[1],
                cars[i].sector_times[2],
                cars[i].current_lap,
                current_value,
                i == 0 ? 0.00 : diff);
        } else if (cars[i].out) {
            snprintf(line, sizeof(line),
                CYAN "|" RESET " " RED "%3d" RESET "  " CYAN "|" RESET "  %7.2f    " CYAN "|" RESET "  %7.2f    " CYAN "|" RESET "  %7.2f    " CYAN "|" RESET "    %7.2f     " CYAN "|" RESET "     %7.2f      " CYAN "|" RESET "  %+5.2f     " MAGENTA " (Out)" RESET "\n",
                cars[i].car_number,
                cars[i].sector_times[0],
                cars[i].sector_times[1],
                cars[i].sector_times[2],
                cars[i].current_lap,
                current_value,
                i == 0 ? 0.00 : diff);
        } else {
            snprintf(line, sizeof(line),
                CYAN "|" RESET " " RED "%3d" RESET "  " CYAN "|" RESET "  %7.2f    " CYAN "|" RESET "  %7.2f    " CYAN "|" RESET "  %7.2f    " CYAN "|" RESET "    %7.2f     " CYAN "|" RESET "     %7.2f      " CYAN "|" RESET "  %+5.2f   \n",
                cars[i].car_number,
                cars[i].sector_times[0],
                cars[i].sector_times[1],
                cars[i].sector_times[2],
                cars[i].current_lap,
                current_value,
                i == 0 ? 0.00 : diff);
        }

        size_t line_length = strlen(line);
        if (current_length + line_length >= buffer_size) {
            buffer_size += BUFFER_INCREMENT;
            buffer = realloc(buffer, buffer_size);
            if (!buffer) {
                perror("ERREUR: problème de mémoire non allouée");
                exit(EXIT_FAILURE);
            }
        }
        strcat(buffer, line);
        current_length += line_length;

        prev_value = current_value;
    }

    // Ajoute la ligne de fin
    const char *footer =
        GREEN "================================================================================================" RESET "\n";
    size_t footer_length = strlen(footer);
    if (current_length + footer_length >= buffer_size) {
        buffer_size += footer_length;
        buffer = realloc(buffer, buffer_size);
        if (!buffer) {
            perror("ERREUR: problème de mémoire non allouée");
            exit(EXIT_FAILURE);
        }
    }
    strcat(buffer, footer);

    printf(GREEN "\n===" RESET YELLOW " Résultats de la session %s " RESET GREEN " ===\n" RESET, session_type);

    // Affiche tout le tableau
    printf("%s", buffer);

    // Libère la mémoire
    free(buffer);
}



void display_overall_best_times(car_t cars[], int num_cars, char *session_type) {
    float overall_best_sector_times[NUM_SECTORS] = {cars[num_cars].best_sector_times[0],cars[num_cars].best_sector_times[1], cars[num_cars].best_sector_times[2]};
    int overall_best_sector_car[NUM_SECTORS] = {cars[num_cars].best_cars_sector[0], cars[num_cars].best_cars_sector[1], cars[num_cars].best_cars_sector[2]}; // Ajout du tableau pour les numéros des voitures des meilleurs secteurs
    float overall_best_lap_time = cars[num_cars].best_lap_time;
    int best_lap_car = cars[num_cars].best_cars_tour; // Ajout du numéro de la voiture au meilleur tour

    printf("\n=== Meilleurs temps par section et général pour %s ===\n", session_type);
    for (int i = 0; i < NUM_SECTORS; i++) {
        printf(MAGENTA"Meilleur temps secteur %d :"RESET" Voiture n°%d en "RED"%.2f"RESET" secondes\n", i + 1, overall_best_sector_car[i], overall_best_sector_times[i]);
    }
    printf(MAGENTA"Meilleur temps de tour global :"RESET" Voiture n°%d en "CYAN"%.2f"RESET" secondes\n", best_lap_car, overall_best_lap_time);
    printf(GREEN"=======================================================\n"RESET);
}

void display_starting_grid(int car_numbers[], int num_cars) {
    printf("=== Starting Grid ===\n");
    for (int i = 0; i < num_cars; i++) {
        printf("Position %d: Car Number %d\n", i + 1, car_numbers[i]);
    }
    printf("=====================\n");
}