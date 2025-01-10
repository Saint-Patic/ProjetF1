#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "../include/car.h"
#include "../include/utils.h"
#include "../include/display.h"  
#include "../include/simulate.h"  
#include "../include/file_manager.h"

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



extern char *current_session_type;

void append_to_buffer(char **buffer, size_t *buffer_size, size_t *current_length, const char *str) {
    size_t str_length = strlen(str);
    if (*current_length + str_length >= *buffer_size) {
        *buffer_size += BUFFER_INCREMENT;
        *buffer = realloc(*buffer, *buffer_size);
        if (!*buffer) {
            perror("ERREUR: problème de mémoire non allouée");
            exit(EXIT_FAILURE);
        }
    }
    strcat(*buffer, str);
    *current_length += str_length;
}

void display_practice_results(car_t cars[], int num_cars, char *session_type, char *ville) {
    // Détermine les colonnes et la méthode de tri selon le type de session
    char *nom_de_colonne = strcmp(session_type, "course") == 0 || strcmp(session_type, "sprint") == 0 ? " Temps rouler " : " Meilleur tour ";

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

    append_to_buffer(&buffer, &buffer_size, &current_length, header);

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
        char *etat = cars[i].out ? MAGENTA " (Out)" RESET : cars[i].pit_stop ? BLUE " (P)" RESET : "";
        // Crée la ligne correspondante
        snprintf(line, sizeof(line),
            CYAN "|" RESET " " RED "%3d" RESET "  " CYAN "|" RESET "  %7.2f    " CYAN "|" RESET "  %7.2f    " CYAN "|" RESET "  %7.2f    " CYAN "|" RESET "    %7.2f     " CYAN "|" RESET "     %7.2f      " CYAN "|" RESET "  %+5.2f   %s \n",
            cars[i].car_number,
            cars[i].sector_times[0],
            cars[i].sector_times[1],
            cars[i].sector_times[2],
            cars[i].current_lap,
            current_value,
            i == 0 ? 0.00 : diff, 
            etat);

        append_to_buffer(&buffer, &buffer_size, &current_length, line);
    }

    // Ajoute la ligne de fin
    const char *footer =
        GREEN "================================================================================================" RESET "\n";
    append_to_buffer(&buffer, &buffer_size, &current_length, footer);
    clear_screen();
    printf(GREEN "\n===" RESET YELLOW " Résultats de la session %s - %s " RESET GREEN " ===\n" RESET, session_type, ville);

    // Affiche tout le tableau
    printf("%s", buffer);

    // Libère la mémoire
    free(buffer);
}



void display_overall_best_times(car_t cars[], int num_cars, char *session_type) {
    float overall_best_sector_times[NUM_SECTORS];
    int overall_best_sector_car[NUM_SECTORS]; // Ajout du tableau pour les numéros des voitures des meilleurs secteurs

    for (int i = 0; i < NUM_SECTORS; i++) {
        overall_best_sector_times[i] = cars[num_cars].best_sector_times[i];
        overall_best_sector_car[i] = cars[num_cars].best_cars_sector[i];
    }

    float overall_best_lap_time = cars[num_cars].best_lap_time;
    int best_lap_car = cars[num_cars].best_cars_tour; // Ajout du numéro de la voiture au meilleur tour

    const char *header_format = GREEN"=== "YELLOW"Meilleurs temps par section et général pour %s"GREEN" ===\n"RESET;
    const char *sector_format = MAGENTA"Meilleur temps secteur %d :"RESET" Voiture n°%d en "RED"%.2f"RESET" secondes\n";
    const char *lap_format = MAGENTA"Meilleur temps de tour global :"RESET" Voiture n°%d en "RED"%.2f"RESET" secondes\n";
    const char *footer_format = GREEN"=======================================================\n"RESET;

    printf(header_format, session_type);
    for (int i = 0; i < NUM_SECTORS; i++) {
        printf(sector_format, i + 1, overall_best_sector_car[i], overall_best_sector_times[i]);
    }
    printf(lap_format, best_lap_car, overall_best_lap_time);
    printf("%s", footer_format);
}



void display_starting_grid(int car_numbers[], int num_cars) {
    clear_screen();
    printf("=== Starting Grid ===\n");
    for (int i = 0; i < num_cars; i++) {
        printf("Position %d: Car Number %d\n", i + 1, car_numbers[i]);
    }
    printf("=====================\n");
}

void display_points(const car_t cars[], int car_count) {
    clear_screen();
    char *suffixe;
    printf("\n=== Résultats finaux ===\n");
    for (int i = 0; i < car_count; i++) {
        suffixe = cars[i].nb_points > 1 ? "points" : "point";
        printf("Voiture n°%d : %d %s\n", cars[i].car_number, cars[i].nb_points, suffixe);
    }
}