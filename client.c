#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include "user.h"
#include "msgcli.h"
#include "msgsrv.h"

#define BUF_SIZE 256

void* recevoir_notification(void* arg){
    adr_port * a = (adr_port *) arg;
    int sock = socket(AF_INET6, SOCK_DGRAM, 0);

    struct sockaddr_in6 grsock;
    memset(&grsock, 0, sizeof(grsock));
    grsock.sin6_family = AF_INET6;
    grsock.sin6_addr = in6addr_any;
    grsock.sin6_port = htons(a->port);

    if(bind(sock, (struct sockaddr*)&grsock, sizeof(grsock))) {
        close(sock);
        return NULL;
    }

    struct ipv6_mreq group;
    inet_pton (AF_INET6, a->adr, &group.ipv6mr_multiaddr.s6_addr);
    int ifindex = if_nametoindex ("eth0");
    group.ipv6mr_interface = ifindex;
    if(setsockopt(sock, IPPROTO_IPV6, IPV6_JOIN_GROUP, &group, sizeof(group))<0){
        close(sock);
        return NULL;
    }
    while(1){

    }
}

msg_inscri * inscription(const char * pseudo){
    uint16_t e = compose_entete(0,0);
    msg_inscri * m = compose_msg_inscri(e, pseudo);
    return m;
}

msg_fil * poster_billet(uint16_t id, uint16_t f, const char * message){
    msg_fil * m = compose_msg_fil(message, 2, id, f, 0);
    return m;
}

msg_fil * demande_abonnement(uint16_t id, uint16_t f){
    msg_fil * m = compose_msg_fil(NULL, 4, id, f, 0);
    return m;
}

void sabonner(int sockfd, const char * buffer){
    //Je pense que ça serait mieux d'avoir plusieurs sockets ?
    msg_srv_fil * mf = malloc(sizeof(msg_srv_fil));
    memcpy(mf, buffer, sizeof(msg_srv_fil));
    char data_buf[16];
    memcpy(data_buf, mf->adr, 16);
    struct ipv6_mreq mreq;
    if (setsockopt(sockfd, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, (char*)&mreq, sizeof(mreq)) < 0) {
        perror("Erreur lors de l'abonnement à l'adresse de multidiffusion");
    }

}

int main(){
    int sock = socket(PF_INET6, SOCK_DGRAM, 0);
    if(sock < 0) {
        perror("socket()");
        return -1;
    }

    int ok = 1;
    if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &ok, sizeof(ok)) < 0) {
        perror("echec de SO_REUSEADDR");
        close(sock);
        return 1;
    }

    struct sockaddr_in6 servadr;
    memset(&servadr, 0, sizeof(servadr));
    servadr.sin6_family = AF_INET6;
    servadr.sin6_port = htons(7777);
    inet_pton(AF_INET6, "::1", &servadr.sin6_addr);

    
    
    msg_inscri * m = NULL;
    msg_fil * ma = NULL;
    char send_buffer[sizeof(msg_inscri)];
    char recv_buffer[sizeof(msg_srv)];
    char nomFichierTelechargement[100], nomFichier[100], message[256], pseudo[11];

    uint8_t codeReq = 0;
    uint16_t id = 0;
    uint16_t e = *((uint16_t*)recv_buffer);
    int numFil, n, f, numFilAbonnement, numFilFichier, port, numFilTelechargement, choix;


    
    

    do {
        memset(send_buffer, 0, sizeof(send_buffer));
        memset(recv_buffer, 0, sizeof(recv_buffer));
        
        printf("----------------------------------------\n");
        printf("Menu :\n");
        printf("1. Inscription\n");
        printf("2. Poster un billet\n");
        printf("3. Demander les n derniers billets\n");
        printf("4. S'abonner à un fil\n");
        printf("5. Ajouter un fichier\n");
        printf("6. Télécharger un fichier\n");
        printf("0. Quitter\n");
        choix = -1;
        printf("Choisissez une option : ");
        scanf("%d", &choix);

        switch (choix) {
            case 1: {
                printf("Option : Inscription\n");
                
                printf("Saisissez votre pseudo : ");
                scanf("%s", pseudo);
                // Fonction à définir ou  supp la condition
                if (is_user_registered(my_users, pseudo) == 0) {
                    printf("Erreur : L'utilisateur est déjà inscrit\n");
               
                } else {
                    // Ajouter l'utilisateur à la liste des utilisateurs
                    m = inscription(pseudo);
                    memcpy(send_buffer, m, sizeof(msg_inscri));

                    if (sendto(sock, send_buffer, sizeof(msg_inscri), 0, (struct sockaddr*)&servadr, sizeof(servadr)) < 0) {
                        perror("Erreur lors de l'envoi du message");
                        exit(EXIT_FAILURE);
                    }

                    memset(&servadr, 0, sizeof(servadr));
                    socklen_t recv_len = sizeof(servadr);

                    if (recvfrom(sock, recv_buffer, sizeof(msg_inscri), 0, (struct sockaddr*)&servadr, &recv_len) < 0) {
                        perror("Erreur lors de l'envoi du message");
                        exit(EXIT_FAILURE);
                    }
                    extract_entete(e, &codeReq, &id);
                    printf("codeReq : %d id : %d\n", codeReq, id);
                    
                }
                break;
            }
            case 2:
                printf("Option : Poster un billet\n");
                printf("Numéro du fil (0 pour un nouveau billet) : ");
                scanf("%d", &numFil);
                printf("Message : ");
                scanf("%s", message);
                //créer un nouveau fil si f = 0, poster un billet dans un fil existant sinon

                break;
            
            case 3: {
                printf("Option : Demander les n derniers billets\n");
                printf("Valeur de n : ");
                scanf("%d", &n);
                printf("Valeur de f : ");
                scanf("%d", &f);
                if (n != 0) {
                    if (f != 0) {
                        // Traitement lorsque n et f sont différents de 0
                    } else {
                        // Traitement lorsque n est différent de 0 et f est égal à 0
                    }
                } else {
                    if (f != 0) {
                        // Traitement lorsque n est égal à 0 et f est différent de 0
                    } else {
                        // Traitement lorsque n et f sont tous les deux égaux à 0
                    }
                }
                break;

            }
            case 4:
                printf("Option : S'abonner à un fil\n");
                printf("Numéro du fil auquel s'abonner : ");
                scanf("%d", &numFilAbonnement);
                ma = demande_abonnement(id, numFilAbonnement);
                
                memcpy(send_buffer, ma, sizeof(msg_fil));

                if (sendto(sock, send_buffer, sizeof(msg_fil), 0, (struct sockaddr*)&servadr, sizeof(servadr)) < 0) {
                    perror("Erreur lors de l'envoi du message");
                    exit(EXIT_FAILURE);
                }

                memset(&servadr, 0, sizeof(servadr));
                socklen_t recv_len = sizeof(servadr);

                if (recvfrom(sock, recv_buffer, sizeof(msg_fil), 0, (struct sockaddr*)&servadr, &recv_len) < 0) {
                    perror("Erreur lors de l'envoi du message");
                    exit(EXIT_FAILURE);
                }
                extract_entete(e, &codeReq, &id);
                printf("codeReq : %d id : %d\n", codeReq, id);

                break;
            case 5:
                printf("Option : Ajouter un fichier\n");
                printf("Numéro du fil : ");
                scanf("%d", &numFilFichier);
                printf("Nom du fichier : ");
                scanf("%s", nomFichier);

                // Ajouter le code pour ajouter un fichier
                break;
            
            case 6:
                printf("Option : Télécharger un fichier\n");
                printf("Port de réception des paquets : ");
                scanf("%d", &port);
                printf("Nom du fichier à télécharger : ");
                scanf("%s", nomFichierTelechargement);
                printf("Numéro du fil : ");
                scanf("%d", &numFilTelechargement);

                // Ajouter le code pour télécharger un fichier
                break;
            
            case 0:
                printf("Au revoir !\n");
                break;
            default:
                printf("Option invalide\n");
                break;
        }

        printf("\n");
    } while (choix != 0);



    
}