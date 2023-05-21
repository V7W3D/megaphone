#ifndef FIL_H
#define FIL_H
#include "user.h"
#include <stdint.h>

#define LEN_FILE 33554432 // 32 Mo
#define LEN_MSG 513 

typedef struct billet{
    int numero; // numéro du billet
    int id_proprietaire; // id de l'auteur
    char * message; // message du billet < 255
    struct billet *suivant; // pointeur vers le billet suivant
} billet;


typedef struct fichier {
    int numero; // numéro du fichier
    int id_proprietaire; // id de l'auteur
    char nom[10]; // nom du fichier
    char data[LEN_MSG]; // données du fichier [33.5 Mo]
    struct fichier *suivant; // pointeur vers le fichier suivant
} fichier;


typedef struct fil{
    int numero; // numéro du fil
    int id_proprietaire;
    char * adresse; // adresse de multidifusion?
    billet * billets; // (Pile) liste des billets publiés
    fichier * fichiers; // (Pile) liste des fichiers publiés
    uint16_t * abonnes; // liste des personnes abonnées à ce fil
    struct fil * suivant; // pointeur vers le fil suivant
} fil;

fil * get_fil(fil * fils, int num_fil);
fil * add_new_fil(fil *fils, int id_proprietaire, const char *adresse, int num_fil);
int add_new_billet(fil *fils, int num_fil, int id_proprietaire, const char * message);
int nb_msgs_fil(fil*, int);
int nb_fils(fil*);
int nb_msgs_total_fil(fil*);
int ajout_bloc_fichier(fil *fils, int numfil, int numbloc, fichier *fich);
fichier* creer_fichier(int numeron, int id, char *nom, char *data);

#endif