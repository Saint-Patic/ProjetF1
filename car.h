#ifndef CAR_H
#define CAR_H

#define NUM_SECTORS 3
#define MIN_PIT_STOP_DURATION 25 // Durée minimale du pit stop en secondes
#define MAX_PIT_STOP_DURATION 45 // Durée maximale du pit stop en secondes
#define MAX_TIME_LAP 3*MAX_TIME // Temps maximum d'un tour en secondes
#define DUREE_ESSAI 3600 // Temps maximum de la période d'essai

struct CarTime {
    int car_number;
    int pit_stop; // 1 si en pit stop, 0 sinon
    int out; // 1 si hors course, 0 sinon
    float sector_times[NUM_SECTORS];
    float best_lap_time;
    float best_sector_times[NUM_SECTORS]; // Meilleur temps pour chaque secteur
    int pit_stop_duration; // Durée du pit stop en tours
    float temps_rouler; // Temps de roulage total en secondes
};

void generate_sector_times(struct CarTime *car, int min_time, int max_time);
void simulate_practice_session(struct CarTime cars[], int num_cars, int min_time, int max_time);
void initialize_cars(struct CarTime cars[], int car_numbers[], int num_cars, int max_time);
void display_practice_results(struct CarTime cars[], int num_cars);
void display_overall_best_times(struct CarTime cars[], int num_cars);
int compare_cars(const void *a, const void *b);

#endif // CAR_H
