#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <pthread.h>
#include <fcntl.h>
#include <float.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h> // Include this header for shared memory functions
#include "../include/car.h"
#include "../include/utils.h"
#include "../include/display.h"
#include "../include/file_manager.h"
#include "../include/simulate.h"
#include "../include/algorithm.h"

sem_t *mutex; // Define the semaphore as a global variable
sem_t *mutlect;

// Variables globales pour l'algorithme de Courtois
int nblect = 0;

// Fonction pour initialiser le sémaphore global
void init_semaphore() {
    printf("Hello World !\n");
    mutex = sem_open(SEM_NAME_MUTEX, O_CREAT, 0644, 1); // Valeur initiale du sémaphore : 1 (sémaphore binaire)
    if (mutex == SEM_FAILED) {
        perror("Erreur lors de l'initialisation du sémaphore");
        exit(EXIT_FAILURE);
    }
    mutlect = sem_open(SEM_NAME_MUTLECT, O_CREAT, 0644, 1); // Valeur initiale du sémaphore : 1 (sémaphore binaire)
    if (mutlect == SEM_FAILED) {
        perror("Erreur lors de l'initialisation du sémaphore");
        exit(EXIT_FAILURE);
    }
}

// Fonction pour détruire le sémaphore à la fin du programme
void destroy_semaphore() {
    sem_close(mutex);
    sem_close(mutlect);
    sem_unlink(SEM_NAME_MUTEX);
    sem_unlink(SEM_NAME_MUTLECT);
}

// Fonction pour entrer en section critique (lecteur)
void enter_critical_section_reader() {
    sem_wait(mutlect);
    nblect++;
    if (nblect == 1) {
        sem_wait(mutex);
    }
    sem_post(mutlect);
}

// Fonction pour sortir de la section critique (lecteur)
void exit_critical_section_reader() {
    sem_wait(mutlect);
    nblect--;
    if (nblect == 0) {
        sem_post(mutex);
    }
    sem_post(mutlect);
}

// Fonction pour entrer en section critique (rédacteur)
void enter_critical_section_writer() {
    sem_wait(mutex);
}

// Fonction pour sortir de la section critique (rédacteur)
void exit_critical_section_writer() {
    sem_post(mutex);
}