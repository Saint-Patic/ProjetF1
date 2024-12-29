#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/sem.h>
#include <sys/shm.h> // Include this header for shared memory functions
#include "../include/car.h"
#include "../include/utils.h"
#include "../include/display.h"
#include "../include/file_manager.h"
#include "../include/simulate.h"
#include "../include/semaphore.h"

//sem_t *sem; // Define the semaphore as a global variable

// Variables globales pour l'algorithme de Courtois
int flag[MAX_NUM_CARS] = {0}; // Indique si une voiture veut entrer dans la section critique
int turn = 0;                 // Tour actuel pour le processus


// Fonction pour initialiser le sémaphore global
void init_semaphore() {
    for (int i = 0; i < MAX_NUM_CARS; i++) {
        flag[i] = 0; // Aucune voiture n'est initialement en attente
    }
    turn = 0; // Le premier processus à passer sera le processus 0
}

// Fonction pour détruire le sémaphore à la fin du programme
void destroy_semaphore() {
    
}

void enter_critical_section(int i) {
    flag[i] = 1; // Indique l'intention d'entrer
    int j = turn;

    // Boucle pour attendre son tour
    while (j != i) {
        if (flag[j] == 0) { // Si le processus courant n'attend pas, passer au suivant
            j = (j + 1) % MAX_NUM_CARS;
        }
    }

    flag[i] = 2; // Indique que la section critique est acquise

    // Vérifie si un autre processus a déjà la section critique
    for (j = 0; j < MAX_NUM_CARS; j++) {
        if (j != i && flag[j] == 2) {
            // Si un autre processus est en section critique, attendre
            while (flag[j] == 2) {
                // Attendre activement que l'autre processus sorte
            }
        }
    }
}

void exit_critical_section(int i) {
    turn = (turn + 1) % MAX_NUM_CARS; // Passe au processus suivant
    flag[i] = 0; // Libère la section critique
}