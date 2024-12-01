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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/car.h"
#include "../include/display.h"

#define BUFFER_INCREMENT 1024

void display_practice_results(car_t cars[], int num_cars) {
    // Trie les voitures par temps
    qsort(cars, num_cars, sizeof(car_t), compare_cars);

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

    // Ajoute l'en-tête au buffer
    const char *header =
        GREEN "================================================================================================" RESET "\n"
        CYAN "|" RESET RED "  ##  " RESET CYAN "|" RESET YELLOW "  Secteur 1  " RESET CYAN "|" RESET YELLOW "  Secteur 2  " RESET CYAN "|" RESET YELLOW "  Secteur 3  " RESET CYAN "|" RESET "   Tour Actuel  " CYAN "|" RESET "   Meilleur tour  " CYAN "|" RESET REDBG "   Diff   " RESET "\n"
        GREEN "================================================================================================" RESET "\n";

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
    float prev_time = cars[0].best_lap_time;
    for (int i = 0; i < num_cars; i++) {
        char line[512];
        float diff = cars[i].best_lap_time - prev_time;

        // Crée la ligne correspondante
        if (cars[i].pit_stop) {
            snprintf(line, sizeof(line),
                CYAN "|" RESET " " RED "%3d" RESET "  " CYAN "|" RESET "  %7.2f    " CYAN "|" RESET "  %7.2f    " CYAN "|" RESET "  %7.2f    " CYAN "|" RESET "    %7.2f     " CYAN "|" RESET "     %7.2f      " CYAN "|" RESET "  %+5.2f    " BLUE " (P)" RESET "\n",
                cars[i].car_number,
                cars[i].sector_times[0],
                cars[i].sector_times[1],
                cars[i].sector_times[2],
                cars[i].current_lap,
                cars[i].best_lap_time,
                i == 0 ? 0.00 : diff);
        } else if (cars[i].out) {
            snprintf(line, sizeof(line),
                CYAN "|" RESET " " RED "%3d" RESET "  " CYAN "|" RESET "  %7.2f    " CYAN "|" RESET "  %7.2f    " CYAN "|" RESET "  %7.2f    " CYAN "|" RESET "    %7.2f     " CYAN "|" RESET "     %7.2f      " CYAN "|" RESET "  %+5.2f     " MAGENTA " (Out)" RESET "\n",
                cars[i].car_number,
                cars[i].sector_times[0],
                cars[i].sector_times[1],
                cars[i].sector_times[2],
                cars[i].current_lap,
                cars[i].best_lap_time,
                i == 0 ? 0.00 : diff);
        } else {
            snprintf(line, sizeof(line),
                CYAN "|" RESET " " RED "%3d" RESET "  " CYAN "|" RESET "  %7.2f    " CYAN "|" RESET "  %7.2f    " CYAN "|" RESET "  %7.2f    " CYAN "|" RESET "    %7.2f     " CYAN "|" RESET "     %7.2f      " CYAN "|" RESET "  %+5.2f   \n",
                cars[i].car_number,
                cars[i].sector_times[0],
                cars[i].sector_times[1],
                cars[i].sector_times[2],
                cars[i].current_lap,
                cars[i].best_lap_time,
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

        prev_time = cars[i].best_lap_time;
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

    // Affiche tout le tableau
    printf("%s", buffer);

    // Libère la mémoire
    free(buffer);
}

void display_overall_best_times(car_t cars[], int num_cars) {
    float overall_best_sector_times[NUM_SECTORS] = {cars[NUM_CARS - 1].best_sector_times[0],cars[NUM_CARS - 1].best_sector_times[1], cars[NUM_CARS - 1].best_sector_times[2]};
    int overall_best_sector_car[NUM_SECTORS] = {-1, -1, -1}; // Ajout du tableau pour les numéros des voitures des meilleurs secteurs
    float overall_best_lap_time = cars[NUM_CARS - 1].best_lap_time;
    int best_lap_car = cars[0].car_number; // Ajout du numéro de la voiture au meilleur tour

    printf("\n=== Meilleurs temps par section et général ===\n");
    for (int i = 0; i < NUM_SECTORS; i++) {
        printf("Meilleur temps secteur %d : Voiture n°%d en "RED"%.2f"RESET" secondes\n", i + 1, overall_best_sector_car[i], overall_best_sector_times[i]);
    }
    printf("Meilleur temps de tour global : Voiture n°%d en "CYAN"%.2f"RESET" secondes\n", best_lap_car, overall_best_lap_time);
    printf("=======================================================\n");
}

void display_starting_grid(int car_numbers[], int num_cars) {
    printf("=== Starting Grid ===\n");
    for (int i = 0; i < num_cars; i++) {
        printf("Position %d: Car Number %d\n", i + 1, car_numbers[i]);
    }
    printf("=====================\n");
}