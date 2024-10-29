#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h> 
#include "car.h"
#include "utils.h" 
#include "display.h"
#include "file_manager.h"

void generate_sector_times(struct CarTime *car, int min_time, int max_time) {
    car->current_lap = 0.0;
    for (int i = 0; i < NUM_SECTORS; i++) {
        car->sector_times[i] = random_float(min_time, max_time);
        car->current_lap += car->sector_times[i];

        // Mise à jour du meilleur temps par secteur
        if (car->best_sector_times[i] == 0 || car->sector_times[i] < car->best_sector_times[i]) {
            car->best_sector_times[i] = car->sector_times[i];
        }
    }

    // Mise à jour du meilleur temps au tour
    if (car->best_lap_time == 0 || car->current_lap < car->best_lap_time) {
        car->best_lap_time = car->current_lap;
    }
}

int compare_cars(const void *a, const void *b) {
    struct CarTime *carA = (struct CarTime *)a;
    struct CarTime *carB = (struct CarTime *)b;
    
    return (carA->current_lap > carB->current_lap) - (carA->current_lap < carB->current_lap);
}

void reset_out_status_and_temps_rouler(struct CarTime cars[], int num_cars) { // Réinitialise le statut "Out" et le temps de roulage des voitures après chaque séance
    for (int i = 0; i < num_cars; i++) {
        cars[i].out = 0;
        cars[i].temps_rouler = 0;
    }
}

void simulate_practice(struct CarTime cars[], int num_cars, int min_time, int max_time, int session_duration) {
    int total_laps = MAX_LAPS;
    for (int lap = 0; lap < total_laps; lap++) {
        for (int i = 0; i < num_cars; i++) {
            if (cars[i].out) continue;

            if (cars[i].pit_stop) {
                int pit_stop_time = random_float(min_time, max_time);
                if (cars[i].pit_stop_duration > pit_stop_time) {
                    cars[i].pit_stop_duration -= pit_stop_time;
                    continue;
                } else {
                    cars[i].pit_stop = 0;
                }
            }

            generate_sector_times(&cars[i], min_time, max_time);
            cars[i].temps_rouler += max_time;

            if (cars[i].temps_rouler >= session_duration) {
                for (int j = 0; j < num_cars; j++) {
                    cars[j].out = 1;
                }
                reset_out_status_and_temps_rouler(cars, num_cars);
                return;
            }

            if (rand() % 100 < 35) {
                cars[i].pit_stop_duration = random_float(MIN_PIT_STOP_DURATION, MAX_PIT_STOP_DURATION);
                cars[i].pit_stop = 1;
            } else if (rand() % 100 < 1) {
                cars[i].out = 1;
            }
        }

        system("clear");
        printf("Tour %d:\n", lap + 1);
        display_practice_results(cars, num_cars);
        sleep(1);
    }
    reset_out_status_and_temps_rouler(cars, num_cars);
}

