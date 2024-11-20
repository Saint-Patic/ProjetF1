#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "car.h"
#include "utils.h"
#include "display.h"
#include "file_manager.h"

// Fonction pour effectuer toutes les vérifications
int verifier_parametres(char *session_file, char *ville, char *session_type, int *session_num) {

    if (ville == NULL) {
        printf("Erreur d'allocation mémoire\n");
        return 1;
    }

    // Vérifie le format du fichier de session
    if (sscanf(session_file, "fichiers/%49[^/]/%19[a-zA-Z]_%d.csv", ville, session_type, session_num) != 3) {
        printf("Nom de fichier invalide. Utilisez le format fichiers/<x_ville>/<type>_<numéro>.csv\n");
        return 0;
    }

    // Vérifie le type de session
    if (strcmp(session_type, "essai") != 0 && strcmp(session_type, "qualif") != 0 && strcmp(session_type, "course") != 0) {
        printf("Erreur: Type de session invalide.\nTypes attendus: essai - qualif - course\n");
        return 0;
    }

    // Vérifie le nombre maximum de sessions autorisées
    if ((strcmp(session_type, "essai") == 0 && *session_num > MAX_SESSION_ESSAI) ||
        (strcmp(session_type, "qualif") == 0 && *session_num > MAX_SESSION_QUALIF) ||
        (strcmp(session_type, "course") == 0 && *session_num > MAX_SESSION_COURSE)) {
        printf("Il ne peut y avoir plus de %d sessions pour le type %s. Exécution annulée.\n",
            (strcmp(session_type, "essai") == 0) ? MAX_SESSION_ESSAI :
            (strcmp(session_type, "qualif") == 0) ? MAX_SESSION_QUALIF :
            MAX_SESSION_COURSE,
            session_type);
        return 0;
    }

    int num_ville;
    if (sscanf(ville, "%d", &num_ville) != 1) {
        printf("Erreur : Impossible d'extraire le numéro de ville.\n");
        return 0;
    }

    // Vérifications spécifiques aux types de session
    if (strcmp(session_type, "course") == 0 && *session_num == 1) {
        char qualif_resume_file[100];
        snprintf(qualif_resume_file, 100, "fichiers/%s/resume_qualif.csv", ville);
        if (!file_exists(qualif_resume_file)) {
            printf("Erreur: La simulation de course requiert un résumé des qualifications (%s).\n", qualif_resume_file);
            return 0;
        }
    }

    if (strcmp(session_type, "qualif") == 0 && *session_num == 1) {
        char essai_resume_file[100];
        snprintf(essai_resume_file, 100, "fichiers/%s/resume_essai.csv", ville);
        if (!file_exists(essai_resume_file)) {
            printf("Erreur: La simulation de qualification requiert un résumé des essais (%s).\n", essai_resume_file);
            return 0;
        }
    }

    // Crée la chaîne pour la session précédente
    char prev_session_file[100];
    sprintf(prev_session_file, "fichiers/%s/%s_%d.csv", ville, session_type, *session_num - 1);

    if (*session_num < 1) {
        printf("Veuillez indiquer un numéro de session positif.\n");
        return 0;
    }

    if (*session_num > 1 && !file_exists(prev_session_file)) {
        printf("La session précédente %s n'existe pas. Exécution annulée.\n", prev_session_file);
        return 0;
    }

    if (file_exists(session_file)) {
        printf("Le fichier %s existe déjà. La session a déjà été exécutée.\n", session_file);
        return 0;
    }

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
