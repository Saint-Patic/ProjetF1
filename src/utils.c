#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include "../include/car.h"
#include "../include/utils.h"
#include "../include/display.h"
#include "../include/file_manager.h"
#include "../include/simulate.h"


int check_file_exists(const char *file_path, const char *error_message) {
    if (!file_exists(file_path)) {
        printf("Erreur : %s (%s).\n", error_message, file_path);
        return 0;
    }
    return 1;
}

int check_session_num(int session_num, int max_num, const char *error_message) {
    if (session_num < 1 || session_num > max_num) {
        printf("Erreur : %s\n", error_message);
        return 0;
    }
    return 1;
}

/**
 * @brief Vérifie la validité des paramètres pour une session de course.
 * 
 * @param session_file Chemin du fichier décrivant la session.
 * @param ville Nom de la ville où se déroule la session.
 * @param session_type Type de la session ("essai", "qualif", "course", etc.).
 * @param session_num Numéro de la session (1 pour Essai 1, 2 pour Qualif 2, etc.).
 * @param directory_num Numéro de la course associé au répertoire de fichiers.
 * @return 1 si les paramètres sont valides, 0 sinon.
 */
int verifier_parametres(char *session_file, char *ville, char *session_type, int *session_num, int *directory_num) {
    if (ville == NULL) {
        printf("Erreur d'allocation mémoire\n");
        return 1;
    }

    if (sscanf(session_file, "data/fichiers/%49[^/]/%19[a-zA-Z]_%d.csv", ville, session_type, session_num) != 3) {
        printf("Nom de fichier invalide. Utilisez le format ../data/fichiers/<x_ville>/<type>_<numéro>.csv\n");
        return 0;
    }

    int special_weekend = is_special_weekend(ville);

    if (special_weekend) {
        if (strcmp(session_type, "essai") == 0) {
            return check_session_num(*session_num, 1, "Pour un week-end spécial, il n'y a qu'une seule période d'essai.");
        } else if (strcmp(session_type, "shootout") == 0) {
            if (!check_session_num(*session_num, 3, "Nombre maximum de Sprint Shootout dépassé (3).")) return 0;
            char essai_resume_file[100];
            snprintf(essai_resume_file, sizeof(essai_resume_file), "data/fichiers/%s/essai_1.csv", ville);
            return check_file_exists(essai_resume_file, "Le Sprint Shootout nécessite une période d'essai préalable");
        } else if (strcmp(session_type, "sprint") == 0) {
            if (!check_session_num(*session_num, 1, "Nombre maximum de courses sprint dépassé (1).")) return 0;
            char shootout_resume_file[100];
            snprintf(shootout_resume_file, sizeof(shootout_resume_file), "data/fichiers/%s/resume_shootout.csv", ville);
            return check_file_exists(shootout_resume_file, "La course sprint nécessite un Sprint Shootout complet");
        } else if (strcmp(session_type, "qualif") == 0) {
            if (!check_session_num(*session_num, MAX_SESSION_QUALIF, "Nombre maximum de qualifications dépassé (3).")) return 0;
            char essai_resume_file[100];
            snprintf(essai_resume_file, sizeof(essai_resume_file), "data/fichiers/%s/sprint_1.csv", ville);
            return check_file_exists(essai_resume_file, "Les qualifications nécessitent une période de sprint préalable");
        } else if (strcmp(session_type, "course") == 0) {
            if (!check_session_num(*session_num, 1, "Nombre maximum de courses principales dépassé (1).")) return 0;
            char qualif_resume_file[100];
            snprintf(qualif_resume_file, sizeof(qualif_resume_file), "data/fichiers/%s/resume_qualif.csv", ville);
            return check_file_exists(qualif_resume_file, "La course principale nécessite des qualifications normales");
        }
    } else {
        if (strcmp(session_type, "essai") == 0) {
            return check_session_num(*session_num, MAX_SESSION_ESSAI, "Nombre maximum d'essais dépassé");
        } else if (strcmp(session_type, "shootout") == 0 || strcmp(session_type, "sprint") == 0) {
            printf("Erreur : Il n'y a pas de %s pour un week-end normal\n", session_type);
            return 0;
        } else if (strcmp(session_type, "qualif") == 0) {
            if (!check_session_num(*session_num, MAX_SESSION_QUALIF, "Nombre maximum de qualifications dépassé (3).")) return 0;
            char essai_resume_file[100];
            snprintf(essai_resume_file, sizeof(essai_resume_file), "data/fichiers/%s/resume_essai.csv", ville);
            return check_file_exists(essai_resume_file, "Les qualifications nécessitent une période d'essai préalable");
        } else if (strcmp(session_type, "course") == 0) {
            if (!check_session_num(*session_num, 1, "Nombre maximum de courses principales dépassé (1).")) return 0;
            char qualif_resume_file[100];
            snprintf(qualif_resume_file, sizeof(qualif_resume_file), "data/fichiers/%s/resume_qualif.csv", ville);
            return check_file_exists(qualif_resume_file, "La course principale nécessite des qualifications");
        }
    }

    if (*session_num < 1) {
        printf("Erreur : Numéro de session invalide.\n");
        return 0;
    }

    char prev_session_file[100];
    snprintf(prev_session_file, sizeof(prev_session_file), "data/fichiers/%s/%s_%d.csv", ville, session_type, *session_num - 1);
    if (*session_num > 1 && !file_exists(prev_session_file)) {
        printf("Erreur : La session précédente %s n'existe pas.\n", prev_session_file);
        return 0;
    }

    if (file_exists(session_file)) {
        printf("Erreur : Le fichier %s existe déjà. Session déjà exécutée.\n", session_file);
        return 0;
    }

    return 1;
}

int verifier_dossier_precedent(char *ville) {
    int num_ville = atoi(ville);
    char dir_path[256];
    char resume_file[512];
    DIR *dir;
    struct dirent *entry;
    int found = 0;

    if (num_ville <= 1) {
        return 1; // Pas de dossier précédent pour la première ville
    }

    snprintf(dir_path, sizeof(dir_path), "data/fichiers/%d_", num_ville - 1);

    dir = opendir("data/fichiers");
    if (!dir) {
        perror("Erreur lors de l'ouverture du répertoire");
        return 0;
    }

    // Recherche du dossier correspondant
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR && strncmp(entry->d_name, dir_path, strlen(dir_path)) == 0) {
            found = 1;
            snprintf(resume_file, sizeof(resume_file), "data/fichiers/%s/resume_course.csv", entry->d_name);
            break;
        }
    }

    closedir(dir);

    if (!found) {
        printf("Aucun dossier trouvé pour le motif : %s\n", dir_path);
        return 0;
    }

    // Vérifie si le fichier "resume_course.csv" existe
    if (!file_exists(resume_file)) {
        printf("ERREUR: Le fichier resume_course n'a pas été trouvé : %s\n", resume_file);
        return 0;
    }

    return 1;
}


/**
 * @brief Initialise les nombres aléatoires.
 */
void initialize_random() {
    srand(time(NULL));
}

/**
 * @brief Génère un nombre aléatoire entre une valeur minimale et maximale.
 * 
 * @param min Valeur minimale.
 * @param max Valeur maximale.
 * @return Un float aléatoire compris entre [min, max].
 */
float random_float(int min, int max) {
    if (min >= max) {
        printf("Erreur: min >= max dans random_float()\n");
        return min; // Valeur par défaut raisonnable
    }
    return min + (float)rand() / RAND_MAX * (max - min); 
}

/**
 * @brief Estime le nombre maximum de tours possibles en fonction de la durée de la session et du temps maximum d'un tour.
 *
 * @param session_duration Durée de la session en secondes.
 * @param max_time Temps maximal pour un tour en secondes.
 * @return Le nombre estimé de tours.
 */
int estimate_max_laps(int session_duration, float max_time) {
    return session_duration / max_time;
}

/**
 * @brief Supprime les espaces en début et en fin de chaîne de caractères.
 *
 * @param str La chaîne de caractères à nettoyer.
 * @return Un pointeur vers la chaîne nettoyée.
 */
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

/**
 * @brief Renvoie une valeur en fonction du numéro de session, permettant d'éviter les conditions ternaires complexes.
 *
 * @param session_num Numéro de la session (1, 2 ou autre).
 * @param res1 Résultat à retourner si session_num == 1.
 * @param res2 Résultat à retourner si session_num == 2.
 * @param resDefault Résultat par défaut si session_num n'est ni 1 ni 2.
 * @return La valeur correspondante au cas.
 */
int ternaire_moins_criminel(int session_num, int res1, int res2, int resDefault) {
    switch (session_num) {
        case 1:
            return res1; // Q1
        case 2:
            return res2; // Q2
        default:
            return resDefault; // Q3
    }
} // Cordialment, Eloy

/**
 * @brief Vérifie si un week-end est spécial (Sprint).
 *
 * @param ville La ville où se déroule l'événement.
 * @return 1 si le week-end est spécial, 0 sinon.
 */
int is_special_weekend(const char *ville) {
    int nb_resultats;
    char **weekend_types = recuperer_colonne_csv("data/liste_circuits.csv", "Sprint", &nb_resultats);
    int ville_num = atoi(ville);

    if (ville_num > 0 && ville_num <= nb_resultats) {
        int result = atoi(weekend_types[ville_num - 1]);

        // Libération de la mémoire
        for (int i = 0; i < nb_resultats; i++) {
            free(weekend_types[i]);
        }
        free(weekend_types);

        return result == 1;
    }

    // Libération de la mémoire
    for (int i = 0; i < nb_resultats; i++) {
        free(weekend_types[i]);
    }
    free(weekend_types);

    return 0;
}

/**
 * @brief Calcule le nombre total de tours en fonction de la distance de la session et de la longueur du circuit.
 *
 * @param ville La ville où se déroule l'événement.
 * @param session_distance Distance totale de la session en mètres.
 * @return Le nombre estimé de tours.
 */
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


