#ifndef FIL_H
#define FIL_H

struct user
{
    /* data */
};



struct billet
{
    int numero; // numéro du billet
    int id_proprietaire; // id de l'auteur
    char* message; // message du billet < 255
    struct billet *suivant; // pointeur vers le billet suivant
};


struct fichier
{
    int numero; // numéro du fichier
    int id_proprietaire; // id de l'auteur
    char nom[50]; // nom du fichier
    char* data; // données du fichier [33.5 Mo]
    struct fichier *suivant; // pointeur vers le fichier suivant
};


struct fil
{
    int numero; // numéro du fil
    char *adresse; // adresse de multidifusion?
    struct billet *billet; // (Pile) liste des billets difusés
    struct fichier *fichier; // (Pile) liste des fichiers difusés
    struct user *abonnes; // liste des personnes abonnées
    struct fil *gauche; // pointeur vers le fil suivant
    struct fil *droite; // pointeur vers le fil suivant

};

int add_new_fil(struct fil *fils, char *nom, char *adresse, int num_fil);
int add_new_billet(struct fil *fils, int num_fil, int id_proprietaire, char *message);
int add_new_fichier(struct fil *fils, int num_fil, int id_proprietaire, char *nom, char *data);
int add_new_user(struct fil *fils, int num_fil, int id_user);



#endif