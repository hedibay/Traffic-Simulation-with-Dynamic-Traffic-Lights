#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#define DELAI 200000
#define DELAI_CREATION_VEHICULE 1000000 // Délai entre la création des véhicules (1 seconde)
#define MAX_LARGEUR 50
#define MAX_HAUTEUR 50

typedef struct {
    int rouge_x, rouge_y; // Position du feu rouge
    int vert_x, vert_y;   // Position du feu vert
    char etat;            // État actuel du feu (R ou V)
    pthread_mutex_t mutex; // Mutex pour synchroniser l'accès à l'état
} Feu;

typedef struct {
    int x, y;   // Position actuelle
    int dx, dy; // Direction de déplacement
    int actif;  // 1 si le véhicule est actif (dans la grille), 0 sinon
     pthread_mutex_t mutex;
} Vehicule;


pthread_mutex_t grille_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex global pour la grille
char grille[MAX_HAUTEUR][MAX_LARGEUR];

// Fonction pour lire les paramètres du fichier
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
    system("cls"); // Nettoie l'écran (fonctionne sous Windows)
    for (int i = 0; i < hauteur; i++) {
        for (int j = 0; j < largeur; j++) {
            putchar(grille[i][j]);
        }
        putchar('\n');
    }
}

// Fonction pour gérer les feux de signalisation
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

void deplacer_vehicule(char grille[][MAX_HAUTEUR], int largeur, int hauteur, Vehicule *vehicule, Feu *feux, int nb_feux, pthread_mutex_t *grille_mutex) {
    if (!vehicule->actif) return;

    int next_x = vehicule->x + vehicule->dx;
    int next_y = vehicule->y + vehicule->dy;

    int stop = 0;
    for (int j = 0; j < nb_feux; j++) {
            if (vehicule->dy!=0){
        if (next_x-1 == feux[j].rouge_x && next_y-1 == feux[j].rouge_y && feux[j].etat == 'V')  {
            stop = 1;
            break;
        }
    }
    else {if (next_x-1 == feux[j].rouge_x && next_y-1 == feux[j].rouge_y && feux[j].etat == 'R')  {
            stop = 1;
            break;
        }}}
    pthread_mutex_lock(grille_mutex); // Accès sécurisé à la grille
    if (grille[next_y][next_x] == '*') { // Si une voiture est devant
        stop = 1;
    }
    pthread_mutex_unlock(grille_mutex);
    pthread_mutex_lock(grille_mutex);
    if (!stop) {
        // Déplacer le véhicule
        grille[vehicule->y][vehicule->x] = (vehicule->dy != 0) ? '|' : '-';
        vehicule->x = next_x;
        vehicule->y = next_y;
        grille[vehicule->y][vehicule->x] = '*';
    }
    pthread_mutex_unlock(grille_mutex);
}





// Fonction pour initialiser la grille avec des espaces
void initialiser_grille(char grille[][MAX_LARGEUR], int largeur, int hauteur) {
    for (int i = 0; i < hauteur; i++) {
        for (int j = 0; j < largeur; j++) {
            grille[i][j] = ' ';
        }
    }
}

// Fonction pour dessiner les routes verticales et enregistrer leurs positions
void dessiner_routes_verticales(char grille[][MAX_LARGEUR], int largeur, int hauteur, int route_verticale, int positions_verticales[]) {
    for (int i = 0; i < route_verticale; i++) {
        positions_verticales[i] = (i + 1) * (largeur / (route_verticale + 1));
        for (int j = 0; j < hauteur; j++) {
            grille[j][positions_verticales[i]] = '|';
        }
    }
}

// Fonction pour dessiner les routes horizontales et enregistrer leurs positions
void dessiner_routes_horizontales(char grille[][MAX_LARGEUR], int largeur, int hauteur, int route_horizontale, int positions_horizontales[]) {
    for (int i = 0; i < route_horizontale; i++) {
        positions_horizontales[i] = (i + 1) * (hauteur / (route_horizontale + 1));
        for (int j = 0; j < largeur; j++) {
            grille[positions_horizontales[i]][j] = '-';
        }
    }
}

// Fonction pour initialiser les feux de signalisation et démarrer les threads
int initialiser_feux(Feu feux[], int route_verticale, int route_horizontale, int positions_verticales[], int positions_horizontales[], pthread_t threads_feux[]) {
    int index = 0;
    srand(time(NULL));
    for (int i = 0; i < route_horizontale; i++) {

        for (int j = 0; j < route_verticale; j++) {

            int rouge_x = positions_verticales[j] - 1;
            int rouge_y = positions_horizontales[i] - 1;
            int vert_x = positions_verticales[j] + 1;
            int vert_y = positions_horizontales[i] + 1;

            char etat_initial_rouge = (rand() % 2 == 0) ? 'R' : 'V' ;// Alterner les feux
           // char etat_initial_vert = (etat_initial_rouge == 'R') ? 'V' : 'R';
            feux[index] = (Feu){rouge_x, rouge_y, vert_x, vert_y, etat_initial_rouge, PTHREAD_MUTEX_INITIALIZER};
           // feux[index + 1] = (Feu){rouge_x , rouge_y, vert_x , vert_y, etat_initial_vert, PTHREAD_MUTEX_INITIALIZER};


            pthread_create(&threads_feux[index], NULL, gestion_feu, &feux[index]);

            index++;
        }
    }
    return index;
}


// Fonction pour initialiser les véhicules sur les routes
void initialiser_vehicules(char grille[][MAX_LARGEUR], int largeur, int hauteur, int nb_vehicules, int route_verticale, int route_horizontale, int positions_verticales[], int positions_horizontales[], Vehicule vehicules[]) {
    for (int i = 0; i < nb_vehicules; i++) {
        int x, y, dx, dy;

        do {
            if (rand() % 2 == 0) {
                int colonne_verticale = positions_verticales[rand() % route_verticale];
                x = colonne_verticale;
                y = rand() % hauteur;
                dx = 0;
                dy = -1;
            } else {
                int ligne_horizontale = positions_horizontales[rand() % route_horizontale];
                x = rand() % largeur;
                y = ligne_horizontale;
                dx =+1;
                dy = 0;
            }
        } while (grille[y][x] != '-' && grille[y][x] != '|');

        vehicules[i] = (Vehicule){x, y, dx, dy, 1, PTHREAD_MUTEX_INITIALIZER};

        pthread_mutex_lock(&grille_mutex);
        grille[y][x] = '*';
        pthread_mutex_unlock(&grille_mutex);
    }
}



// Fonction pour gérer la simulation principale
void simulation_principale(char grille[][MAX_LARGEUR], int largeur, int hauteur, int nb_vehicules, Vehicule vehicules[], Feu feux[], int nb_feux) {
    int vehicules_actifs = nb_vehicules;

    while (vehicules_actifs > 0) {
        pthread_mutex_lock(&grille_mutex);
for (int i = 0; i < nb_feux; i += 1) { // Parcourir les feux par paires
    grille[feux[i].rouge_y][feux[i].rouge_x] = feux[i].etat;
    grille[feux[i].vert_y][feux[i].vert_x] = (feux[i].etat == 'R') ? 'V' : 'R';
}
pthread_mutex_unlock(&grille_mutex);


        // Déplacer les véhicules
        vehicules_actifs = 0; // Réinitialiser le compteur
        for (int i = 0; i < nb_vehicules; i++) {
            if (vehicules[i].actif) {
                deplacer_vehicule(grille, largeur, hauteur, &vehicules[i], feux, nb_feux, &grille_mutex);

                // Vérifier si le véhicule est sorti de la grille
                if (vehicules[i].x < 0 || vehicules[i].x >= largeur || vehicules[i].y < 0 || vehicules[i].y >= hauteur) {
                    vehicules[i].actif = 0;
                } else {
                    vehicules_actifs++;
                }
            }
        }

        afficher_grille(grille, largeur, hauteur);
        usleep(DELAI);
    }
}

// Fonction principale
void generer_grille(int largeur, int hauteur, int nb_vehicules, int route_verticale, int route_horizontale) {
    Vehicule vehicules[nb_vehicules];
    pthread_t threads_feux[route_verticale * route_horizontale];
    Feu feux[route_verticale * route_horizontale];

    int positions_verticales[route_verticale];
    int positions_horizontales[route_horizontale];

    initialiser_grille(grille, largeur, hauteur);
    dessiner_routes_verticales(grille, largeur, hauteur, route_verticale, positions_verticales);
    dessiner_routes_horizontales(grille, largeur, hauteur, route_horizontale, positions_horizontales);

    int nb_feux = initialiser_feux(feux, route_verticale, route_horizontale, positions_verticales, positions_horizontales, threads_feux);
    initialiser_vehicules(grille, largeur, hauteur, nb_vehicules, route_verticale, route_horizontale, positions_verticales, positions_horizontales, vehicules);

    simulation_principale(grille, largeur, hauteur, nb_vehicules, vehicules, feux, nb_feux);

    pthread_mutex_destroy(&grille_mutex);
}




int main() {
    int largeur, hauteur, nb_vehicules, route_verticale, route_horizontale;

    // Lire les paramètres à partir d'un fichier
    lire_fichier("texte.txt", &largeur, &hauteur, &nb_vehicules, &route_verticale, &route_horizontale);

    // Générer la grille avec les véhicules et les routes
    generer_grille(largeur, hauteur, nb_vehicules, route_verticale, route_horizontale);

    return 0;
}
