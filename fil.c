#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <unistd.h>
#include "fil.h"
#include "user.h"

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
fil * add_new_fil(fil *fils, int id_proprietaire, const char * adresse, int num_fil){
    if(get_fil(fils, num_fil) == NULL){
        fil *new_fil = malloc(sizeof(fil));
        new_fil->numero = num_fil;
        new_fil->id_proprietaire = id_proprietaire;
        new_fil->adresse = malloc(sizeof(strlen(adresse)));
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
    new_billet->message = malloc(sizeof(strlen(message)));
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

int nb_msgs_fil(fil* fils, int num_fil){
    fil *f = get_fil(fils, num_fil);
    if (!f) return -1;
    billet* billets = f->billets;
    int nb = 0;
    while (billets){
        nb++;
        billets = billets->suivant;
    }
    return nb;
}

int nb_fils(fil* fils){
    int nb = 0;
    while (fils){
        nb++;
        fils = fils->suivant;
    }
    return nb;
}

int nb_msgs_total_fil(fil* fils){
    int nb = 0;
    while (fils){
        billet* billets = fils->billets;
        while(billets){
            nb++;
            billets = billets->suivant;
        }
        fils = fils->suivant;
    }
    return nb;
}

void ajout_bloc_fichier(int numbloc, char *bloc, fichier *fich){
    data_fichier *data = malloc(sizeof(data_fichier));
    strcpy(data->data, bloc);
    data->suivant = NULL;
    int indice = 1;

    if (numbloc == 1){
        fich->data = data;
    }

    data_fichier *first_bloc = fich->data;
    
    while (indice != numbloc && first_bloc){
        if (indice+1 == numbloc){
            data->suivant = first_bloc->suivant;
            first_bloc->suivant = data;
        }
        first_bloc = first_bloc->suivant;
        indice++;
    }
}

fichier* creer_fichier(fil *mes_fils, int f,int numeron, int id, char *nom){
    fil *fils = get_fil(mes_fils, f);
    fichier *fich = malloc(sizeof(fichier));
    fich->numero = numeron;
    fich->id_proprietaire = id;
    strcpy(fich->nom, nom);
    fich->suivant = fils->fichiers;
    fils->fichiers = fich;
    return fich;
}

fichier *get_fichier(fil *f, char *nom){
    fichier *fichs = f->fichiers;
    while (fichs){
        if (strcmp(fichs->nom, nom) == 0) return fichs;
        fichs = fichs->suivant;
    }
    return NULL;
}

int exsist_fichier(fil *f, char *nom){
    return get_fichier(f, nom) != NULL;
}