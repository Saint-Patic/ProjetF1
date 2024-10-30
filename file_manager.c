#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include "car.h"
#include "display.h"
#include "file_manager.h"


char *extract_type_session(char *filename) {
    char *type_start = strrchr(filename, '/');  // Trouver le dernier '/' dans le chemin
    if (!type_start) type_start = filename;     // Si pas de '/', prendre le nom de fichier directement
    else type_start++;                          // Avancer d'un caractère pour passer le '/'

    char *underscore_pos = strchr(type_start, '_'); // Trouver le premier '_'
    if (!underscore_pos) return NULL;           // Retourner NULL si le format est incorrect

    size_t type_length = underscore_pos - type_start;
    char *session_type = malloc(type_length + 1);
    if (!session_type) return NULL;

    strncpy(session_type, type_start, type_length);
    session_type[type_length] = '\0';           // Terminer la chaîne

    return session_type;
}


int file_exists(const char *filename) {
    struct stat buffer;
    return (stat(filename, &buffer) == 0);
}


void save_session_results(struct CarTime cars[], int num_cars, const char *filename, const char *mode) {
    FILE *file = fopen(filename, mode);
    if (!file) {
        perror("Erreur d'ouverture du fichier");
        return;
    }

    fprintf(file, "Car Number,Best Lap Time,Sector 1 Time,Sector 2 Time,Sector 3 Time\n");

    float best_overall_sector_times[NUM_SECTORS] = {0};
    float best_overall_lap_time = 0;

    for (int i = 0; i < num_cars; i++) {
        fprintf(file, "%d,%.2f,%.2f,%.2f,%.2f\n",
                cars[i].car_number,
                cars[i].best_lap_time,
                cars[i].best_sector_times[0],
                cars[i].best_sector_times[1],
                cars[i].best_sector_times[2]);

        for (int j = 0; j < NUM_SECTORS; j++) {
            if (best_overall_sector_times[j] == 0 || cars[i].best_sector_times[j] < best_overall_sector_times[j]) {
                best_overall_sector_times[j] = cars[i].best_sector_times[j];
            }
        }

        if (best_overall_lap_time == 0 || cars[i].best_lap_time < best_overall_lap_time) {
            best_overall_lap_time = cars[i].best_lap_time;
        }
    }

    fprintf(file, "\nBest Sector Times\n");
    fprintf(file, "Sector 1,%.2f\n", best_overall_sector_times[0]);
    fprintf(file, "Sector 2,%.2f\n", best_overall_sector_times[1]);
    fprintf(file, "Sector 3,%.2f\n", best_overall_sector_times[2]);
    fprintf(file, "Best Overall Lap,%.2f\n", best_overall_lap_time);

    fclose(file);
}

void load_session_results(struct CarTime cars[], int *num_cars, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Erreur d'ouverture du fichier");
        return;
    }

    int i = 0;
    while (fscanf(file, "Car #%d: Best Lap: %f s, S1: %f s, S2: %f s, S3: %f s\n",
                  &cars[i].car_number,
                  &cars[i].best_lap_time,
                  &cars[i].best_sector_times[0],
                  &cars[i].best_sector_times[1],
                  &cars[i].best_sector_times[2]) == 5) {
        i++;
    }
    *num_cars = i;

    fclose(file);
    printf("Les résultats de la session ont été chargés depuis %s\n", filename);
}

void combine_session_results(char *session_files[], int num_sessions, const char *output_file) {
    float best_sector_times[NUM_SECTORS] = {9999.0, 9999.0, 9999.0};
    float best_overall_lap_time = 9999.0;

    // Parcourir chaque fichier de session pour trouver les meilleurs temps
    for (int session = 0; session < num_sessions; session++) {
        FILE *file = fopen(session_files[session], "r");
        if (!file) {
            perror("Erreur d'ouverture du fichier");
            continue;
        }

        // Ignorer la première ligne (en-tête)
        char buffer[128];
        fgets(buffer, sizeof(buffer), file);

        // Lire les temps pour chaque voiture et mettre à jour les meilleurs temps
        int car_number;
        float lap_time, sector1, sector2, sector3;
        while (fscanf(file, "%d,%f,%f,%f,%f\n", &car_number, &lap_time, &sector1, &sector2, &sector3) == 5) {
            // Meilleur temps de chaque secteur parmi les voitures de toutes les sessions
            if (sector1 < best_sector_times[0]) best_sector_times[0] = sector1;
            if (sector2 < best_sector_times[1]) best_sector_times[1] = sector2;
            if (sector3 < best_sector_times[2]) best_sector_times[2] = sector3;

            // Meilleur tour global
            if (lap_time < best_overall_lap_time) best_overall_lap_time = lap_time;
        }

        fclose(file);
    }

    // Écrire les résultats dans le fichier de sortie
    FILE *output = fopen(output_file, "w");
    if (!output) {
        perror("Erreur d'ouverture du fichier de sortie");
        return;
    }

    fprintf(output, "Meilleur temps par secteur et tour global sur toutes les périodes d'essai\n");
    fprintf(output, "Secteur 1,%.2f\n", best_sector_times[0]);
    fprintf(output, "Secteur 2,%.2f\n", best_sector_times[1]);
    fprintf(output, "Secteur 3,%.2f\n", best_sector_times[2]);
    fprintf(output, "Meilleur temps de tour global,%.2f\n", best_overall_lap_time);

    fclose(output);
    printf("Les meilleurs temps ont été enregistrés dans %s\n", output_file);
}


void process_session_files(int session_num, char *type_session) {
    if (session_num == MAX_SESSION) {
        char *session_files[session_num];
        
        for (int i = 0; i < session_num; i++) {
            session_files[i] = malloc(50 * sizeof(char));  // Allocation mémoire pour chaque nom de fichier
            snprintf(session_files[i], 50, "fichier_enregistree/%s_%d.csv", type_session, i + 1);
            printf("%s\n", session_files[i]);
        }

        char output_file[50];
        sprintf(output_file, "fichier_enregistree/resume_%s.csv", type_session);

        combine_session_results(session_files, session_num, output_file);

        for (int i = 0; i < session_num; i++) {
            free(session_files[i]);  // Libérer la mémoire après utilisation
        }
    }
}


void save_eliminated_cars(struct CarTime eligible_cars[], int num_cars_in_stage, int eliminated_cars_count, int session_num, struct CarTime cars[], int total_cars) {
    FILE *ranking_file = fopen("fichier_enregistree/classement.csv", "a");
    if (ranking_file == NULL) {
        printf("Erreur lors de l'ouverture de classement.csv\n");
        return;
    }

    // If the file is empty, write the header
    if (ftell(ranking_file) == 0) {
        fprintf(ranking_file, "Car Number,Session Number,Position,Best Lap Time\n");
    }

    // If it's the third qualifying session, we do not eliminate cars but record their positions
    if (session_num == 3) {
        for (int i = 0; i < num_cars_in_stage; i++) {
            fprintf(ranking_file, "%d,%d,%d,%.2f\n", eligible_cars[i].car_number, session_num, i + 1, eligible_cars[i].best_lap_time);
        }
    } else {
        // For Q1 and Q2, save eliminated cars
        for (int i = num_cars_in_stage - eliminated_cars_count; i < num_cars_in_stage; i++) {
            fprintf(ranking_file, "%d,%d,%d,%.2f\n", eligible_cars[i].car_number, session_num, num_cars_in_stage - eliminated_cars_count + (i - (num_cars_in_stage - eliminated_cars_count) + 1), eligible_cars[i].best_lap_time);
            
            // Mark these cars as eliminated in the original cars array
            for (int j = 0; j < total_cars; j++) {
                if (cars[j].car_number == eligible_cars[i].car_number) {
                    cars[j].out = 1; // Mark car as eliminated
                    break;
                }
            }
        }
    }

    fclose(ranking_file);
}
