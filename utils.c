#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "car.h"
#include "utils.h"
#include "display.h"
#include "file_manager.h"

void initialize_random() {
    srand(time(NULL));
}

float random_float(int min, int max) {
    return min + (float)rand() / RAND_MAX * (max - min); 
}


int estimate_max_laps(int session_duration, int max_time) {
    return session_duration / max_time;
}


char *trim(char *str) {
    char *end;

    // Supprimer les espaces en début
    while (isspace((unsigned char)*str)) str++;

    // Si la chaîne est vide
    if (*str == 0) return str;

    // Supprimer les espaces en fin
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;

    // Ajouter le terminateur de chaîne
    *(end + 1) = '\0';

    return str;
}