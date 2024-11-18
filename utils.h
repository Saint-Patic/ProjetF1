#ifndef UTILS_H
#define UTILS_H


// Déclarations des fonctions utilitaires
void initialize_random();
float random_float(int min, int max);
int estimate_max_laps(int session_duration, int max_time);
char *trim(char *str);

#endif