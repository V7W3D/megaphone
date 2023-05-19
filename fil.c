#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "fil.h"

#define LEN_FILE 33554432 // 32 Mo
#define LEN_MSG 255 


/* -- Fil --
Chaque fil de discussions est constitué de:
    - numéro du fil
    - liste des personnes abonnées
    - Son adresse de multidifusion (FFxx::/8 xx id du groupe)
    - les billets (messages) difusés sur ce fil
    - les noms des fichiers difusés sur ce fil
    - fils suivants gauche et droite  (sous forme d'ABR)
>Taille d'un billet < 255 octets
>Taille d'un fichiers < 33.5 Mo
*/

int add_new_fil(struct fil *fils, char *adresse, int num_fil){
    struct fil *new_fil = malloc(sizeof(struct fil));
    new_fil->numero = num_fil;
    new_fil->adresse = adresse;
    new_fil->billet = NULL;
    new_fil->fichier = NULL;
    new_fil->abonnes = NULL;
    new_fil->gauche = NULL;
    new_fil->droite = NULL;

    struct fil *tmp = fils;
    while (tmp != NULL)
    {
        if (tmp->numero > num_fil)
        {
            if (tmp->gauche == NULL)
            {
                tmp->gauche = new_fil;
                return;
            }
            tmp = tmp->gauche;
        }
        else
        {
            if (tmp->droite == NULL)
            {
                tmp->droite = new_fil;
                return;
            }
            tmp = tmp->droite;
        }
    }
    
}

// Inserer une nouvelle personne dans la liste personne
int add_new_personne(struct user *personnes, user *personne){
    struct user *new_personne = malloc(sizeof(struct user));
    new_personne->abonne = personne;
    new_personne->suivant = NULL;

    struct user *tmp = personnes;
    while (tmp != NULL)
    {
        if (tmp->suivant == NULL)
        {
            tmp->suivant = new_personne;
            return;
        }
        tmp = tmp->suivant;
    }
}

// Inserer un nouveau billet dans la liste billet
int add_new_billet(struct billet *billets, char *message){
    struct billet *new_billet = malloc(sizeof(struct billet));
    new_billet->message = message;
    new_billet->suivant = NULL;

    struct billet *tmp = billets;
    while (tmp != NULL)
    {
        if (tmp->suivant == NULL)
        {
            tmp->suivant = new_billet;
            return;
        }
        tmp = tmp->suivant;
    }
}

// Inserer un nouveau fichier dans la liste fichier
int add_new_fichier(struct fichier *fichiers, char *nom_fichier){
    struct fichier *new_fichier = malloc(sizeof(struct fichier));
    new_fichier->nom_fichier = nom_fichier;
    new_fichier->suivant = NULL;

    struct fichier *tmp = fichiers;
    while (tmp != NULL)
    {
        if (tmp->suivant == NULL)
        {
            tmp->suivant = new_fichier;
            return;
        }
        tmp = tmp->suivant;
    }
}






/*
int main(int argc, char const *argv[]){

    return 0;
}
*/
