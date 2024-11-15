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


    // check s'il y a des paramètres 
    if (argc != 2) {
        printf("Usage: %s <session_filename>\n", argv[0]);
        printf("Format paramètre attendu:  fichiers/<type>_<numéro>.csv\n");
        return 1;
    }

    char *session_file = argv[1];
    int session_num;


    char *session_type = extract_type_session(session_file);
    if (session_type == NULL) {
        printf("Erreur: Impossible de déterminer le type de session.\n");
        printf("type attendu: essai - qualif - course \n");
        return 1;
    }

    // check si le chemin du fichier est correct
    if (sscanf(session_file, "fichiers/%[a-zA-Z]_%d.csv", session_type, &session_num) != 2) {
        printf("Nom de fichier invalide. Utilisez le format fichiers/<type>_<numéro>.csv\n");
        return 1;
    }


    //session_num = strcmp(session_type, "course") == 0 ? -1 : session_num;
    // Si le type est "qualif", on vérifie que les essais ont déjà un résumé
    if (strcmp(session_type, "qualif") == 0 && session_num == 1) {
        if (!file_exists("fichiers/resume_essai.csv")) {
            printf("Erreur: La simulation de qualification requiert un résumé des essais (resume_essai.csv).\n");
            return 0;
        }
    }

    // crée la chaine prev_session_file avec le numéro précédent de celui mis en paramètre
    char prev_session_file[50];
    sprintf(prev_session_file, "fichiers/%s_%d.csv", session_type, session_num - 1);

    // fichier exite ? ne simule rien : simule la session
    if (file_exists(session_file)) {
        printf("Le fichier %s existe déjà. La session a déjà été exécutée.\n", session_file);
        return 0;
    }

    if (session_num < 1) {
        printf("Veuillez indiquer un numéro de session positif.\n");
        return 0;
    }


    // course précédente existe ? simule ? simule rien
    if (session_num > 1 && !file_exists(prev_session_file)) {
        printf("La session précédente %s n'existe pas. Exécution annulée.\n", prev_session_file);
        return 0;
    }


    // nombre de session autorisée est atteinte ? simule rien : simule
    if (session_num > MAX_SESSION) {
        printf("Il ne peut y avoir plus de %d sessions. Exécution annulée.\n", MAX_SESSION);
        return 0;
    }


    // qualif terminée ? simule Course : rien
        if (strcmp(session_type, "course") == 0 && session_num == 1) {
            if (!file_exists("fichiers/resume_qualif.csv")) {
                printf("Erreur: La simulation de course requiert un résumé des qualif (resume_qualif.csv).\n");
                return 0;
            }
    }


    // ######################## initialision des voitures ########################

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


    // ######################## simulation ########################

    printf("===== Début de la session: %s =====\n\n", session_file);


    if (strcmp(session_type,"qualif") == 0) {
        simulate_qualification(cars, session_num, session_file, MIN_TIME, MAX_TIME, NUM_CARS);
    } else if (strcmp(session_type,"essai") == 0) {
        simulate_sess(cars, NUM_CARS, MIN_TIME, MAX_TIME, session_duration);
        save_session_results(cars, NUM_CARS, session_file, "w");
        printf("Les résultats de la session ont été enregistrés dans %s\n", session_file);
    } else if (strcmp(session_type,"course") == 0) {
        printf("simulation de la course\n");
        //test_recuperer_colonne_csv();
        //create_directories_from_csv_values("liste_circuits.csv", "Course", "Ville");
        supprimer_dossiers_dans_repertoire("fichiers");
    }

    // int_session == MAX_SESSION ? trouver meilleurs temps et secteurs des MAX_SESSION sessions
    process_session_files(session_num, session_type);

    return 0;
}