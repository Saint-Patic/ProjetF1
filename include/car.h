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
#define MAX_NUM_CARS 21
#define MIN_TIME 25
#define MAX_TIME 45
#define SESSION_DISTANCE 300
#define SPRINT_DISTANCE 100
#define SEM_KEY 1234
#define POINTS_SPRINT {8, 7, 6, 5, 4, 3, 2, 1}
#define POINTS_COURSE {25, 18, 15, 12, 10, 8, 6, 4, 2, 1}

typedef struct car{
    int car_number;
    int out; // 1 si hors course, 0 sinon
    float sector_times[NUM_SECTORS];
    float best_sector_times[NUM_SECTORS]; // Meilleur temps pour chaque secteur
    float current_lap;
    float best_lap_time;
    int pit_stop; // 1 si en pit stop, 0 sinon
    int pit_stop_duration; // Durée du pit stop en tours
    int pit_stop_nb; // nombre de pit stop effectué
    float temps_rouler; // Temps de roulage total en secondes
    int eliminate; // true si elimine pdt qualif
    int nb_points; // nombre de points obtenus lors de la simul

    // pour voiture fictive
    int best_cars_sector[NUM_SECTORS]; // Numéro des meilleures voitutres par secteurs
    int best_cars_tour; // Numéro des meilleures voitures au tour
} car_t;


void generate_sector_times(car_t *car, int min_time, int max_time);
void simulate_sess(car_t cars[], int num_cars, int session_duration, int total_laps, char *session_type);
int compare_cars(const void *a, const void *b);
void simulate_pit_stop(car_t *car, int min_time, int max_time, char *session_type);
void simulate_qualification(car_t cars[], int session_num, const char *ville, char *filename, char *session_type);
void simulate_course(car_t cars[], const char *ville, char *session_type, char *session_file);
void initialize_cars(car_t cars[], int car_numbers[]);
void find_overall_best_times(car_t cars[], int num_cars);
void gestion_points(car_t cars[], const char *input_file, const char *output_file, const char *type_session);
int compare_tour_cars(const void *a, const void *b);
void init_semaphore();
void destroy_semaphore();



#endif // CAR_H