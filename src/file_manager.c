#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include "../include/car.h"
#include "../include/display.h"
#include "../include/file_manager.h"
#include "../include/utils.h"


char *extract_type_session(char *filename) {
    char *type_start = strrchr(filename, '/');  // Trouver le dernier '/' dans le chemin
    if (!type_start) type_start = filename;     // Si pas de '/', prendre le nom de fichier directement
    else type_start++;                          // Avancer d'un caractère pour passer le '/'

    char *underscore_pos = strchr(type_start, '_'); // Trouver le premier '_'
    if (!underscore_pos) {
        return NULL; 
    }          // Retourner NULL si le format est incorrect

    size_t type_length = underscore_pos - type_start;
    char *type_session = malloc(type_length + 1);
    if (!type_session) return NULL;

    strncpy(type_session, type_start, type_length);
    type_session[type_length] = '\0';           // Terminer la chaîne

    return type_session;
}


int file_exists(const char *filename) {
    struct stat buffer;
    return (stat(filename, &buffer) == 0);
}


void save_session_results(car_t cars[], int num_cars, const char *filename, const char *mode) {
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
}

void process_session_files(int session_num, char *ville, char *type_session) {
    if ((strcmp(type_session, "essai") == 0 && session_num == MAX_SESSION_ESSAI) ||
        (strcmp(type_session, "qualif") == 0 && session_num == MAX_SESSION_QUALIF) ||
        (strcmp(type_session, "course") == 0 && session_num == MAX_SESSION_COURSE)) {
        char *session_files[session_num];

        // Allocation et création des chemins pour chaque fichier de session
        for (int i = 0; i < session_num; i++) {
            session_files[i] = malloc(100 * sizeof(char));  // Allocation mémoire pour chaque nom de fichier
            snprintf(session_files[i], 100, "data/fichiers/%s/%s_%d.csv", ville, type_session, i + 1);
        }
        

        char output_file[100];
        snprintf(output_file, 100, "data/fichiers/%s/resume_%s.csv", ville, type_session);

        // Appel à la fonction pour combiner les résultats des sessions
        combine_session_results(session_files, session_num, output_file);

        // Libération de la mémoire allouée
        for (int i = 0; i < session_num; i++) {
            free(session_files[i]);
        }
    }
}



void save_eliminated_cars(car_t eligible_cars[], int num_cars_in_stage, int eliminated_cars_count, int session_num, car_t cars[], int total_cars, const char *ville) {
    char *ranking_file_path = malloc(100 * sizeof(char));
    snprintf(ranking_file_path, 100, "data/fichiers/%s/classement.csv", ville);

    FILE *ranking_file = fopen(ranking_file_path, "a");
    if (ranking_file == NULL) {
        printf("Erreur lors de l'ouverture de %s\n", ranking_file_path);
        return;
    }

    // Si le fichier est vide, écrire l'en-tête
    if (ftell(ranking_file) == 0) {
        fprintf(ranking_file, "Car Number,Session Number,Position,Best Lap Time\n");
    }

    // Si c'est la troisième session de qualifications (Q3), enregistrer les positions finales
    if (session_num == 3) {
        for (int i = 0; i < num_cars_in_stage; i++) {
            fprintf(ranking_file, "%d,%d,%d,%.2f\n", 
                    eligible_cars[i].car_number, session_num, i + 1, eligible_cars[i].best_lap_time);
        }
    } else {
        // Pour Q1 et Q2, enregistrer les voitures éliminées
        for (int i = num_cars_in_stage - eliminated_cars_count; i < num_cars_in_stage; i++) {
            fprintf(ranking_file, "%d,%d,%d,%.2f\n", 
                    eligible_cars[i].car_number, session_num, 
                    num_cars_in_stage - eliminated_cars_count + (i - (num_cars_in_stage - eliminated_cars_count) + 1), 
                    eligible_cars[i].best_lap_time);

            // Marquer les voitures comme éliminées dans le tableau original
            for (int j = 0; j < total_cars; j++) {
                if (cars[j].car_number == eligible_cars[i].car_number) {
                    cars[j].out = 1; // Marquer la voiture comme éliminée
                    break;
                }
            }
        }
    }
    fclose(ranking_file);
    free(ranking_file_path);
}


void load_eliminated_cars(char *filename, car_t cars[], int total_cars) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Erreur lors de l'ouverture de %s\n", filename);
        return;
    }
    
    int car_number, session_num, place;
    float lap_time;
    char *line = malloc(100 * sizeof(char));
    
    // Mark all cars as eligible initially
    for (int i = 0; i < total_cars; i++) {
        cars[i].out = 0;
    }
    
    // Read each line to mark cars that are already eliminated
    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%d,%d,%d,%f", &car_number, &session_num, &place, &lap_time);
        for (int i = 0; i < total_cars; i++) {
            if (cars[i].car_number == car_number) { 
                cars[i].out = 1;
                break;
            }
        }
    }
    fclose(file);
    free(line);
}

// Fonction pour récupérer les valeurs d'une colonne spécifique dans un fichier CSV
char **recuperer_colonne_csv(const char *nom_fichier, const char *nom_colonne, int *nb_resultats) {
    FILE *fichier = fopen(nom_fichier, "r");
    if (!fichier) {
        printf("Erreur : Impossible d'ouvrir le fichier %s.\n", nom_fichier);
        return NULL;
    }

    char ligne[MAX_COLONNE];
    char **resultats = NULL;
    int indice_colonne = -1;
    *nb_resultats = 0;

    // Lecture de la première ligne (en-tête)
    if (fgets(ligne, sizeof(ligne), fichier)) {
        char *colonne = strtok(ligne, ",");
        int index = 0;

        while (colonne) {
            if (strcmp(trim(colonne), nom_colonne) == 0) {
                indice_colonne = index;
                break;
            }
            colonne = strtok(NULL, ",");
            index++;
        }
    }

    // Si la colonne est introuvable
    if (indice_colonne == -1) {
        printf("Erreur : Colonne \"%s\" introuvable dans le fichier.\n", nom_colonne);
        fclose(fichier);
        return NULL;
    }

    // Lecture des lignes suivantes pour récupérer les valeurs
    resultats = malloc(100 * sizeof(char *));
    if (!resultats) {
        printf("Erreur : Échec de l'allocation mémoire.\n");
        fclose(fichier);
        return NULL;
    }

    while (fgets(ligne, sizeof(ligne), fichier)) {
        char *valeur = NULL;
        char *colonne = strtok(ligne, ",");
        int index = 0;

        while (colonne) {
            if (index == indice_colonne) {
                valeur = trim(colonne);
                break;
            }
            colonne = strtok(NULL, ",");
            index++;
        }

        if (valeur) {
            resultats[*nb_resultats] = strdup(valeur);
            if (!resultats[*nb_resultats]) {
                printf("Erreur : Échec de l'allocation mémoire pour la valeur.\n");
                // Nettoyer les valeurs précédemment allouées
                for (int i = 0; i < *nb_resultats; i++) {
                    free(resultats[i]);
                }
                free(resultats);
                fclose(fichier);
                return NULL;
            }
            (*nb_resultats)++;
        }
    }

    fclose(fichier);
    return resultats;
}

// exemple d'utilisation de recuperer_colonne_csv
int test_recuperer_colonne_csv() {
    int nb_resultats;
    char **resultats = recuperer_colonne_csv("../data/liste_circuits.csv", "Ville", &nb_resultats);

    if (resultats != NULL) {
        for (int i = 0; resultats[i] != NULL; i++) {
            printf("Résultat %d: %s\n", i + 1, resultats[i]);
            free(resultats[i]);
        }
        free(resultats);
    }
    return 0;
}

// Fonction pour créer un dossier si inexistant
void create_directory_if_not_exists(const char *path) {
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        if (mkdir(path, 0700) != 0) {
            perror("Erreur lors de la création du dossier");
        } 
    }
}

// Fonction pour créer des dossiers à partir des valeurs des colonnes CSV
void create_directories_from_csv_values(const char *csv_file, const char *course_column, const char *city_column) {
    int nb_courses = 0, nb_cities = 0;

    // Récupérer les valeurs des colonnes "Course" et "Ville"
    char **course_values = recuperer_colonne_csv(csv_file, course_column, &nb_courses);
    char **city_values = recuperer_colonne_csv(csv_file, city_column, &nb_cities);

    if (!course_values || !city_values || nb_courses == 0 || nb_cities == 0) {
        printf("Erreur : Impossible de récupérer les colonnes %s et %s.\n", course_column, city_column);
        // Libérer la mémoire si partiellement allouée
        if (course_values) {
            for (int i = 0; i < nb_courses; i++) free(course_values[i]);
            free(course_values);
        }
        if (city_values) {
            for (int i = 0; i < nb_cities; i++) free(city_values[i]);
            free(city_values);
        }
        return;
    }

    // Créer les dossiers pour chaque paire Course-Ville
    for (int i = 0; i < nb_courses && i < nb_cities; i++) {
        char path[MAX_PATH_LENGTH];
        snprintf(path, sizeof(path), "data/fichiers/%s_%s", course_values[i], city_values[i]);
        create_directory_if_not_exists(path);
    }

    // Libérer la mémoire
    for (int i = 0; i < nb_courses; i++) free(course_values[i]);
    for (int i = 0; i < nb_cities; i++) free(city_values[i]);
    free(course_values);
    free(city_values);
}

void generate_special_filename(const char *ville, const char *session_type, int session_num, int special_weekend, char *output_filename) {
    if (special_weekend) {
        if (strcmp(session_type, "essai") == 0) {
            // Les essais libres restent inchangés
            snprintf(output_filename, 100, "data/fichiers/%s/%s_%d.csv", ville, session_type, session_num);
        } else if (strcmp(session_type, "qualif") == 0) {
            if (session_num >= 1 && session_num <= 3) {
                // Qualif_1, 2, 3 deviennent sprint_shootout_1, 2, 3
                snprintf(output_filename, 100, "data/fichiers/%s/sprint_shootout_%d.csv", ville, session_num);
            } else if (session_num >= 4 && session_num <= 6) {
                // Qualif_4, 5, 6 deviennent qualif_1, 2, 3
                snprintf(output_filename, 100, "data/fichiers/%s/qualif_%d.csv", ville, session_num - 3);
            }
        } else if (strcmp(session_type, "course") == 0) {
            if (session_num == 1) {
                // Course_1 devient sprint_1
                snprintf(output_filename, 100, "data/fichiers/%s/sprint_1.csv", ville);
            } else if (session_num == 2) {
                // Course_2 devient course_1
                snprintf(output_filename, 100, "data/fichiers/%s/course_1.csv", ville);
            }
        } else {
            // Nom standard pour les autres cas
            snprintf(output_filename, 100, "data/fichiers/%s/%s_%d.csv", ville, session_type, session_num);
        }
    } else {
        // Pas de changement pour un week-end normal
        snprintf(output_filename, 100, "data/fichiers/%s/%s_%d.csv", ville, session_type, session_num);
    }
}

// Function to read car numbers from classement.csv
void read_starting_grid(const char *filename, int car_numbers[], int num_cars) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open classement.csv");
        exit(EXIT_FAILURE);
    }

    // Skip the header line
    char buffer[256];
    fgets(buffer, sizeof(buffer), file);

    int car_number, session_num, position;
    float best_lap_time;

    while (fscanf(file, "%d,%d,%d,%f\n", &car_number, &session_num, &position, &best_lap_time) == 4) {
        if (position > 0 && position <= num_cars) {
            car_numbers[position - 1] = car_number;
        }
    }

    fclose(file);
}
