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

#define BUF_SIZE 256
#define MAX_CLIENTS 2047 //le plus grand id représentable sur 11 bits = 2^11 - 1

lusers my_users = NULL;
fil * mes_fils = NULL;
uint16_t id_u = 0;
int sockfd;

void register_user(const char * pseudo){
    user * u = malloc(sizeof(user));
    u->id = id_u;
    strcpy(u->pseudo, pseudo);
    my_users = add_user(my_users, id_u, pseudo);
}

void dernier_n_billets(const char *buffer, uint16_t id,
                                     struct sockaddr_in6 adrcli){
    msg_fil *mf = malloc(sizeof(msg_fil));
    memcpy(mf, buffer, sizeof(msg_fil));
    if (!est_inscrit(lusers, id)){
        //envoi du msg d'erreur au client
        return;
    }
    msg_srv *reponse_srv = malloc(sizeof(msg_srv));
    reponse_srv->entete = mf->entete;
    
    if (mf->numfil > 0){
        int nb_msgs_fil = nb_msgs_fil(mes_fils, mf->numfil);
        if (nb_msgs_fil < 0){
            //msg erreur
            return;
        }
        reponse_srv->numfil = mf->numfil;
        reponse_srv->nb = (mf->nb > nb_msgs_fil || mf->nb == 0) 
                                    ? nb_msgs_fil : mf->nb ;
    }
    else{
        reponse_srv->numfil = nb_fils(mes_fils);
        reponse_srv->nb = nb_msgs_total_fil(mes_fils);
    }

    char send_buffer[sizeof(msg_srv)];
    memcpy(send_buffer, reponse_srv, sizeof(send_buffer));

    if (sendto(sockfd, send_buffer, sizeof(msg_srv),
            (struct sockaddr *)&adrcli, sizeof(adrcli)) < 0){
        perror("sendTo() => dernier_n_billets ");
        exit(EXIT_FAILURE);
    }

    free(send_buffer);
    free(buffer);
}

msg_srv * inscription(const char * buffer){
    msg_inscri * mi = malloc(sizeof(msg_inscri));
    memcpy(mi, buffer, sizeof(msg_inscri));
    register_user(mi->pseudo);
    uint16_t entete = compose_entete(1, id_u);
    id_u++;
    msg_srv * ms = compose_msg_srv(entete, 0, 0);
    return ms;
}

msg_srv * poster_billet(uint16_t id, const char * buffer){
    msg_fil * mf = malloc(sizeof(msg_fil));
    memcpy(mf, buffer, sizeof(msg_fil));
    //int n = add_new_billet(mes_fils, mf->numfil, id, mf->data);
}



int main(){

    my_users = malloc(sizeof(user));

    int sockfd = socket(PF_INET6, SOCK_DGRAM, 0);
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

    while (1) {
        struct sockaddr_in6 adrcli;
        socklen_t lencli = sizeof(adrcli);
        memset(&adrcli, 0, sizeof(adrcli));

        char recv_buffer[BUF_SIZE];
        memset(recv_buffer, 0, sizeof(recv_buffer));

        ssize_t recv_len = recvfrom(sockfd, recv_buffer, BUF_SIZE - 1, 0, (struct sockaddr*)&adrcli, &lencli);
        if (recv_len < 0) {
            perror("Erreur lors de la réception du message");
            continue;
        }

        //Récupération de l'adresse IP du client
        char ipcli[INET6_ADDRSTRLEN];
        inet_ntop(PF_INET6, &(adrcli.sin6_addr), ipcli, INET6_ADDRSTRLEN);

        //Lecture de l'entête
        uint16_t e = *((uint16_t*)recv_buffer);
        uint8_t codeReq = 0;
        uint16_t id = 0;
        extract_entete(e, &codeReq, &id);

        char send_buffer[sizeof(msg_srv)];
        msg_srv * ms = NULL;
        ssize_t send_len = 0; 

        switch(codeReq){
            case 0:
                ms = inscription(recv_buffer);
                memcpy(send_buffer, ms, sizeof(msg_srv));
                send_len = sendto(sockfd, send_buffer, sizeof(msg_srv), 0, (struct sockaddr*)&adrcli, lencli);
                break;
            default:
                break;
        }
    }

    close(sockfd);

    return 0;
}