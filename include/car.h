#ifndef CAR_H
#define CAR_H
#define NUM_SECTORS 3
#define MIN_PIT_STOP_DURATION 14 // Durée minimale du pit stop en secondes
#define MAX_PIT_STOP_DURATION 16 // Durée maximale du pit stop en secondes
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
#define SHM_KEY 12345 // Clé pour la mémoire partagée

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
    int nb_tours; // nombre de tours effectués

    // pour voiture fictive
    int best_cars_sector[NUM_SECTORS]; // Numéro des meilleures voitutres par secteurs
    int best_cars_tour; // Numéro des meilleures voitures au tour
} car_t;


void generate_sector_times(car_t *car, int min_time, int max_time, char *session_type);
void update_best_times(car_t *car, int sector_index);
void handle_pit_stop(car_t *car, int lap, int total_laps, char *session_type);
void initialize_cars(car_t cars[], int car_numbers[]);
void find_overall_best_times(car_t cars[], int num_cars);
void gestion_points(car_t cars[], const char *input_file, const char *output_file, const char *type_session);
void init_semaphore();
void destroy_semaphore();
void enter_critical_section_reader();
void exit_critical_section_reader();
void enter_critical_section_writer();
void exit_critical_section_writer();
int compare_function(const void *a, const void *b);


#endif // CAR_H