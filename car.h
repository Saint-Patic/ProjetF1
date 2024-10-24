#ifndef CAR_H
#define CAR_H

#define NUM_SECTORS 3

struct CarTime {
    int car_number;
    float sector_times[NUM_SECTORS];
    float best_lap_time;
    int pit_stop;
    int out;
};

// Déclarations des fonctions
void initialize_cars(struct CarTime cars[], int car_numbers[], int num_cars, int max_time);
void generate_sector_times(struct CarTime *car, int min_time, int max_time);
void simulate_practice_session(struct CarTime cars[], int num_cars, int min_time, int max_time);
void display_practice_results(struct CarTime cars[], int num_cars);
int compare_cars(const void *a, const void *b);

#endif
