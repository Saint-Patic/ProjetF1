#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H
#define MAX_SESSION 3

#include "car.h"

void save_session_results(struct CarTime cars[], int num_cars, const char *filename);
void load_session_results(struct CarTime cars[], int *num_cars, const char *filename);
void combine_session_results( char *session_files[], int num_sessions, const char *output_file);
int file_exists(const char *filename);
void process_session_files(int session_num);

#endif // FILE_MANAGER_H
