#ifndef FIL_H
#define FIL_H
#include "user.h"
#include <stdint.h>

#define LEN_FILE 33554432 // 32 Mo
#define LEN_MSG 512

#define SET_END_AT(f,i) *(f+i) = '\0'


typedef struct billet{
    int numero; // numéro du billet
    int id_proprietaire; // id de l'auteur
    char * message; // message du billet < 255
    struct billet *suivant; // pointeur vers le billet suivant
} billet;


typedef struct data_fichier {
    char data[LEN_MSG]; // données du fichier
    struct data_fichier *suivant; // pointeur vers le fichier suivant
} data_fichier;

typedef struct fichier{
    int numero; // numéro du fichier
    int id_proprietaire; // id de l'auteur
    char nom[10]; // nom du fichier
    data_fichier *data;
    struct fichier *suivant;
}fichier;

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
void ajout_bloc_fichier(int numbloc, char *bloc, fichier *fich);
fichier* creer_fichier(fil *mes_fils, int f, int numeron, int id, char *nom);
int exsist_fichier(fil *f, char *nom);
fichier *get_fichier(fil *f, char *nom);

#endif