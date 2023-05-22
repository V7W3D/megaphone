#ifndef FIL_H
#define FIL_H
#include "user.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define LEN_FILE 33554432 // 32 Mo
#define LEN_MSG 512

#define SET_END_AT(f,i) *(f+i) = '\0'

extern char current_adr[INET6_ADDRSTRLEN];
extern uint16_t f;
extern int port_multi;

typedef struct billet{
    int numero; // numéro du billet
    char pseudo[10]; // id de l'auteur
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
    int port; //port de multidiffusion
    billet * billets; // (Pile) liste des billets publiés
    fichier * fichiers; // (Pile) liste des fichiers publiés
    lusers abonnes; // liste des personnes abonnées à ce fil
    struct fil * suivant; // pointeur vers le fil suivant
} fil;

fil * get_fil(fil * fils, uint16_t num_fil);
fil * add_new_fil(fil *fils, uint16_t num_fil);
int nb_msgs_fil(fil*, int);
int nb_fils(fil*);
int nb_msgs_total_fil(fil*);
void ajout_bloc_fichier(int numbloc, char *bloc, fichier *fich);
fichier* creer_fichier(fil *mes_fils, int f, int numeron, int id, char *nom);
int exsist_fichier(fil *f, char *nom);
fichier *get_fichier(fil *f, char *nom);
char * add_new_abonne(fil *fils, uint16_t num_fil, uint16_t id);
int add_new_billet(fil **fils, uint16_t num_fil, const char * pseudo, const char * message);

#endif