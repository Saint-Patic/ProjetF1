#ifndef UTILS_H
#define UTILS_H


// Déclarations des fonctions utilitaires
void initialize_random();
float random_float(int min, int max);
int estimate_max_laps(int session_duration, float max_time);
char *trim(char *str);
int verifier_parametres(char *session_file, char *ville, char *session_type, int *session_num, int *directory_num);
int ternaire_moins_criminel(int session_num, int res1, int res2, int resDefault);
int is_special_weekend(const char *ville);
int calculate_total_laps(const char *ville, float session_distance);
int verifier_dossier_precedent(char *ville);
int check_file_exists(const char *file_path, const char *error_message);
int check_session_num(int session_num, int max_num, const char *error_message);
void clear_screen();
#endif