#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "car.h"
#include "utils.h"
#include "display.h"
#include "file_manager.h"

void initialize_random() {
    srand(time(NULL));
}

float random_float(int min, int max) {
    return min + (float)rand() / RAND_MAX * (max - min); 
}


int estimate_max_laps(int session_duration, int max_time) {
    return session_duration / max_time;
}


void load_eliminated_cars(char *filename, struct CarTime cars[], int total_cars) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Erreur lors de l'ouverture de %s\n", filename);
        return;
    }
    
    int car_number, session_num, place;
    float lap_time;
    char line[100];
    
    // Mark all cars as eligible initially
    for (int i = 0; i < total_cars; i++) {
        cars[i].out = 0;
    }
    
    // Read each line to mark cars that are already eliminated
    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%d,%d,%d,%f", &car_number, &session_num, &place, &lap_time);
        for (int i = 0; i < total_cars; i++) {
            if (cars[i].car_number == car_number && place > 15) { // Cars with place > 15 are eliminated
                cars[i].out = 1;
                break;
            }
        }
    }
    fclose(file);
}