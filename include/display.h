#ifndef DISPLAY_H
#define DISPLAY_H
#define BUFFER_INCREMENT 1024

void display_practice_results(car_t cars[], int num_cars, char *session_type);
void display_overall_best_times(car_t cars[], int num_cars, char *session_type);
void display_starting_grid(int car_numbers[], int num_cars);
void display_points(const car_t cars[], int car_count);


#endif //DISPLAY_H  