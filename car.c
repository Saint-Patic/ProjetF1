#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h> 
#include "car.h"
#include "utils.h" 
#include "display.h"
#include "file_manager.h"

void generate_sector_times(struct CarTime *car, int min_time, int max_time) {
    car->best_lap_time = 0;
    for (int i = 0; i < NUM_SECTORS; i++) {
        car->sector_times[i] = random_float(min_time, max_time);
        car->best_lap_time += car->sector_times[i];

        if (car->best_sector_times[i] == 0 || car->sector_times[i] < car->best_sector_times[i]) {
            car->best_sector_times[i] = car->sector_times[i];
        }
    }

    if (car->best_lap_time < car->best_lap_time || car->best_lap_time == 0) {
        car->best_lap_time = car->best_lap_time;
    }
}

int compare_cars(const void *a, const void *b) {
    struct CarTime *carA = (struct CarTime *)a;
    struct CarTime *carB = (struct CarTime *)b;
    return (carA->best_lap_time > carB->best_lap_time) ? 1 : -1;
}

void simulate_practice_session(struct CarTime cars[], int num_cars, int min_time, int max_time) {
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
            cars[i].temps_rouler += (float)max_time;

            if (cars[i].temps_rouler >= 3600) {
                for (int j = 0; j < num_cars; j++) {
                    cars[j].out = 1;
                }
                reset_out_status_and_temps_rouler(cars, num_cars);
                return;
            }

            if (rand() % 100 < 35) {
                cars[i].pit_stop_duration = random_float(MIN_PIT_STOP_DURATION, MAX_PIT_STOP_DURATION);
                cars[i].pit_stop = 1;
            } else if (rand() % 100 < 1) { // Probabilité plus faible d'être "Out"
                cars[i].out = 1;
            }
        }

        system("clear");
        printf("Tour %d:\n", lap + 1);
        display_practice_results(cars, num_cars);
        sleep(1);
    }
    save_session_results(cars, num_cars, "session_results.csv");

    reset_out_status_and_temps_rouler(cars, num_cars);
}

void reset_out_status_and_temps_rouler(struct CarTime cars[], int num_cars) { // Réinitialise le statut "Out" et le temps de roulage des voitures après chaque séance
    for(int i = 0; i < num_cars; i++){
        printf("Voiture %d - Temps roulé : %.2f secondes\n", cars[i].car_number, cars[i].temps_rouler);
    }
    for (int i = 0; i < num_cars; i++) {
        cars[i].out = 0;
        cars[i].temps_rouler = 0;
    }
}
