#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include "car.h"
#include "display.h"
#include "file_manager.h"
#include "utils.h"
#include <sys/types.h>
#include <dirent.h>


char *extract_type_session(char *filename) {
    char *type_start = strrchr(filename, '/');  // Trouver le dernier '/' dans le chemin
    if (!type_start) type_start = filename;     // Si pas de '/', prendre le nom de fichier directement
    else type_start++;                          // Avancer d'un caractère pour passer le '/'

    char *underscore_pos = strchr(type_start, '_'); // Trouver le premier '_'
    if (!underscore_pos) {
        return NULL; 
    }          // Retourner NULL si le format est incorrect

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
            snprintf(session_files[i], 50, "fichiers/%s_%d.csv", type_session, i + 1);
            printf("%s\n", session_files[i]);
        }

        char output_file[50];
        sprintf(output_file, "fichiers/resume_%s.csv", type_session);

        combine_session_results(session_files, session_num, output_file);

        for (int i = 0; i < session_num; i++) {
            free(session_files[i]);  // Libérer la mémoire après utilisation
        }
    }
}


void save_eliminated_cars(struct CarTime eligible_cars[], int num_cars_in_stage, int eliminated_cars_count, int session_num, struct CarTime cars[], int total_cars) {
    FILE *ranking_file = fopen("fichiers/classement.csv", "a");
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



char **recuperer_colonne_csv(const char *nom_fichier, const char *nom_colonne, int numero_course, int *nb_resultats) {
    FILE *fichier = fopen(nom_fichier, "r");
    if (fichier == NULL) {
        printf("Erreur lors de l'ouverture du fichier %s.\n", nom_fichier);
        return NULL;
    }

    char ligne[1024];
    char *colonnes[50];
    int indice_colonne = -1;
    *nb_resultats = 0; // Initialiser le compteur de résultats

    // Lecture de la première ligne pour trouver l'index de la colonne
    if (fgets(ligne, sizeof(ligne), fichier) != NULL) {
        int i = 0;
        colonnes[i] = strtok(ligne, ",");
        while (colonnes[i] != NULL) {
            // Comparer en supprimant les espaces de début et de fin
            if (strcmp(trim(colonnes[i]), nom_colonne) == 0) {
                indice_colonne = i;
                // Supprimé: printf("Colonne \"%s\" trouvée à l'index %d\n", nom_colonne, indice_colonne);
            }
            colonnes[++i] = strtok(NULL, ",");
        }
    }


    // Si la colonne n'est toujours pas trouvée, afficher un message d'erreur détaillé
    if (indice_colonne == -1) {
        printf("Erreur: Colonne \"%s\" introuvable dans le fichier.\n", nom_colonne);
        fclose(fichier);
        return NULL;
    }


    // Allocation initiale pour le tableau de résultats
    char **resultats = malloc(100 * sizeof(char *));
    if (resultats == NULL) {
        printf("Erreur d'allocation mémoire.\n");
        fclose(fichier);
        return NULL;
    }

    // Lecture des lignes suivantes pour récupérer les valeurs de la colonne
    while (fgets(ligne, sizeof(ligne), fichier) != NULL) {
        int i = 0;
        char *valeur = NULL;
        colonnes[i] = strtok(ligne, ",");

        while (colonnes[i] != NULL) {
            if (i == indice_colonne) {
                valeur = trim(colonnes[i]);
            }
            colonnes[++i] = strtok(NULL, ",");
        }

        if (valeur != NULL) {
            resultats[*nb_resultats] = strdup(valeur);
            (*nb_resultats)++;
        }
    }



    fclose(fichier);
    return resultats;
}

// exemple d'utilisation de recuperer_colonne_csv
int test_recuperer_colonne_csv() {
    int nb_resultats;
    char **resultats = recuperer_colonne_csv("liste_circuits.csv", "Course", -1, &nb_resultats);

    if (resultats != NULL) {
        for (int i = 0; resultats[i] != NULL; i++) {
            printf("Résultat %d: %s\n", i + 1, resultats[i]);
            free(resultats[i]);
        }
        free(resultats);
    }
    return 0;
}

void create_directory_if_not_exists(const char *path) {
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        if (mkdir(path, 0700) != 0) {
            perror("Erreur lors de la création du dossier");
        } else {
            printf("Dossier %s créé avec succès.\n", path);
        }
    }
}

// Fonction pour créer un dossier pour chaque valeur de la colonne "Ville"
void create_directories_from_csv_values(const char *csv_file, const char *course_column, const char *city_column) {
    int nb_resultats_course, nb_resultats_city;
    
    // Récupérer les valeurs de la colonne "Course"
    char **course_values = recuperer_colonne_csv(csv_file, course_column, -1, &nb_resultats_course);
    // Récupérer les valeurs de la colonne "Ville"
    char **city_values = recuperer_colonne_csv(csv_file, city_column, -1, &nb_resultats_city);

    if (course_values != NULL && city_values != NULL && nb_resultats_course > 0 && nb_resultats_city > 0) {
        // Créer un dossier pour chaque ville en utilisant la course correspondante
        for (int i = 0; i < nb_resultats_course && i < nb_resultats_city; i++) {
            // Construire le chemin complet du dossier avec "fichiers/NuméroCourse_Ville"
            char full_path[256]; // Taille ajustable selon le besoin
            snprintf(full_path, sizeof(full_path), "fichiers/%s_%s", course_values[i], city_values[i]);

            // Créer le dossier avec le chemin complet
            create_directory_if_not_exists(full_path);
        }

        // Libérer la mémoire
        for (int i = 0; i < nb_resultats_course; i++) {
            free(course_values[i]);
        }
        for (int i = 0; i < nb_resultats_city; i++) {
            free(city_values[i]);
        }
        free(course_values);
        free(city_values);
    } else {
        printf("Aucune valeur trouvée dans les colonnes %s et %s.\n", course_column, city_column);
    }
}

// Fonction pour supprimer un dossier et tout son contenu
void supprimer_dossiers_dans_repertoire(const char *path) {
    struct dirent *entry;
    struct stat statbuf;
    char full_path[512]; // Taille ajustable selon le besoin

    DIR *dir = opendir(path);
    if (dir == NULL) {
        perror("Erreur lors de l'ouverture du répertoire");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        // Ignorer les entrées spéciales "." et ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        if (stat(full_path, &statbuf) == 0 && S_ISDIR(statbuf.st_mode)) {
            // Supprimer le contenu du dossier récursivement
            supprimer_dossiers_dans_repertoire(full_path);
            // Supprimer le dossier vide après suppression du contenu
            if (rmdir(full_path) == 0) {
                printf("Dossier %s supprimé avec succès.\n", full_path);
            } else {
                perror("Erreur lors de la suppression du dossier");
            }
        }
    }

    closedir(dir);
}
