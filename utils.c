#include <stdlib.h>
#include <time.h>

void initialize_random() {
    srand(time(NULL));
}

float random_float(int min, int max) {
    return (rand() % (max - min + 1)) + min + (float)rand() / RAND_MAX;
}
