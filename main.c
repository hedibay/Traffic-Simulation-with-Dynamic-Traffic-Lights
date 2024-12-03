#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#define DELAI 200000
#define DELAI_CREATION_VEHICULE 1000000 // D�lai entre la cr�ation des v�hicules (1 seconde)
#define MAX_LARGEUR 50
#define MAX_HAUTEUR 50

typedef struct {
    int rouge_x, rouge_y; // Position du feu rouge
    int vert_x, vert_y;   // Position du feu vert
    char etat;            // �tat actuel du feu (R ou V)
    pthread_mutex_t mutex; // Mutex pour synchroniser l'acc�s � l'�tat
} Feu;

typedef struct {
    int x, y;   // Position actuelle
    int dx, dy; // Direction de d�placement
    int actif;  // 1 si le v�hicule est actif (dans la grille), 0 sinon
     pthread_mutex_t mutex;
} Vehicule;


pthread_mutex_t grille_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex global pour la grille
char grille[MAX_HAUTEUR][MAX_LARGEUR];

// Fonction pour lire les param�tres du fichier
void lire_fichier(const char *nom_fichier, int *largeur, int *hauteur, int *nb_vehicules, int *route_verticale, int *route_horizontale) {
    FILE *fichier = fopen(nom_fichier, "r");
    if (!fichier) {
        perror("Erreur lors de l'ouverture du fichier");
        exit(EXIT_FAILURE);
    }

    fscanf(fichier, "%d %d", largeur, hauteur);
    fscanf(fichier, "%d", nb_vehicules);
    fscanf(fichier, "%d %d", route_verticale, route_horizontale);

    fclose(fichier);
}

// Fonction pour afficher la grille
void afficher_grille(char grille[][MAX_LARGEUR], int largeur, int hauteur) {
    system("cls"); // Nettoie l'�cran (fonctionne sous Windows)
    for (int i = 0; i < hauteur; i++) {
        for (int j = 0; j < largeur; j++) {
            putchar(grille[i][j]);
        }
        putchar('\n');
    }
}

// Fonction pour g�rer les feux de signalisation
void *gestion_feu(void *arg) {
    Feu *feu = (Feu *)arg;

    while (1) {
        pthread_mutex_lock(&feu->mutex);
        feu->etat = (feu->etat == 'R') ? 'V' : 'R'; // Alterner entre R et V
        pthread_mutex_unlock(&feu->mutex);

        // Attendre 5 secondes
        sleep(5);
    }

    return NULL;
}


// Fonction pour d�placer les v�hicules avec un mutex
void *deplacer_vehicule(void *arg) {
    Vehicule *vehicule = (Vehicule *)arg;

    while (vehicule->actif) {
        // Verrouiller la grille pour modifier la position du v�hicule
        pthread_mutex_lock(&grille_mutex);

        if (vehicule->dx != 0) {
            grille[vehicule->y][vehicule->x] = '-';
            vehicule->x += vehicule->dx;
        }
        if (vehicule->dy != 0) {
            grille[vehicule->y][vehicule->x] = '|';
            vehicule->y += vehicule->dy;
        }

        // V�rifier si le v�hicule sort de la grille
        if (vehicule->x < 0 || vehicule->x >= MAX_LARGEUR || vehicule->y < 0 || vehicule->y >= MAX_HAUTEUR) {
            vehicule->actif = 0; // Inactif si le v�hicule est hors de la grille
        } else {
            // Mettre � jour la grille si le v�hicule est encore dans la grille
            grille[vehicule->y][vehicule->x] = '*';
        }

        // D�verrouiller la grille apr�s modification
        pthread_mutex_unlock(&grille_mutex);

        //afficher_grille(grille, MAX_LARGEUR, MAX_HAUTEUR);
      //  afficher_grille(grille, MAX_LARGEUR, MAX_HAUTEUR);
        // Attendre un court d�lai pour le mouvement suivant
        usleep(100000); // D�lai pour simuler le mouvement du v�hicule
    }

    return NULL;
}




// Fonction pour g�n�rer la grille avec les routes, feux et v�hicules
void generer_grille(int largeur, int hauteur, int nb_vehicules, int route_verticale, int route_horizontale) {
    Vehicule vehicules[nb_vehicules];
    pthread_t threads_feux[route_verticale * route_horizontale];
    pthread_t threads_vehicules[nb_vehicules];
    Feu feux[route_verticale * route_horizontale];

    int positions_verticales[route_verticale];
    int positions_horizontales[route_horizontale];

    pthread_mutex_t grille_mutex = PTHREAD_MUTEX_INITIALIZER;

    // Initialiser la grille avec des espaces
    for (int i = 0; i < hauteur; i++) {
        for (int j = 0; j < largeur; j++) {
            grille[i][j] = ' ';
        }
    }

    // Dessiner les routes verticales
    for (int i = 0; i < route_verticale; i++) {
        positions_verticales[i] = (i + 1) * (largeur / (route_verticale + 1));
        for (int j = 0; j < hauteur; j++) {
            grille[j][positions_verticales[i]] = '|';
        }
    }

    // Dessiner les routes horizontales
    for (int i = 0; i < route_horizontale; i++) {
        positions_horizontales[i] = (i + 1) * (hauteur / (route_horizontale + 1));
        for (int j = 0; j < largeur; j++) {
            grille[positions_horizontales[i]][j] = '-';
        }
    }

    // Ajouter les feux de signalisation et d�marrer les threads
    int index = 0;
    srand(time(NULL)); // Initialiser la graine du g�n�rateur al�atoire
    for (int i = 0; i < route_horizontale; i++) {
        for (int j = 0; j < route_verticale; j++) {
            int rouge_x = positions_verticales[j] - 1;
            int rouge_y = positions_horizontales[i] - 1;
            int vert_x = positions_verticales[j] + 1;
            int vert_y = positions_horizontales[i] + 1;

            char etat_initial = (rand() % 2 == 0) ? 'R' : 'V'; // Choisir un �tat al�atoire (R ou V)
            feux[index] = (Feu){rouge_x, rouge_y, vert_x, vert_y, etat_initial, PTHREAD_MUTEX_INITIALIZER};

            // Cr�er un thread pour le feu
            pthread_create(&threads_feux[index], NULL, gestion_feu, &feux[index]);
            index++;
        }
    }

    // Cr�er les v�hicules
    for (int i = 0; i < nb_vehicules; i++) {
        int x, y, dx, dy;

        // Boucle pour trouver une position valide pour chaque v�hicule
        do {
            if (rand() % 2 == 0) {
                // Route verticale
                int colonne_verticale = positions_verticales[rand() % route_verticale];
                x = colonne_verticale;
                y = rand() % hauteur;
                dx = 0;
                dy = (rand() % 2 == 0) ? 1 : -1;
            } else {
                // Route horizontale
                int ligne_horizontale = positions_horizontales[rand() % route_horizontale];
                x = rand() % largeur;
                y = ligne_horizontale;
                dx = (rand() % 2 == 0) ? 1 : -1;
                dy = 0;
            }
        } while (grille[y][x] != '-' && grille[y][x] != '|');

        // Initialiser le v�hicule
        vehicules[i] = (Vehicule){x, y, dx, dy, 1, PTHREAD_MUTEX_INITIALIZER};

        // Placer le v�hicule sur la grille
        pthread_mutex_lock(&grille_mutex);
        grille[y][x] = '*';
        pthread_mutex_unlock(&grille_mutex);
    }

    // Boucle principale de gestion de la grille
    while (1) {
        pthread_mutex_lock(&grille_mutex);

        // Mettre � jour les feux dans la grille
        for (int i = 0; i < index; i++) {
            grille[feux[i].rouge_y][feux[i].rouge_x] = (feux[i].etat == 'R') ? 'R' : ' ';
            grille[feux[i].vert_y][feux[i].vert_x] = (feux[i].etat == 'V') ? 'V' : ' ';
        }

        // D�placer les v�hicules ou les arr�ter au feu rouge
        int vehicules_actifs = 0; // Compteur des v�hicules actifs
        for (int i = 0; i < nb_vehicules; i++) {
            if (!vehicules[i].actif) continue;



            int next_x = vehicules[i].x + vehicules[i].dx;
            int next_y = vehicules[i].y + vehicules[i].dy;

            int stop = 0;
            for (int j = 0; j < index; j++) {
                if ((next_x -1== feux[j].rouge_x && next_y -1== feux[j].rouge_y && feux[j].etat == 'R') ||
                    (next_x == feux[j].vert_x && next_y == feux[j].vert_y && feux[j].etat == 'R')) {
                    stop = 1;
                    break;
                }
            }

            if (!stop) {
                // D�placer le v�hicule
                grille[vehicules[i].y][vehicules[i].x] = (vehicules[i].dy != 0) ? '|' : '-';
                vehicules[i].x = next_x;
                vehicules[i].y = next_y;
                grille[vehicules[i].y][vehicules[i].x] = '*';
            }
        }

        pthread_mutex_unlock(&grille_mutex);

        // Afficher la grille
        afficher_grille(grille, largeur, hauteur);

        // Si aucun v�hicule n'est actif, quitter la boucle
    /*    if (vehicules_actifs == nb_vehicules) {
            break;
        }*/

        // Attendre avant la prochaine mise � jour
        usleep(DELAI);
    }

    // Nettoyage
    pthread_mutex_destroy(&grille_mutex);
}





int main() {
    int largeur, hauteur, nb_vehicules, route_verticale, route_horizontale;

    // Lire les param�tres � partir d'un fichier
    lire_fichier("texte.txt", &largeur, &hauteur, &nb_vehicules, &route_verticale, &route_horizontale);

    // G�n�rer la grille avec les v�hicules et les routes
    generer_grille(largeur, hauteur, nb_vehicules, route_verticale, route_horizontale);

    return 0;
}
