#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include "fil.h"
#include "user.h"

#define LEN_FILE 33554432 // 32 Mo

char current_adr[INET6_ADDRSTRLEN] = "ff02::1";
uint16_t f = 0;
int port_multi = 4444;

void next_adr_multi() {
    char* dernierGroupe = strrchr(current_adr, ':');
    unsigned int dernierGroupeValeur;
    sscanf(dernierGroupe + 1, "%x", &dernierGroupeValeur);
    unsigned int prochainDernierGroupeValeur = dernierGroupeValeur + 1;
    sprintf(dernierGroupe + 1, "%x", prochainDernierGroupeValeur);
}

fil * get_fil(fil * fils, uint16_t num_fil){
    while(fils != NULL){
        if(fils->numero == num_fil) return fils;
        fils = fils->suivant;
    }
    return NULL;
}

//Créer un nouveau fil et l'insérer dans la liste fils, retourne NULL si le numéro du fil est déjà existant
fil * add_new_fil(fil *fils, uint16_t num_fil){
    if(get_fil(fils, num_fil) == NULL){
        fil *new_fil = malloc(sizeof(fil));
        new_fil->numero = num_fil;
        new_fil->billets = NULL;
        new_fil->fichiers = NULL;
        new_fil->abonnes = NULL;
        new_fil->suivant = fils;
        new_fil->port = port_multi;
        port_multi++;
        new_fil->adresse = malloc(INET6_ADDRSTRLEN * sizeof(char));
        strcpy(new_fil->adresse, current_adr);
        next_adr_multi();
        return new_fil;
    }
    return NULL;
}

// Inserer un nouveau billet dans la liste billets d'un fil donné, retoune 0 si le billet a été ajouté -1 sinon
int add_new_billet(fil **fils, uint16_t num_fil, int id_proprietaire, const char * message){
    if(num_fil != 0){
        billet *new_billet = malloc(sizeof(billet));
        new_billet->id_proprietaire = id_proprietaire;
        new_billet->message = malloc(sizeof(message));
        strcpy(new_billet->message, message);
        fil * f = get_fil(*fils, num_fil);
        if(f != NULL){
            new_billet->suivant = f->billets;
            f->billets = new_billet;
            return f->numero;
        }
        return -1;
    }
    *fils = add_new_fil(*fils, f);
    f += 1;
    return add_new_billet(fils, (*fils)->numero, id_proprietaire, message);
}

char * add_new_abonne(fil *fils, uint16_t num_fil, uint16_t id){
    fil * f = get_fil(fils, num_fil);
    if(f != NULL){
        add_user(f->abonnes, id, NULL);
        return f->adresse;
    }
    return NULL;
}


