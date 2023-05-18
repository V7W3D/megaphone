#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "user.h"
#include "msgcli.h"

#define BUF_SIZE 256
#define MAX_CLIENTS 256

lusers my_users = NULL;
int id_u = 0;
int sockfd;

void register_user(const char * pseudo){
    user * u = malloc(sizeof(user));
    u->id = id_u;
    strcpy(u->pseudo, pseudo);
    my_users = add_user(my_users, u);
}

void* handler(void* arg) {
    struct sockaddr_in6 addr = *(struct sockaddr_in6*)arg;
    char buffer[BUF_SIZE + 1];
    socklen_t len = sizeof(addr);

    while (1) {
        memset(buffer, 0, sizeof(buffer));

        // Réception du message du client
        ssize_t recv_len = recvfrom(sockfd, buffer, BUF_SIZE, 0, (struct sockaddr*)&addr, &len);
        if (recv_len < 0) {
            perror("Erreur lors de la réception du message");
            break;
        } else if (recv_len == 0) {
            // Le client s'est déconnecté
            printf("Le client s'est déconnecté.\n");
            break;
        }

        uint16_t e = *((uint16_t*)buffer);
        uint8_t codeReq = 0;
        uint16_t id = 0;
        extract_entete(e, &codeReq, &id);

        printf("code : %d id : %d \n", codeReq, id);
        
    }

    free(arg);  // Libération de la mémoire

    return NULL;
}

int main(){

    my_users = malloc(sizeof(struct list_users));

    sockfd = socket(PF_INET6, SOCK_DGRAM, 0);
    if(sockfd < 0) {
        perror("socket()");
        return -1;
    }

    struct sockaddr_in6 cliadr;
    memset(&cliadr, 0, sizeof(cliadr));
    cliadr.sin6_family = AF_INET6;
    cliadr.sin6_addr = in6addr_any;
    cliadr.sin6_port = htons(7777);

    if (bind(sockfd, (struct sockaddr *)&cliadr, sizeof(cliadr)) < 0) return -1;

    pthread_t clientThreads[MAX_CLIENTS];
    int nbclients = 0;

    while (1) {
        struct sockaddr_in6 clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        memset(&clientAddr, 0, sizeof(clientAddr));

        char buffer[BUF_SIZE];
        memset(buffer, 0, sizeof(buffer));

        // Réception du message du client
        ssize_t recv_len = recvfrom(sockfd, buffer, BUF_SIZE - 1, 0, (struct sockaddr*)&clientAddr, &clientLen);
        if (recv_len < 0) {
            perror("Erreur lors de la réception du message");
            continue;
        }

        /*
        
        uint16_t e = *((uint16_t*)buffer);
        uint8_t codeReq = 0;
        uint16_t id = 0;
        extract_entete(e, &codeReq, &id);

        printf("code : %d id : %d \n", codeReq, id);
        */

        // Vérifier si le nombre maximal de clients est atteint
        if (nbclients >= MAX_CLIENTS) {
            printf("Nombre maximal de clients atteint. La connexion du client a été refusée.\n");
            continue;
        }

        // Copier l'adresse du client dans une nouvelle structure sockaddr_in6 pour le thread
        struct sockaddr_in6* clientAddrCopy = malloc(sizeof(struct sockaddr_in6));
        memcpy(clientAddrCopy, &clientAddr, sizeof(struct sockaddr_in6));

        // Créer un thread pour gérer le client
        pthread_t thread;
        if (pthread_create(&thread, NULL, handler, (void*)clientAddrCopy) != 0) {
            perror("Erreur lors de la création du thread client");
            free(clientAddrCopy);
            continue;
        }

        // Ajouter le thread client au tableau
        clientThreads[nbclients] = thread;
        nbclients++;
    }

    // Attendre la fin de tous les threads clients
    for (int i = 0; i < nbclients; i++) {
        pthread_join(clientThreads[i], NULL);
    }

    // Fermer la socket principale
    close(sockfd);

    return 0;
}