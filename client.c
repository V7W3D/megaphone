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

int sock;
struct sockaddr_in6 servadr;

msg_fil* compose_msg_dernier_n_billets(uint16_t id, uint16_t f, uint16_t nb){
    return compose_msg_fil(NULL, 3, id, f, nb);
}

int erreur(uint16_t entete){
    uint8_t codeReq;
    uint16_t id;
    extract_entete(entete, &codeReq, &id);
    
    if (codeReq==31){
        fprintf(stderr, "\nErreur du serveur\n");
        return 1;//oui il y'a une erreur
    }
    return 0;
}

void recv_dernier_n_billets(int nb){
    while (nb){
        char recv_buffer[sizeof(msg_dernier_billets)];
        if (recv(sock, recv_buffer, sizeof(msg_dernier_billets)) < 0){
            perror("recv() => recv_dernier_n_billets ");
            exit(EXIT_FAILURE);
        }

        msg_dernier_billets *msg = malloc(sizeof(msg_dernier_billets));
        memcpy(msg, recv_buffer, sizeof(msg_dernier_billets));
        printf("\n------------------------------------------\n");
        printf("Numero du fil : %d\n", msg->numfil);
        printf("Origine : %s\n", msg->origin);
        printf("Pseudo : %s\n", msg->pseudo);
        printf("Taille du message : %d\n", msg->datalen);
        printf("message : \n%s\n", msg->data);
        printf("\n------------------------------------------\n");
        free(msg);
        nb--;
    }
}

void dernier_n_billets(uint16_t id, uint16_t f, uint16_t nb){
    msg_fil *msg = compose_msg_dernier_n_billets(id, f, nb);

    char send_buffer[sizeof(msg_fil)];
    memcpy(send_buffer, msg, sizeof(msg_fil));

    //envoyer la requete au serveur
    if (sendto(sock, send_buffer, sizeof(msg_fil), 0,
                (struct sockaddr*)&servadr, sizeof(servadr)) < 0){
        perror("sendto()");
        exit(EXIT_FAILURE);
    }

    char recv_buffer[sizeof(msg_srv)];
    memset(recv_buffer, 0, sizeof(msg_srv));

    if (recv(sock, recv_buffer, sizeof(msg_srv), 0) < 0){
        perror("sendto()");
        exit(EXIT_FAILURE);
    }

    msg_srv *rep_srv = malloc(sizeof(msg_srv));
    memcpy(rep_srv, recv_buffer, sizeof(msg_srv));

    if (erreur(rep_srv->entete)) return;

    uint16_t nb = rep_srv->nb;

    recv_dernier_n_billets(nb);
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

int main(){
    sock = socket(PF_INET6, SOCK_DGRAM, 0);
    if(sock < 0) {
        perror("socket()");
        return -1;
    }

    memset(&servadr, 0, sizeof(servadr));
    servadr.sin6_family = AF_INET6;
    servadr.sin6_port = htons(7777);
    inet_pton(AF_INET6, "::1", &servadr.sin6_addr);
    
    msg_inscri * m = inscription("ahmed");

    char send_buffer[sizeof(msg_inscri)];
    memcpy(send_buffer, m, sizeof(msg_inscri));

    if (sendto(sock, send_buffer, sizeof(msg_inscri), 0, (struct sockaddr*)&servadr, sizeof(servadr)) < 0) {
        perror("Erreur lors de l'envoi du message");
        exit(EXIT_FAILURE);
    }

    char recv_buffer[sizeof(msg_srv)];
    memset(&servadr, 0, sizeof(servadr));
    socklen_t recv_len = sizeof(servadr);

    if (recvfrom(sock, recv_buffer, sizeof(msg_inscri), 0, (struct sockaddr*)&servadr, &recv_len) < 0) {
        perror("Erreur lors de l'envoi du message");
        exit(EXIT_FAILURE);
    }

    uint16_t e = *((uint16_t*)recv_buffer);
    uint8_t codeReq = 0;
    uint16_t id = 0;
    extract_entete(e, &codeReq, &id);

    printf("codeReq : %d id : %d\n", codeReq, id);
}