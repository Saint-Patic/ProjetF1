# ProjetF1

# Projet C - Makefile

Ce projet utilise un `Makefile` pour faciliter la compilation, l'organisation et la gestion des fichiers. Ce guide explique les fonctionnalités et l'utilisation des différentes cibles du `Makefile`.

---

## Contenu du Projet

Le projet se compose de plusieurs fichiers sources (`.c`) et d'entêtes associés (`.h`). L'objectif est de produire un exécutable nommé `main`.

### Structure des Fichiers

- **`main.c`** : Point d'entrée du programme.
- **`car.c` / `car.h`** : Gestion des fonctionnalités liées aux voitures.
- **`utils.c` / `utils.h`** : Fonctions utilitaires pour le projet.
- **`display.c` / `display.h`** : Fonctions pour afficher les informations.
- **`file_manager.c` / `file_manager.h`** : Gestion des fichiers.

---

## Utilisation du Makefile

### Compilation

Pour compiler le projet et générer l'exécutable :

```bash
make
```

pour nettoyer les fichiers.o, dossiers, fichiers.csv ou tout d'un coup, il existe respectivement:

```bash
make clean
make clean_dirs
make clean_files
make clean_all
```

Après avoir exécuté le `Makefile`, exécuté la commande `./main fichiers/$ville/$nom_fichier.csv`
