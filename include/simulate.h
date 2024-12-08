#ifndef SIMULATE_H
#define SIMULATE_H
#include "car.h"
void simulate_sess(car_t cars[], int num_cars, int session_duration, int total_laps, char *session_type);
void simulate_qualification(car_t cars[], int session_num, const char *ville, char *filename, char *session_type);
void simulate_course(car_t cars[], const char *ville, char *session_type, char *session_file, int  car_numbers[]);
#endif 