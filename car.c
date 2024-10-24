#include <stdio.h>
#include <stdlib.h>
#include "car.h"
#include "utils.h"

void initialize_cars(struct CarTime cars[], int car_numbers[], int num_cars, int max_time) {
    for (int i = 0; i < num_cars; i++) {
        cars[i].car_number = car_numbers[i];
        cars[i].pit_stop = 0;
        cars[i].out = 0;
        cars[i].best_lap_time = max_time * 3;
    }
}

void generate_sector_times(struct CarTime *car, int min_time, int max_time) {
    car->best_lap_time = 0;
    for (int i = 0; i < NUM_SECTORS; i++) {
        car->sector_times[i] = random_float(min_time, max_time);
        car->best_lap_time += car->sector_times[i];
    }
}

void simulate_practice_session(struct CarTime cars[], int num_cars, int min_time, int max_time) {
    for (int lap = 0; lap < MAX_LAPS; lap++) {
        for (int i = 0; i < num_cars; i++) {
            if (cars[i].out || cars[i].pit_stop) {
                continue;
            }
            generate_sector_times(&cars[i], min_time, max_time);

            // Random pit or out event
            if (rand() % 100 < 10) {
                cars[i].pit_stop = 1;
            } else if (rand() % 100 < 5) {
                cars[i].out = 1;
            }
        }
    }
}

void display_practice_results(struct CarTime cars[], int num_cars) {
    qsort(cars, num_cars, sizeof(struct CarTime), compare_cars);
    
    printf("=================================================================\n");
    printf("|  #   |  Secteur 1  |  Secteur 2  |  Secteur 3  |   Tour   | Diff |\n");
    printf("=================================================================\n");

    float prev_time = 0;
    for (int i = 0; i < num_cars; i++) {
        if (i == 0) prev_time = cars[i].best_lap_time;

        float diff = cars[i].best_lap_time - prev_time;
        printf("| %3d  |   %7.2f   |   %7.2f   |   %7.2f   |  %7.2f  |  %+5.2f |\n",
               cars[i].car_number,
               cars[i].sector_times[0],
               cars[i].sector_times[1],
               cars[i].sector_times[2],
               cars[i].best_lap_time,
               i == 0 ? 0.00 : diff);

        if (cars[i].pit_stop) {
            printf(" (P)\n");
        } else if (cars[i].out) {
            printf(" (Out)\n");
        }
        prev_time = cars[i].best_lap_time;
    }
    printf("=================================================================\n");
}

int compare_cars(const void *a, const void *b) {
    struct CarTime *carA = (struct CarTime *)a;
    struct CarTime *carB = (struct CarTime *)b;
    return (carA->best_lap_time > carB->best_lap_time) ? 1 : -1;
}
