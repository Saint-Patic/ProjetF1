#ifndef UTILS_H
#define UTILS_H


// Déclarations des fonctions utilitaires
void initialize_random();
float random_float(int min, int max);
int estimate_max_laps(int session_duration, float max_time);
char *trim(char *str);
int verifier_parametres(char *session_file, char *ville, char *session_type, int *session_num, int *directory_num);


#endif