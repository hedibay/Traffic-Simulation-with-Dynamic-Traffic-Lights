
#ifndef MAIN_H
#define MAIN_H

void lire_fichier(const char *nom_fichier, int *largeur, int *hauteur, int *nb_vehicules, int *route_verticale, int *route_horizontale) ;
void afficher_grille(char grille[][MAX_LARGEUR], int largeur, int hauteur) ;
void deplacer_vehicules(Vehicule vehicules[], int nb_vehicules, char grille[][MAX_LARGEUR], int largeur, int hauteur) ;
void generer_grille(int largeur, int hauteur, int nb_vehicules, int route_verticale, int route_horizontale) ;
