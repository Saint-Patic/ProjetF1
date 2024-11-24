#ifndef CAR_H
#define CAR_H
#define NUM_SECTORS 3
#define MIN_PIT_STOP_DURATION 24 // Durée minimale du pit stop en secondes
#define MAX_PIT_STOP_DURATION 26 // Durée maximale du pit stop en secondes
#define MAX_TIME_LAP 3*MAX_TIME // Temps maximum d'un tour en secondes
#define DUREE_QUALIF_1 1080 // Temps maximum de la période d'essai
#define DUREE_QUALIF_2 900 // Temps maximum de la période d'essai
#define DUREE_QUALIF_3 720 // Temps maximum de la période d'essai
#define DUREE_ESSAI 3600 // Temps maximum de la période d'essai
#define NUM_CARS 20
#define MIN_TIME 25
#define MAX_TIME 45
#define SESSION_DISTANCE 300
#define SPRINT_DISTANCE 100
#define SEM_KEY 1234

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


void generate_sector_times(car_t *car, int min_time, int max_time);
void simulate_sess(car_t cars[], int num_cars, int session_duration, int total_laps, char *session_type);
void display_practice_results(car_t cars[], int num_cars);
void display_overall_best_times(car_t cars[], int num_cars);
int compare_cars(const void *a, const void *b);
void reset_out_status_and_temps_rouler(car_t cars[], int num_cars);
void simulate_pit_stop(car_t *car, int min_time, int max_time, char *session_type);
void simulate_qualification(car_t cars[], int session_num, const char *ville, int special_weekend, char *filename, char *session_type);
void simulate_course(car_t cars[], int special_weekend, int session_num, const char *ville, char *session_type);
void initialize_cars(car_t cars[], int car_numbers[], int num_cars);



#endif // CAR_H