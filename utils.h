#ifndef UTILS_H
#define UTILS_H


// Déclarations des fonctions utilitaires
void initialize_random();
float random_float(int min, int max);
int estimate_max_laps(int session_duration, int max_time);
void load_eliminated_cars(char *filename, struct CarTime cars[], int total_cars);
char *trim(char *str);

#endif