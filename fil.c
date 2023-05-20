#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include "fil.h"
#include "user.h"

#define LEN_FILE 33554432 // 32 Mo
#define LEN_MSG 255 


/* -- Fil --
Chaque fil de discussions est constitué de:
    - numéro du fil
    - liste des personnes abonnées
    - Son adresse de multidifusion (FFxx::/8 xx id du groupe)
    - les billets (messages) publiés sur ce fil
    - les noms des fichiers publiés sur ce fil
    - fils suivants gauche et droite  (sous forme d'ABR)
>Taille d'un billet < 255 octets
>Taille d'un fichiers < 33.5 Mo
*/

fil * get_fil(fil * fils, int num_fil){
    while(fils != NULL){
        if(fils->numero == num_fil) return fils;
        fils = fils->suivant;
    }
    return NULL;
}

//Créer un nouveau fil et l'insérer dans la liste fils, retourne NULL si le numéro du fil est déjà existant
fil * add_new_fil(fil *fils, const char * adresse, int num_fil){
    if(get_fil(fils, num_fil) == NULL){
        fil *new_fil = malloc(sizeof(fil));
        new_fil->numero = num_fil;
        strcpy(new_fil->adresse, adresse);
        new_fil->billets = NULL;
        new_fil->fichiers = NULL;
        new_fil->abonnes = NULL;
        new_fil->suivant = fils;
        return new_fil;
    }
    return NULL;
}

// Inserer un nouveau billet dans la liste billets d'un fil donné, retoune 0 si le biller a été ajouté 1 sinon
int add_new_billet(fil *fils, int num_fil, int id_proprietaire, const char * message){
    billet *new_billet = malloc(sizeof(billet));
    new_billet->id_proprietaire = id_proprietaire;
    strcpy(new_billet->message, message);
    fil * f = get_fil(fils, num_fil);
    if(f != NULL){
        new_billet->suivant = f->billets;
        f->billets = new_billet;
        return 0;
    }
    return 1;
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


