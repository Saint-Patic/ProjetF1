#include <stdlib.h>
#include <time.h>

void initialize_random() {
    srand(time(NULL));
}

float random_float(int min, int max) {
    return min + (float)rand() / RAND_MAX * (max - min); 
}

