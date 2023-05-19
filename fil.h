#ifndef FIL_H
#define FIL_H


typedef struct billet{
    int numero; // numéro du billet
    int id_proprietaire; // id de l'auteur
    char* message; // message du billet < 255
    struct billet *suivant; // pointeur vers le billet suivant
} billet;


typedef struct fichier {
    int numero; // numéro du fichier
    int id_proprietaire; // id de l'auteur
    char* nom; // nom du fichier
    char* data; // données du fichier [33.5 Mo]
    struct fichier *suivant; // pointeur vers le fichier suivant
} fichier;


typedef struct fil{
    int numero; // numéro du fil
    char *adresse; // adresse de multidifusion?
    billet *billet; // (Pile) liste des billets difusés
    fichier *fichier; // (Pile) liste des fichiers difusés
    luser abonnes; // liste des personnes abonnées
    struct fil *suivant; // pointeur vers le fil suivant
} fil;

void add_new_fil(fil *fils, char *adresse, int num_fil);
void add_new_user(fil *fils, int num_fil, int id_user);
void add_new_billet(fil *fils, int num_fil, int id_proprietaire, char *message);
void add_new_fichier(fil *fils, int num_fil, int id_proprietaire, char *nom, char *data);

#endif