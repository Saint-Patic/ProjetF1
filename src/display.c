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

void display_practice_results(car_t cars[], int num_cars) {
    qsort(cars, num_cars, sizeof(car_t), compare_cars);

    printf(GREEN "===========================================================================" RESET "\n");
    printf(CYAN"|"RESET"  "RED"##"RESET"  "CYAN"|"RESET"  "YELLOW"Secteur 1"RESET"  "CYAN"|"RESET"  "YELLOW"Secteur 2"RESET"  "CYAN"|"RESET"  "YELLOW"Secteur 3"RESET"  "CYAN"|"RESET"   Tour (s)  "CYAN"|"RESET"" REDBG "   Diff   " RESET ""CYAN"|"RESET"\n");
    printf(GREEN "===========================================================================" RESET "\n");

    float prev_time = 0;
    for (int i = 0; i < num_cars; i++) {
        if (i == 0) prev_time = cars[i].best_lap_time;

        float diff = cars[i].best_lap_time - prev_time;
        printf(CYAN"|"RESET" "RED"%3d"RESET"  "CYAN"|"RESET"   %7.2f   "CYAN"|"RESET"   %7.2f   "CYAN"|"RESET"   %7.2f   "CYAN"|"RESET"   %7.2f   "CYAN"|"RESET"   %+5.2f  "CYAN"|"RESET"\n",
               cars[i].car_number,
               cars[i].sector_times[0], //secteur 1
               cars[i].sector_times[1], //secteur 2
               cars[i].sector_times[2], //secteur 3
               cars[i].best_lap_time,
               i == 0 ? 0.00 : diff);

        if (cars[i].pit_stop) {
            printf(BLUE" (P)\n"RESET);
        } else if (cars[i].out) {
            printf(MAGENTA" (Out)\n"RESET);
        }
        prev_time = cars[i].best_lap_time;
    }
    printf(GREEN "=================================================================" RESET "\n");
}

void display_overall_best_times(car_t cars[], int num_cars) {
    float overall_best_sector_times[NUM_SECTORS] = {9999.0, 9999.0, 9999.0};
    int overall_best_sector_car[NUM_SECTORS] = {-1, -1, -1}; // Ajout du tableau pour les numéros des voitures des meilleurs secteurs
    float overall_best_lap_time = 9999.0;
    int best_lap_car = -1; // Ajout du numéro de la voiture au meilleur tour

    for (int i = 0; i < num_cars; i++) {
        for (int j = 0; j < NUM_SECTORS; j++) {
            if (cars[i].best_sector_times[j] > 0 && cars[i].best_sector_times[j] < overall_best_sector_times[j]) {
                overall_best_sector_times[j] = cars[i].best_sector_times[j];
                overall_best_sector_car[j] = cars[i].car_number; // Stockez le numéro de la voiture
            }
        }

        if (cars[i].best_lap_time > 0 && cars[i].best_lap_time < overall_best_lap_time) {
            overall_best_lap_time = cars[i].best_lap_time;
            best_lap_car = cars[i].car_number; // Stockez le numéro de la voiture pour le meilleur tour global
        }
    }

    printf("\n=== Meilleurs temps par section et général de la période d'essai ===\n");
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