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

extern char current_adr[INET6_ADDRSTRLEN];
extern uint16_t f;
extern int port_multi;

typedef struct billet{
    int numero; // numéro du billet
    int id_proprietaire; // id de l'auteur
    char * message; // message du billet < 255
    struct billet *suivant; // pointeur vers le billet suivant
} billet;


typedef struct fichier {
    int numero; // numéro du fichier
    int id_proprietaire; // id de l'auteur
    char * nom; // nom du fichier
    char * data; // données du fichier [33.5 Mo]
    struct fichier *suivant; // pointeur vers le fichier suivant
} fichier;


typedef struct fil{
    int numero; // numéro du fil
    char * adresse; // adresse de multidiffusion?
    int port; //port de multidiffusion
    billet * billets; // (Pile) liste des billets publiés
    fichier * fichiers; // (Pile) liste des fichiers publiés
    lusers abonnes; // liste des personnes abonnées à ce fil
    struct fil * suivant; // pointeur vers le fil suivant
} fil;

fil * get_fil(fil * fils, uint16_t num_fil);
fil * add_new_fil(fil *fils, uint16_t num_fil);
int add_new_billet(fil **fils, uint16_t num_fil, int id_proprietaire, const char * message);
char * add_new_abonne(fil *fils, uint16_t num_fil, uint16_t id);

#endif