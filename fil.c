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
    - les billets (messages) difusés sur ce fil
    - les noms des fichiers difusés sur ce fil
    - fils suivants gauche et droite  (sous forme d'ABR)
>Taille d'un billet < 255 octets
>Taille d'un fichiers < 33.5 Mo
*/

int fil_existant(fil *fils, int num_fil){
    fil *tmp = fils;
    while (tmp != NULL)
    {
        if (tmp->numero == num_fil)
        {
            return 1;
        }
        tmp = tmp->suivant;
    }
    return 0;
}

void add_new_fil(fil *fils, char *adresse, int num_fil){
    if(fil_existant(fils, num_fil) == 1){
        printf("Le fil %d existe déjà\n", num_fil);
        return;
    }
    fil *new_fil = malloc(sizeof(fil));
    new_fil->numero = num_fil;
    new_fil->adresse = adresse;
    new_fil->billet = NULL;
    new_fil->fichier = NULL;
    new_fil->abonnes = NULL;
    new_fil->suivant = NULL;

    fil *tmp = fils;
    while (tmp != NULL)
    {
        if (tmp->suivant == NULL)
        {
            tmp->suivant = new_fil;
            return;
        }
        tmp = tmp->suivant;
    }
}


// Inserer une nouvelle personne dans la liste personne d'un fil
void add_new_user(fil *fils, int num_fil, user user){
    fil *tmp = fils;
    while (tmp != NULL)
    {
        if (tmp->numero == num_fil)
        {
            tmp->abonnes = add_user(tmp->abonnes, &user);
        }
        tmp = tmp->suivant;
    }
}

// Inserer un nouveau billet dans la liste billet
void add_new_billet(fil *fils, int num_fil, int id_proprietaire, char *message){
    billet *new_billet = malloc(sizeof(billet));
    new_billet->id_proprietaire = id_proprietaire;
    new_billet->message = message;
    new_billet->suivant = NULL;

    fil *tmp = fils;
    while (tmp != NULL)
    {
        if (tmp->numero == num_fil)
        {
            billet *tmp_billet = tmp->billet;
            while (tmp_billet != NULL)
            {
                if (tmp_billet->suivant == NULL)
                {
                    tmp_billet->suivant = new_billet;
                    return;
                }
                tmp_billet = tmp_billet->suivant;
            }
        }
        tmp = tmp->suivant;
    }
}

// Inserer un nouveau fichier dans la liste fichier
void add_new_fichier(fil *fils, int num_fil, int id_proprietaire, char *nom, char *data){
    fichier *new_fichier = malloc(sizeof(fichier));
    new_fichier->id_proprietaire = id_proprietaire;
    new_fichier->nom = nom;
    new_fichier->data = data;
    new_fichier->suivant = NULL;

    fil *tmp = fils;
    while (tmp != NULL)
    {
        if (tmp->numero == num_fil)
        {
            fichier *tmp_fichier = tmp->fichier;
            while (tmp_fichier != NULL)
            {
                if (tmp_fichier->suivant == NULL)
                {
                    tmp_fichier->suivant = new_fichier;
                    return;
                }
                tmp_fichier = tmp_fichier->suivant;
            }
        }
        tmp = tmp->suivant;
    }
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


