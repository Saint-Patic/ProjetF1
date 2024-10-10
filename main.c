#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <time.h>

#define MAX_LINE_LENGTH 1024
#define MAX_COLS 100



float generate_random_time(float min_time, float max_time) {
  return min_time + ((float)rand() / RAND_MAX) * (max_time - min_time);
}

void get_csv_headers(const char *filename, char headers[][MAX_LINE_LENGTH], int *col_count) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Could not open file");
        exit(EXIT_FAILURE);
    }

    char line[MAX_LINE_LENGTH];
    if (fgets(line, MAX_LINE_LENGTH, file)) {
        char *token = strtok(line, ",");
        *col_count = 0;
        while (token) {
            strcpy(headers[*col_count], token);
            token = strtok(NULL, ",");
            (*col_count)++;
        }
    }

    fclose(file);
}
void exemple_get_csv_headers {
    char headers[MAX_COLS][MAX_LINE_LENGTH];
    int col_count = 0;

    get_csv_headers("data.csv", headers, &col_count);

    for (int i = 0; i < col_count; i++) {
        printf("Header %d: %s\n", i + 1, headers[i]);
    }
}



