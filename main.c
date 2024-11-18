#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include "car.h"
#include "display.h"
#include "file_manager.h"

#define NUM_CARS 20
#define MIN_TIME 25
#define MAX_TIME 45




int main(int argc, char *argv[]) {

    // ######################## Phase de check ########################

    // Vérifie s'il y a des paramètres
    if (argc != 2) {
        printf("Usage: %s <session_filename>\n", argv[0]);
        printf("Format paramètre attendu: fichiers/<x_ville>/<type>_<numéro>.csv\n");
        return 1;
    }

    char *session_file = argv[1];
    int session_num;
    char session_type[20];
    char ville[50];

    // Vérifie si le chemin du fichier est correct
    if (sscanf(session_file, "fichiers/%[^/]/%[a-zA-Z]_%d.csv", ville, session_type, &session_num) != 3) {
        printf("Nom de fichier invalide. Utilisez le format fichiers/<x_ville>/<type>_<numéro>.csv\n");
        return 1;
    }

    // Vérifie si le type de session est valide
    if (strcmp(session_type, "essai") != 0 && strcmp(session_type, "qualif") != 0 && strcmp(session_type, "course") != 0) {
        printf("Erreur: Type de session invalide.\n");
        printf("Types attendus: essai - qualif - course\n");
        return 1;
    }

    // Vérifie si le nombre de sessions autorisées est atteint
    if ((strcmp(session_type, "essai") == 0 && session_num > MAX_SESSION_ESSAI) ||
        (strcmp(session_type, "qualif") == 0 && session_num > MAX_SESSION_QUALIF) ||
        (strcmp(session_type, "course") == 0 && session_num > MAX_SESSION_COURSE)) {
        printf("Il ne peut y avoir plus de %d sessions pour le type %s. Exécution annulée.\n", 
            (strcmp(session_type, "essai") == 0) ? MAX_SESSION_ESSAI : 
            (strcmp(session_type, "qualif") == 0) ? MAX_SESSION_QUALIF : 
            MAX_SESSION_COURSE, 
            session_type);
        return 0;
    }

    // ######################## Vérifications spécifiques aux sessions ########################

    // Si le type est "course", vérifie que les qualifications sont terminées
    if (strcmp(session_type, "course") == 0 && session_num == 1) {
        char qualif_resume_file[100];
        snprintf(qualif_resume_file, 100, "fichiers/%s/resume_qualif.csv", ville);
        if (!file_exists(qualif_resume_file)) {
            printf("Erreur: La simulation de course requiert un résumé des qualifications (%s).\n", qualif_resume_file);
            return 0;
        }
    }

    // Si le type est "qualif", vérifie que les essais ont déjà un résumé
    if (strcmp(session_type, "qualif") == 0 && session_num == 1) {
        char essai_resume_file[100];
        snprintf(essai_resume_file, 100, "fichiers/%s/resume_essai.csv", ville);
        if (!file_exists(essai_resume_file)) {
            printf("Erreur: La simulation de qualification requiert un résumé des essais (%s).\n", essai_resume_file);
            return 0;
        }
    }

    // Crée la chaîne pour la session précédente
    char prev_session_file[100];
    sprintf(prev_session_file, "fichiers/%s/%s_%d.csv", ville, session_type, session_num - 1);

    // Vérifie si le fichier existe
    if (file_exists(session_file)) {
        printf("Le fichier %s existe déjà. La session a déjà été exécutée.\n", session_file);
        return 0;
    }

    if (session_num < 1) {
        printf("Veuillez indiquer un numéro de session positif.\n");
        return 0;
    }

    // Vérifie si la session précédente existe
    if (session_num > 1 && !file_exists(prev_session_file)) {
        printf("La session précédente %s n'existe pas. Exécution annulée.\n", prev_session_file);
        return 0;
    }

    // ######################## Initialisation des voitures ########################

    srand(time(NULL));
    int session_duration = 3600;
    struct CarTime cars[NUM_CARS];
    int car_numbers[NUM_CARS] = {1, 11, 44, 63, 16, 55, 4, 81, 14, 18, 10, 31, 23, 2, 22, 3, 77, 24, 20, 27};

    for (int i = 0; i < NUM_CARS; i++) {
        cars[i].car_number = car_numbers[i];
        cars[i].best_lap_time = 0;
        cars[i].temps_rouler = 0;
        cars[i].pit_stop = 0;
        cars[i].out = 0;
        for (int j = 0; j < NUM_SECTORS; j++) {
            cars[i].sector_times[j] = 0;
            cars[i].best_sector_times[j] = 0;
        }
    }

    // ######################## Simulation ########################

    printf("===== Début de la session: %s =====\n\n", session_file);
    create_directories_from_csv_values("liste_circuits.csv", "Course", "Ville");

    if (strcmp(session_type, "qualif") == 0) {
        simulate_qualification(cars, session_num, ville, MIN_TIME, MAX_TIME, NUM_CARS);
    } else if (strcmp(session_type, "essai") == 0) {
        simulate_sess(cars, NUM_CARS, MIN_TIME, MAX_TIME, session_duration);
        save_session_results(cars, NUM_CARS, session_file, "w");
        printf("Les résultats de la session ont été enregistrés dans %s\n", session_file);
    } else if (strcmp(session_type, "course") == 0) {
        printf("simulation de la course\n");
        test_recuperer_colonne_csv();
    }

    // Processus des fichiers de session
    process_session_files(session_num, ville, session_type);

    return 0;
}
