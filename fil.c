#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include "fil.h"
#include "user.h"

#define LEN_FILE 33554432 // 32 Mo

char current_adr[INET6_ADDRSTRLEN];

void init(){
    strncpy(current_adr, "ff02::1", INET6_ADDRSTRLEN);
}

struct sockaddr_in6 get_current_adr() {
    struct sockaddr_in6 adresse;
    memset(&adresse, 0, sizeof(struct sockaddr_in6));
    adresse.sin6_family = AF_INET6;
    inet_pton(AF_INET6, current_adr, &(adresse.sin6_addr));
    return adresse;
}

void next_adr_multi(struct sockaddr_in6* adresseActuelle, struct sockaddr_in6* adresseSuivante) {
    memcpy(adresseSuivante, adresseActuelle, sizeof(struct sockaddr_in6));
    char adresseTexte[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, &(adresseSuivante->sin6_addr), adresseTexte, INET6_ADDRSTRLEN);
    char* dernierGroupe = strrchr(adresseTexte, ':');
    unsigned int dernierGroupeValeur;
    sscanf(dernierGroupe + 1, "%x", &dernierGroupeValeur);
    unsigned int prochainDernierGroupeValeur = dernierGroupeValeur + 1;
    sprintf(dernierGroupe + 1, "%x", prochainDernierGroupeValeur);
    inet_pton(AF_INET6, adresseTexte, &(adresseSuivante->sin6_addr));
}

fil * get_fil(fil * fils, int num_fil){
    while(fils != NULL){
        if(fils->numero == num_fil) return fils;
        fils = fils->suivant;
    }
    return NULL;
}

//Créer un nouveau fil et l'insérer dans la liste fils, retourne NULL si le numéro du fil est déjà existant
fil * add_new_fil(fil *fils, int num_fil){
    if(get_fil(fils, num_fil) == NULL){
        fil *new_fil = malloc(sizeof(fil));
        new_fil->numero = num_fil;
        new_fil->billets = NULL;
        new_fil->fichiers = NULL;
        new_fil->abonnes = NULL;
        new_fil->suivant = fils;
        return new_fil;
    }
    return NULL;
}

// Inserer un nouveau billet dans la liste billets d'un fil donné, retoune 0 si le billet a été ajouté -1 sinon
int add_new_billet(fil *fils, int * f, int num_fil, int id_proprietaire, const char * message){
    if(num_fil != 0){
        billet *new_billet = malloc(sizeof(billet));
        new_billet->id_proprietaire = id_proprietaire;
        strcpy(new_billet->message, message);
        fil * f = get_fil(fils, num_fil);
        if(f != NULL){
            new_billet->suivant = f->billets;
            f->billets = new_billet;
            return f->numero;
        }
        return -1;
    }
    fil * new_fil = add_new_fil(fils, *f);
    *f += 1;
    return add_new_billet(fils, f, new_fil->numero, id_proprietaire, message);
}

void* notificationThread(void* arg) {
    // Boucle pour gérer les notifications en continu
    while (1) {
        // Logique de gestion des notifications
        printf("Traitement des notifications...\n");

        // Temporisation de 20 secondes
        sleep(20);
    }
}



/*
int main(int argc, char const *argv[]){
    pthread_t thread;
    if (pthread_create(&thread, NULL, notificationThread, NULL) != 0) {
        perror("Erreur lors de la création du thread de gestion des notifications");
        exit(EXIT_FAILURE);
    }

    // Continuer avec les autres actions du serveur
    printf("Serveur en cours d'exécution...\n");

    // Attendre la fin du thread de gestion des notifications (ceci ne sera jamais atteint dans cet exemple)
    pthread_join(thread, NULL);

    return 0;
}
*/


