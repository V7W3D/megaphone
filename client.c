#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include "user.h"
#include "msgcli.h"
#include "msgsrv.h"
#include "fil.h"

#define SIZE_BLOC 512

int sock;
uint16_t id;
struct sockaddr_in6 servadr;

msg_fil* compose_msg_dernier_n_billets(uint16_t f, uint16_t nb){
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

        if (recv(sock, recv_buffer, sizeof(msg_dernier_billets), 0) < 0){
            perror("recv() => recv_dernier_n_billets ");
            exit(EXIT_FAILURE);
        }

        msg_dernier_billets *msg = malloc(sizeof(msg_dernier_billets));
        memcpy(msg, recv_buffer, sizeof(msg_dernier_billets));
        *(msg->data + msg->datalen) = '\0';

        printf("\n------------------------------------------\n");
        printf("Numero du fil : %d\n", msg->numfil);
        printf("Origine : %s\n", msg->origin);
        printf("Pseudo : %s\n", msg->pseudo);
        printf("Taille du message : %d\n", msg->datalen);
        printf("message : %s\n", msg->data);
        printf("\n------------------------------------------\n");
        free(msg);
        nb--;
    }
}

void dernier_n_billets(uint16_t f, uint16_t nb){
    msg_fil *msg = compose_msg_dernier_n_billets(f, nb);

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
        perror("recv() => dernier_n_billets ");
        exit(EXIT_FAILURE);
    }

    msg_srv *rep_srv = malloc(sizeof(msg_srv));
    memcpy(rep_srv, recv_buffer, sizeof(msg_srv));

    if (erreur(rep_srv->entete)){
        return;
    }

    uint16_t nb_real_billets = rep_srv->nb;

    printf("Nombre de billets : %d\n", nb_real_billets);

    recv_dernier_n_billets(nb_real_billets);
}

void send_empty_buffer(struct sockaddr_in6 servadrfichier){
    int len = sizeof(msg_fichier);
    msg_fichier *msg = compose_msg_fichier(compose_entete(5, id), htons(-1), "");
    char send_buffer[len];
    memcpy(send_buffer, msg, len);

        //envoyer la requete au serveur
    if (sendto(sock, send_buffer, len, 0,
                    (struct sockaddr*)&servadrfichier, sizeof(servadrfichier)) < 0){
        perror("sendto()");
        exit(EXIT_FAILURE);
    }

}

void ajout_fichier_aux(int port, FILE *fichier){

    struct sockaddr_in6 servadrfichier;

    memset(&servadrfichier, 0, sizeof(servadrfichier));
    servadrfichier.sin6_family = AF_INET6;
    servadrfichier.sin6_port = port;
    inet_pton(AF_INET6, "::1", &servadrfichier.sin6_addr);

    //lecture du fichier
    char buffer[SIZE_BLOC+1];
    int numBloc = 1;
    long nb_read = 0;
    while (!feof(fichier) && nb_read <= LEN_FILE){
        nb_read += fread(buffer, sizeof(unsigned char), SIZE_BLOC, fichier);
        msg_fichier *msg = compose_msg_fichier(compose_entete(5, id), htons(numBloc), buffer);
        int len = sizeof(msg_fichier);

        char send_buffer[len];
        memcpy(send_buffer, msg, len);

        //envoyer la requete au serveur
        if (sendto(sock, send_buffer, len, 0,
                    (struct sockaddr*)&servadrfichier, sizeof(servadrfichier)) < 0){
            perror("sendto()");
            exit(EXIT_FAILURE);
        }

        if (feof(fichier) && strlen(buffer)>=SIZE_BLOC) 
            send_empty_buffer(servadrfichier);

        numBloc++;
    }

}

void ajout_fichier(uint16_t f, const char *nom, const char *path){
    msg_fil * msg = compose_msg_fil(nom, 5, id, f, 0);

    FILE *fichier = fopen(path, "rb");
    if (!fichier){
        perror("fopen() => ajout_fichier ");
        exit(EXIT_FAILURE);
    }

    int len = sizeof(msg_fil) + strlen(nom);

    char send_buffer[len];
    memcpy(send_buffer, msg, len);
    memcpy(send_buffer+sizeof(msg_fil), nom, strlen(nom));

    //envoyer la requete au serveur
    if (sendto(sock, send_buffer, len, 0,
                (struct sockaddr*)&servadr, sizeof(servadr)) < 0){
        perror("sendto()");
        exit(EXIT_FAILURE);
    }

    char recv_buffer[sizeof(msg_srv)];
    memset(recv_buffer, 0, sizeof(msg_srv));

    if (recv(sock, recv_buffer, sizeof(msg_srv), 0) < 0){
        perror("recv() => dernier_n_billets ");
        exit(EXIT_FAILURE);
    }

    msg_srv *rep_srv = malloc(sizeof(msg_srv));
    memcpy(rep_srv, recv_buffer, sizeof(msg_srv));

    if (erreur(rep_srv->entete)){
        return;
    }

    ajout_fichier_aux(rep_srv->nb, fichier);
    
    fclose(fichier);
}

msg_inscri * inscription(const char * pseudo){
    uint16_t e = compose_entete(0,0);
    msg_inscri * m = compose_msg_inscri(e, pseudo);
    return m;
}

msg_fil * poster_billet(uint16_t f, const char * message){
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
    extract_entete(e, &codeReq, &id);

    printf("codeReq : %d id : %d\n", codeReq, id);

    ajout_fichier(0, "younes", "./server.c");
}