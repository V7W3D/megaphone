#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>


/* -- Fil --
Chaque fil de discussions est constitué de:
    - numéro du fil
    - liste des personnes abonnées
    - Son adresse de multidifusion
    // 224.0.0.0 et 239.255.255.255 ou FFxx::/8 xx id du groupe
    - les billets (messages) difusés
    - les noms des fichiers difusés
    - fils suivants gauche et droite  (sous forme d'ABR)
*/

// Taille d'un billet < 255 octets
// Taille d'un fichiers < 33.5 Mo

typedef struct {
    unsigned short entete;
    unsigned short NUMFIL;
    unsigned short NB;
    unsigned short DATALEN;
    char DATA[256];
} msg; // Changer le nom de la structure 


// A ajouter au niveau du serveur

//----------------------------------------//
struct billet
{
    int numero; // numéro du billet
    char id_proprietaire[50]; // nom de l'auteur
    char message[255]; // message du billet
    struct billet *suivant; // pointeur vers le billet suivant
};


struct fichier
{
    int numero; // numéro du fichier
    char id_proprietaire[50]; // nom de l'auteur
    char nom[50]; // nom du fichier
    char data[33554432]; // données du fichier [33.5 Mo]
    struct fichier *suivant; // pointeur vers le fichier suivant
};


struct fil
{
    int numero; // numéro du fil
    char *nom; // nom du fil
    char *adresse; // adresse de multidifusion ??
    struct billet *billet; // (Pile) liste des billets difusés
    struct fichier *fichier; // (Pile) liste des fichiers difusés
    struct personnes *personne; // liste des personnes abonnées
    struct fil *gauche; // pointeur vers le fil suivant
    struct fil *droite; // pointeur vers le fil suivant

};
//----------------------------------------//


unsigned short calculerEntete(unsigned short CODEREQ, unsigned short ID) {
    unsigned short entete = 0;
    entete |= ((CODEREQ & 0x1F) << 11);  // Décalage de CODEREQ de 11 bits et masquage pour conserver uniquement les 5 bits les plus significatifs
    entete |= (ID & 0x7FF);              // ID occupe les 11 bits les moins significatifs
    return entete;
}


int add_new_fil(struct fil *fils, char *nom, char *adresse, int num_fil){
    struct fil *new_fil = malloc(sizeof(struct fil));
    new_fil->numero = num_fil;
    new_fil->nom = nom;
    new_fil->adresse = adresse;
    new_fil->billet = NULL;
    new_fil->fichier = NULL;
    new_fil->personne = NULL;
    new_fil->gauche = NULL;
    new_fil->droite = NULL;

    struct fil *tmp = fils;
    while (tmp != NULL)
    {
        if (tmp->numero > num_fil)
        {
            if (tmp->gauche == NULL)
            {
                tmp->gauche = new_fil;
                return;
            }
            tmp = tmp->gauche;
        }
        else
        {
            if (tmp->droite == NULL)
            {
                tmp->droite = new_fil;
                return;
            }
            tmp = tmp->droite;
        }
    }
    
}

// Inserer une nouvelle personne dans la liste personne
//void add_new_user()  -- A voir avec la structre de Ahmed





int main(int argc, char const *argv[])
{

    /*
    msg datagramme;
    unsigned short CODEREQ = 2;
    unsigned short ID;
    printf("Entrez votre ID : ");
    scanf("%hu", &ID);
    datagramme.entete = calculerEntete(CODEREQ, ID);
    printf("Entrez le numéro du fil : ");
    scanf("%hu", &datagramme.NUMFIL);
    datagramme.NB = 0;
    printf("Entrez les données : ");
    scanf("%s", datagramme.DATA);
    datagramme.DATALEN = strlen(datagramme.DATA);

    // Envoi du datagramme au serveur
    if (sendto(clientSocket, &datagramme, sizeof(Datagramme), 0, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("Erreur lors de l'envoi du datagramme");
        exit(EXIT_FAILURE);
    }

    */
    
    return 0;
}
