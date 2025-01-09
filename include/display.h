#ifndef DISPLAY_H
#define DISPLAY_H
#define BUFFER_INCREMENT 1024

void display_practice_results(car_t cars[], int num_cars, char *session_type, char *ville);
void display_overall_best_times(car_t cars[], int num_cars, char *session_type);
void display_starting_grid(int car_numbers[], int num_cars);
void display_points(const car_t cars[], int car_count);
void append_to_buffer(char **buffer, size_t *buffer_size, size_t *current_length, const char *str);


#endif //DISPLAY_H  