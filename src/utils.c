#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <glob.h>
#include "../include/car.h"
#include "../include/utils.h"
#include "../include/display.h"
#include "../include/file_manager.h"

// Fonction pour effectuer toutes les vérifications
//session_file: chemin du fichier
//ville: le nom de la ville seul
//session_type: type de la course qualif, etc
//seesion_num: essai 1, essai 2, etc
//directory_num: numéro avant le nom de la ville
int verifier_parametres(char *session_file, char *ville, char *session_type, int *session_num, int *directory_num) {
    if (ville == NULL) {
        printf("Erreur d'allocation mémoire\n");
        return 1;
    }

    // Vérifie le format du fichier de session
    if (sscanf(session_file, "data/fichiers/%49[^/]/%19[a-zA-Z]_%d.csv", ville, session_type, session_num) != 3) {
        printf("Nom de fichier invalide. Utilisez le format ../data/fichiers/<x_ville>/<type>_<numéro>.csv\n");
        return 0;
    }

    int special_weekend = is_special_weekend(ville);

    // Vérifications pour les week-ends spéciaux
    if (special_weekend) {
        if (strcmp(session_type, "essai") == 0) {
            if (*session_num != 1) {
                printf("Erreur : Pour un week-end spécial, il n'y a qu'une seule période d'essai.\n");
                return 0;
            }
        } else if (strcmp(session_type, "shootout") == 0) {
            if (*session_num < 1 || *session_num > 3) {
                printf("Erreur : Nombre maximum de Sprint Shootout dépassé (3).\n");
                return 0;
            }

            // Vérifie que l'essai a été effectué avant le Sprint Shootout
            char essai_resume_file[100];
            snprintf(essai_resume_file, sizeof(essai_resume_file), "data/fichiers/%s/essai_1.csv", ville);
            if (!file_exists(essai_resume_file)) {
                printf("Erreur : Le Sprint Shootout nécessite une période d'essai préalable (%s).\n", essai_resume_file);
                return 0;
            }
        } else if (strcmp(session_type, "sprint") == 0) {
            if (*session_num != 1) {
                printf("Erreur : Nombre maximum de courses sprint dépassé (1).\n");
                return 0;
            }

            // Vérifie que le Sprint Shootout a été complété avant le Sprint
            char shootout_resume_file[100];
            snprintf(shootout_resume_file, sizeof(shootout_resume_file), "data/fichiers/%s/shootout_3.csv", ville);
            if (!file_exists(shootout_resume_file)) {
                printf("Erreur : La course sprint nécessite un Sprint Shootout complet (%s).\n", shootout_resume_file);
                return 0;
            }
        } else if (strcmp(session_type, "course") == 0) {
            if (*session_num != 1) {
                printf("Erreur : Nombre maximum de courses principales dépassé (1).\n");
                return 0;
            }

            // Vérifie que les qualifications normales ont été complétées avant la course principale
            char qualif_resume_file[100];
            snprintf(qualif_resume_file, sizeof(qualif_resume_file), "data/fichiers/%s/qualif_3.csv", ville);
            if (!file_exists(qualif_resume_file)) {
                printf("Erreur : La course principale nécessite des qualifications normales (%s).\n", qualif_resume_file);
                return 0;
            }
        }
    } else {
        // Vérifications pour les week-ends normaux
        if (strcmp(session_type, "essai") == 0 && *session_num > MAX_SESSION_ESSAI) {
            printf("Erreur : Nombre maximum d'essais dépassé (%d).\n", MAX_SESSION_ESSAI);
            return 0;
        } else if (strcmp(session_type, "qualif") == 0) {
            if (*session_num < 1 || *session_num > MAX_SESSION_QUALIF) {
                printf("Erreur : Nombre maximum de qualifications dépassé (3).\n");
                return 0;
            }

            // Vérifie que les essais ont été complétés avant les qualifications
            char essai_resume_file[100];
            snprintf(essai_resume_file, sizeof(essai_resume_file), "data/fichiers/%s/essai_1.csv", ville);
            if (!file_exists(essai_resume_file)) {
                printf("Erreur : Les qualifications nécessitent une période d'essai préalable (%s).\n", essai_resume_file);
                return 0;
            }
        } else if (strcmp(session_type, "course") == 0) {
            if (*session_num != 1) {
                printf("Erreur : Nombre maximum de courses principales dépassé (1).\n");
                return 0;
            }

            // Vérifie que les qualifications ont été complétées avant la course
            char qualif_resume_file[100];
            snprintf(qualif_resume_file, sizeof(qualif_resume_file), "data/fichiers/%s/qualif_3.csv", ville);
            if (!file_exists(qualif_resume_file)) {
                printf("Erreur : La course principale nécessite des qualifications (%s).\n", qualif_resume_file);
                return 0;
            }
        }
    }

    // Vérifie que le numéro de session est valide
    if (*session_num < 1) {
        printf("Erreur : Numéro de session invalide.\n");
        return 0;
    }

    // Vérifie que le fichier de la session précédente existe
    char prev_session_file[100];
    snprintf(prev_session_file, sizeof(prev_session_file), "data/fichiers/%s/%s_%d.csv", ville, session_type, *session_num - 1);
    if (*session_num > 1 && !file_exists(prev_session_file)) {
        printf("Erreur : La session précédente %s n'existe pas.\n", prev_session_file);
        return 0;
    }

    // Vérifie que le fichier de la session actuelle n'existe pas déjà
    if (file_exists(session_file)) {
        printf("Erreur : Le fichier %s existe déjà. Session déjà exécutée.\n", session_file);
        return 0;
    }
    // cette vérification sert ne pas commencer un nouveau grand prix si le précédent n'a pas eu lieu
    glob_t result;
    char path_to_city[256];
    int temp_num = *directory_num;  // deréférence le pointeur parce que qu'il ne peut pas être utilisé tel quelle dans la suite

    // crée la string avec le chemin connu + le num de la couse précédente
    snprintf(path_to_city, sizeof(path_to_city), "data/fichiers/%d", temp_num - 1);

    // remplit le structure glob avec les résultats de la recherche 
    int verif = glob(path_to_city, GLOB_NOSORT, NULL, &result);

    // vérifie si le fichier résume_course.csv de la course précédente existe
    if (verif == 0 && temp_num > 0) {
        printf("ERREUR: Aucun fichier trouvé '%s':\n", path_to_city);
        // check si un dossier à été trouvé et pas plusieurs parce qu'il ne doit pas en trouvé plus
        if (result.gl_pathc == 1) {
            if (file_exists(result.gl_pathv[0])) {
                snprintf(path_to_city, sizeof(path_to_city), "%s/resume_course.csv", result.gl_pathv[0]);
                if (!file_exists(path_to_city)) {
                    printf("ERREUR: Le fichier resume_course n'a pas été trouvé: %s\n", path_to_city);
                    return 0;
                }
            }
        }
    }
    globfree(&result);
    return 1;
}



void initialize_random() {
    srand(time(NULL));
}

float random_float(int min, int max) {
    return min + (float)rand() / RAND_MAX * (max - min); 
}

int estimate_max_laps(int session_duration, float max_time) {
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

int ternaire_moins_criminel(int session_num, int res1, int res2, int resDefault, int sprint_mode) {
    if (sprint_mode) {
        switch (session_num) {
            case 1:
                return 12 * 60; // Q1
            case 2:
                return 10 * 60; // Q2
            default:
                return 8 * 60; // Q3
        }
    } else {
            switch (session_num) {
                case 1:
                    return res1; // Q1
                case 2:
                    return res2; // Q2
                default:
                    return resDefault; // Q3
        }   
    }
} // cordialement, Eloy

int is_special_weekend(const char *ville) {
    int nb_resultats;
    char **weekend_types = recuperer_colonne_csv("data/liste_circuits.csv", "Sprint", &nb_resultats);
    int ville_num = atoi(ville);

    if (ville_num > 0 && ville_num <= nb_resultats) {
        if (atoi(weekend_types[ville_num - 1]) == 1) return 1;
    }
    return 0;
}

void initialize_cars(car_t cars[], int car_numbers[], int num_cars) {
    for (int i = 0; i < num_cars; i++) {
        cars[i].car_number = car_numbers[i];
        cars[i].best_lap_time = 0;
        cars[i].temps_rouler = 0;
        cars[i].pit_stop = 0;
        cars[i].pit_stop_nb = 1;
        cars[i].out = 0;
        for (int j = 0; j < NUM_SECTORS; j++) {
            cars[i].sector_times[j] = 0;
            cars[i].best_sector_times[j] = 0;
        }
    }
}

int calculate_total_laps(const char *ville, float session_distance) {
    int nb_resultats;
    char **circuit_distance = recuperer_colonne_csv("data/liste_circuits.csv", "taille (km)", &nb_resultats);
    int ville_index = atoi(ville) - 1;
    float circuit_length = (ville_index >= 0 && ville_index < nb_resultats) ? atof(circuit_distance[ville_index]) : 0.0;

    // Libération des données allouées par recuperer_colonne_csv
    for (int i = 0; i < nb_resultats; i++) {
        free(circuit_distance[i]);
    }
    free(circuit_distance);

    return circuit_length > 0 ? estimate_max_laps(session_distance, circuit_length) + 1 : 0;
}