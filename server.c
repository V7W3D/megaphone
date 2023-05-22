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
#include <net/if.h>


#define BUF_SIZE 4096
#define MAX_CLIENTS 2047 //le plus grand id représentable sur 11 bits = 2^11 - 1

lusers my_users = NULL;
fil * mes_fils = NULL;
uint16_t id_u = 199;

void * envoyer_notification(void* arg) {
    thread_arg  a = *(thread_arg *) arg;
    
    int sock = socket(AF_INET6, SOCK_DGRAM, 0);
    struct sockaddr_in6 grsock;
    memset(&grsock, 0, sizeof(grsock));
    grsock.sin6_family = AF_INET6;
    inet_pton(AF_INET6, a.adr, &grsock.sin6_addr);
    grsock.sin6_port = htons(a.port);
    notif_srv * notif = malloc(sizeof(notif_srv));
    char buffer[sizeof(notif_srv)];
    uint16_t entete = compose_entete(4, 0);
    notif->entete = entete;
    notif->numfil = a.numfil;
    fil * mon_fil = get_fil(mes_fils, a.numfil);
    billet * mes_billets = mon_fil->billets;
    billet * bp = NULL;
    int fin = mes_billets->numero;
    int debut = 0;
    while (1) {
        if(debut != 0) fin = debut;
        sleep(10);
        bp = mes_billets;
        debut = bp->numero;
        for(int i = fin;  i >= debut; i--){
            printf("hello");
            strcpy(notif->pseudo, bp->pseudo);
            strcpy(notif->data, bp->message);
            memcpy(buffer, notif, sizeof(notif_srv));
            sendto(sock, buffer, sizeof(notif_srv), 0, (struct sockaddr*)&grsock, sizeof(grsock));
            bp = bp->suivant;
        }
    }
}

void register_user(const char * pseudo){
    user * u = malloc(sizeof(user));
    u->id = id_u;
    strcpy(u->pseudo, pseudo);
    my_users = add_user(my_users, id_u, pseudo);
}

msg_srv * erreur(){
    uint16_t entete = compose_entete(31, 0);
    msg_srv * ms = compose_msg_srv(entete, 0, 0);
    return ms; 
}

msg_srv * inscription(const char * buffer){
    msg_inscri * mi = malloc(sizeof(msg_inscri));
    memcpy(mi, buffer, sizeof(msg_inscri));
    register_user(mi->pseudo);
    uint16_t entete = compose_entete(1, id_u);
    id_u++;
    msg_srv * ms = compose_msg_srv(entete, 0, 0);
    free(mi);
    return ms;
}

msg_srv * poster_billet(uint16_t id, const char * buffer){
    msg_fil * mf = malloc(sizeof(msg_fil));
    memcpy(mf, buffer, sizeof(msg_fil));
    char data_buf[mf->datalen+1];
    printf("%s\n", mf->data);
    strcpy(data_buf, mf->data);
    char * pseudo = get_user_pseudo(my_users, id);
    if(pseudo != NULL){
        int n = add_new_billet(&mes_fils, mf->numfil, pseudo, data_buf);
        if(n < 0){
            free(mf);
            return erreur();
        }
        else{
            if(mf->numfil == 0){
                pthread_t thread;
                thread_arg * a = malloc(sizeof(thread_arg));
                a->port = mes_fils->port;
                a->numfil = n;
                strcpy(a->adr, mes_fils->adresse);
                pthread_create(&thread, NULL, envoyer_notification, a);
            }
            uint16_t entete = mf->entete;
            msg_srv * ms = compose_msg_srv(entete, n, 0);
            free(mf);
            return ms;
        }
    }
    free(mf);
    return NULL;
}

msg_srv_fil * abonnement(uint16_t id, const char * buffer){
    msg_fil * mf = malloc(sizeof(msg_fil));
    memcpy(mf, buffer, sizeof(msg_fil));
    fil * fil = get_fil(mes_fils, ntohs(mf->numfil));
    if(fil != NULL){
        char * a =  add_new_abonne(mes_fils, fil->numero, id);
        if(a == NULL) return NULL;
        uint16_t entete = mf->entete;
        msg_srv_fil * ms = compose_msg_srv_fil(entete, fil->numero, fil->port, a);
        free(mf);
        return ms;
    }
    free(mf);
    return NULL;
}

int main(){

    my_users = malloc(sizeof(user));

    int sockfd = socket(PF_INET6, SOCK_DGRAM, 0);
    if(sockfd < 0) {
        perror("socket()");
        exit(-1);
    }

    int ifindex = if_nametoindex ("eth0");
    if(setsockopt(sockfd, IPPROTO_IPV6, IPV6_MULTICAST_IF, &ifindex, sizeof(ifindex))){
        perror("erreur initialisation de l’interface locale");
        exit(-1);
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

        printf("id : %d codeReq : %d\n", id, codeReq);

        char send_buffer[sizeof(msg_srv)];
        msg_srv * ms = NULL;
        msg_srv_fil * ms_a = NULL;
        ssize_t send_len = 0; 

        switch(codeReq){
            case 1:
                ms = inscription(recv_buffer);
                memcpy(send_buffer, ms, sizeof(msg_srv));
                send_len = sendto(sockfd, send_buffer, sizeof(msg_srv), 0, (struct sockaddr*)&adrcli, lencli);
                break;
            case 2:
                ms = poster_billet(id, recv_buffer);
                if(ms == NULL) ms = erreur();
                memcpy(send_buffer, ms, sizeof(msg_srv));
                send_len = sendto(sockfd, send_buffer, sizeof(msg_srv), 0, (struct sockaddr*)&adrcli, lencli);
                break;
            case 4:
                ms_a = abonnement(id, recv_buffer);
                if(ms_a != NULL){
                    memcpy(send_buffer, ms_a, sizeof(msg_srv_fil));
                    send_len = sendto(sockfd, send_buffer, sizeof(msg_srv_fil), 0, (struct sockaddr*)&adrcli, lencli);
                }
                else{ 
                    memcpy(send_buffer, erreur(), sizeof(msg_srv));
                    send_len = sendto(sockfd, send_buffer, sizeof(msg_srv), 0, (struct sockaddr*)&adrcli, lencli);
                }
                break;
            default:
                break;
        }
    }

    close(sockfd);

    return 0;
}