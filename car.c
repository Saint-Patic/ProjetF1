#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "car.h"
#include "utils.h"
#include "display.h"
#include "file_manager.h"




void generate_sector_times(struct CarTime *car, int min_time, int max_time) {
    float lap_time = 0;
    for (int i = 0; i < NUM_SECTORS; i++) {
        car->sector_times[i] = random_float(min_time, max_time);
        lap_time += car->sector_times[i];

        if (car->best_sector_times[i] == 0 || car->sector_times[i] < car->best_sector_times[i]) {
            car->best_sector_times[i] = car->sector_times[i];
        }
    }

    if (car->best_lap_time == 0 || lap_time < car->best_lap_time) {
        car->best_lap_time = lap_time;
    }
    car->temps_rouler += lap_time;
}

void simulate_pit_stop(struct CarTime *car, int min_time, int max_time) {
    if (car->pit_stop) {
        float pit_stop_time = random_float(min_time, max_time);
        if (car->pit_stop_duration > pit_stop_time) {
            car->pit_stop_duration -= pit_stop_time;
            car->temps_rouler += pit_stop_time;
        } else {
            car->pit_stop = 0;
        }
    }
}

int compare_cars(const void *a, const void *b) {
    struct CarTime *carA = (struct CarTime *)a;
    struct CarTime *carB = (struct CarTime *)b;
    return (carA->best_lap_time > carB->best_lap_time) ? 1 : -1;
}

void reset_out_status_and_temps_rouler(struct CarTime cars[], int num_cars) {
    for (int i = 0; i < num_cars; i++) {
        // printf("Voiture %d - Temps roulé : %.2f secondes\n", cars[i].car_number, cars[i].temps_rouler);
        cars[i].out = 0;
        cars[i].temps_rouler = 0;
    }
}

void simulate_sess(struct CarTime cars[], int num_cars, int min_time, int max_time, int session_duration) {
    int total_laps = estimate_max_laps(session_duration, 3*min_time) + 1;
    for (int lap = 0; lap < total_laps; lap++) {
        for (int i = 0; i < num_cars; i++) {
            if (cars[i].out) continue;

            if (cars[i].pit_stop) {
                simulate_pit_stop(&cars[i], min_time, max_time);
                if (cars[i].pit_stop) continue;
            }

            generate_sector_times(&cars[i], min_time, max_time);

            if (cars[i].temps_rouler >= session_duration) {
                for (int j = 0; j < num_cars; j++) {
                    cars[j].out = 1;
                }
                reset_out_status_and_temps_rouler(cars, num_cars);
                return;
            }

            if (rand() % 100 < 20) { // 20% d'être en pit stop
                cars[i].pit_stop_duration = random_float(MIN_PIT_STOP_DURATION, MAX_PIT_STOP_DURATION);
                cars[i].pit_stop = 1;
            } else if (rand() % 100 < 1) { // 1% d'être en arrêt
                cars[i].out = 1;
            }
        }

        system("clear");
        printf("Tour %d:\n", lap + 1);
        display_practice_results(cars, num_cars);
        sleep(0.2);
    }
    reset_out_status_and_temps_rouler(cars, num_cars);
}



void simulate_qualification(struct CarTime cars[], int session_num, char *session_file, int min_time, int max_time, int total_cars) {
    int num_cars_in_stage = (session_num == 1) ? 20 : (session_num == 2) ? 15 : 10;
    int eliminated_cars_count = (session_num == 1) ? 5 : (session_num == 2) ? 5 : 0;
    int session_duration = (session_num == 1) ? 720 : (session_num == 2) ? 600 : 480;

    // Load eliminated cars from classement.csv
    load_eliminated_cars("fichier_enregistree/classement.csv", cars, total_cars);
    
    struct CarTime eligible_cars[num_cars_in_stage];
    int eligible_index = 0;

    // Collect eligible (non-eliminated) cars for this qualification round
    for (int i = 0; i < total_cars; i++) {
        if (!cars[i].out && eligible_index < num_cars_in_stage) {
            eligible_cars[eligible_index++] = cars[i];
        }
    }

    // Simulate the session
    simulate_sess(eligible_cars, num_cars_in_stage, min_time, max_time, session_duration);

    // Sort the cars in this session based on best lap time
    qsort(eligible_cars, num_cars_in_stage, sizeof(struct CarTime), compare_cars);

    // Save results for this session in CSV format
    save_session_results(eligible_cars, num_cars_in_stage, session_file, "a");

    // Save eliminated cars and update their status in the main car array

    save_eliminated_cars(eligible_cars, num_cars_in_stage, eliminated_cars_count, session_num, cars, total_cars);
}
