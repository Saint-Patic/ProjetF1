#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H
#define MAX_SESSION 3

#include "car.h"

void save_session_results(struct CarTime cars[], int num_cars, const char *filename, const char *mode);
void load_session_results(struct CarTime cars[], int *num_cars, const char *filename);
void combine_session_results( char *session_files[], int num_sessions, const char *output_file);
int file_exists(const char *filename);
void process_session_files(int session_num, char *type_session);
char *extract_type_session(char *filename);
void save_eliminated_cars(struct CarTime eligible_cars[], int num_cars_in_stage, int eliminated_cars_count, int session_num, struct CarTime cars[], int total_cars);
char **recuperer_colonne_csv(const char *nom_fichier, const char *nom_colonne, int numero_course, int *nb_resultats);
int test_recuperer_colonne_csv();
void create_directory_if_not_exists(const char *path);
void create_directories_from_csv_values(const char *csv_file, const char *course_column, const char *city_column);
void supprimer_dossiers_dans_repertoire(const char *path);

#endif // FILE_MANAGER_H
