#ifndef CAR_H
#define CAR_H

#include <semaphore.h>

#define NUM_SECTORS 3
#define MIN_PIT_STOP_DURATION 24 // Durée minimale du pit stop en secondes
#define MAX_PIT_STOP_DURATION 26 // Durée maximale du pit stop en secondes
#define MAX_TIME_LAP 3*MAX_TIME // Temps maximum d'un tour en secondes
#define DUREE_ESSAI 3600 // Temps maximum de la période d'essai

typedef struct car{
    int car_number;
    int out; // 1 si hors course, 0 sinon
    float sector_times[NUM_SECTORS];
    float best_lap_time;
    float best_sector_times[NUM_SECTORS]; // Meilleur temps pour chaque secteur
    int pit_stop; // 1 si en pit stop, 0 sinon
    int pit_stop_duration; // Durée du pit stop en tours
    int pit_stop_nb; // nombre de pit stop effectué
    float temps_rouler; // Temps de roulage total en secondes
    int eliminate; // true si elimine pdt qualif
} car_t;

typedef struct {
    car_t *car;
    int min_time;
    int max_time;
} lap_args_t;

void generate_sector_times(car_t *car, int min_time, int max_time);
void simulate_sess(car_t cars[], int num_cars, int min_time, int max_time, int session_duration, int total_laps, char *session_type);
void display_practice_results(car_t cars[], int num_cars);
void display_overall_best_times(car_t cars[], int num_cars);
int compare_cars(const void *a, const void *b);
void reset_out_status_and_temps_rouler(car_t cars[], int num_cars);
void simulate_pit_stop(car_t *car, int min_time, int max_time, char *session_type);
void simulate_qualification(car_t cars[], int session_num, const char *ville, int min_time, int max_time, int total_cars);
void simulate_course(int distance, int min_time, int max_time, int total_laps);
int ternaire_moins_criminel(int session_num, int res1, int res2, int resDefault);

extern sem_t sem; // Declare the semaphore

#endif // CAR_H