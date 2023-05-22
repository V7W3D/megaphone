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

char current_adr[INET6_ADDRSTRLEN] = "ff02::1";
uint16_t f = 1;
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
int add_new_billet(fil **fils, uint16_t num_fil, const char * pseudo, const char * message){
    if(num_fil != 0){
        billet *new_billet = malloc(sizeof(billet));
        strcpy(new_billet->pseudo, pseudo);
        new_billet->message = malloc(sizeof(message));
        strcpy(new_billet->message, message);
        fil * f = get_fil(*fils, num_fil);
        if(f != NULL){
            if(f->billets != NULL)
                new_billet->numero = f->billets->numero + 1;
            else new_billet->numero = 1;
            new_billet->suivant = f->billets;
            f->billets = new_billet;
            return f->numero;
        }
        return -1;
    }
    *fils = add_new_fil(*fils, f);
    f += 1;
    return add_new_billet(fils, (*fils)->numero, pseudo, message);
}

char * add_new_abonne(fil *fils, uint16_t num_fil, uint16_t id){
    fil * f = get_fil(fils, num_fil);
    if(f != NULL){
        add_user(f->abonnes, id, NULL);
        return f->adresse;
    }
    return NULL;
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
    int min = (strlen(bloc) > 512) ? 512 : strlen(bloc);
    SET_END_AT(data->data, min);
    data->suivant = NULL;
    int indice = 1;

    if (numbloc == 1){
        data->suivant = fich->data;
        fich->data = data;
        return;
    }

    data_fichier *first_bloc = fich->data;
    
    while (indice+1 != numbloc){
        first_bloc = first_bloc->suivant;
        indice++;
    }

    data->suivant = first_bloc->suivant;
    first_bloc->suivant = data;

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