# ProjetF1

## Projet C - Makefile

Ce projet utilise un `Makefile` pour faciliter la compilation, l'organisation et la gestion des fichiers. Ce guide explique les fonctionnalités et l'utilisation des différentes cibles du `Makefile`.

## Contenu du Projet

Le projet se compose de plusieurs fichiers sources (`.c`) et d'entêtes associés (`.h`). L'objectif est de produire un exécutable nommé `main`.

### Structure des Fichiers

- **`main.c`** : Point d'entrée du programme.
- **`car.c` / `car.h`** : Gestion des fonctionnalités liées aux voitures.
- **`utils.c` / `utils.h`** : Fonctions utilitaires pour le projet.
- **`display.c` / `display.h`** : Fonctions pour afficher les informations.
- **`file_manager.c` / `file_manager.h`** : Gestion des fichiers.
- **`fichiers/`** : Contient les fichiers de simulation (ex : `essai_1.csv`, `qualif_1.csv`).
- **`Makefile`** : Fichier pour automatiser la compilation et la gestion des fichiers.

## Utilisation du Makefile

### Compilation

Pour compiler le projet et générer l'exécutable `main`, exécute la commande suivante dans ton terminal :

```bash
make
```

Cela compile tous les fichiers sources et génère l'exécutable `main` dans le répertoire `bin/`

### Nettoyage des fichiers

Tu peux nettoyer les fichiers générés à chaque étape de la compilation avec les commandes suivantes:

- Supprimer les fichiers objets (`.o`), l'exécutable `main`, et tous les sous-dossiers dans `data/fichiers/` :

```bash
make clean
```

- Supprimer uniquement les sous-dossiers dans `data/fichiers/`:

```bash
make clean_dirs
```

- Supprimer uniquement les fichiers dans `data/fichiers/`(sans toucher aux dossiers):

```bash
make clean_files
```

- Supprimer uniquement les fichiers `.o`:

```bash
make clean_o
```

### Exécution du Programme

Après avoir compilé le projet avec `make`, tu peux exécuter l'exécutable `main`avec des fichiers CSV spécifiques pour les sessoin de simulation (comme `essai_1.csv`,`qualif_1.csv`, etc.):

```bash
./bin/main data/fichiers/$ville/$nom_fichier.csv
```

Assure-toi de remplacer `$ville` par le nom de la ville et `$nom_fichier`par le fichier spécifique que tu souhaites exécuter (par exemple: `essai_1.csv`,`qualif_1.csv`)

### Exécution des Sessions avec `make run_sessions`

Le `Makefile` inclut une cible `run_sessions` qui permet d'exécuter les différents simulation de sessions de course automatiquement.
Cette cible exécute l'exécutable `main` sur les fichiers CSV des sessions de simulation. Ces fichiers CSV doivent être organisés dans un sous dossier de `data/fichiers/`, spécififé par la variable `$(SUBFOLDER)` (par défault, `1_Sakhir`).

#### Fonctionnement de la commande

Lorsque tu exécutes la commande suivante:

```bash
make run_sessions
```

Le `Makefile` effectue les actions suivantes:

1. Compilation (si nécessaire)
2. Exécution des Sessions:

   - Essai 1 à 3
   - Qualification 1 à 3
   - Course

## Structure des Dossiers et Fichiers

### Dossiers et Fichiers Sources:

- `src/`: Contient les fichiers sources `.c`.
- `include/`: Contient les fichiers d'en-tête `.h`.
- `build`: Contient les fichiers objets générés lors de la compilation.
- `bin`: Contient l'exécutable `main` généré.
- `data/fichiers/`: Contient les fichiers de simulation (CSV), organisés par ville et session.

## Fonctionnalités

- Simulation des sessoins de F1: Le programme permet de simuler les session d'essais, de qualifications et de course pour les voitures en fonction des données fournies dans les fichiers CSV.
- Gestion des sessions: Le programme prend en charge les différents types de session: `essai`, `qualif´, `course`, et génère des résultats basés sur ces sessions.
- Gestion des fichiers de données: Le programme utilise des fichiers CSV pour stocker les résultats et permet de combiner les sessions passées dans un résumé.

## Dépendances

Le projet nécessite les bibliothèques standard de C pour fonctionner correctement, ainsi que la structure de sossiers suivante pour l'organisation des fichiers.

```
.
├── Makefile
|   ├──
├── bin
│   ├──
├── build
│   ├──
├── data
│   ├── fichiers
│   |   ├──
└── verification.txt
├── docs
│   ├──
├── include
│   ├──
├── src
│   ├──
└── to_do
```

## Auteurs

- Lemaire Nathan
- Demarcq Alexis
- Mertens Corentin
- Gavage Arthur
