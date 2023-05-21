#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
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
pthread_mutex_t verrou = PTHREAD_MUTEX_INITIALIZER;
int idFichier = 0;

void register_user(const char * pseudo){
    user * u = malloc(sizeof(user));
    u->id = id_u;
    strcpy(u->pseudo, pseudo);
    my_users = add_user(my_users, id_u, pseudo);
}

void envoyer_erreur(struct sockaddr_in6 adrcli){
    char send_buffer[sizeof(msg_srv)];
    memcpy(send_buffer, msg_erreur(), sizeof(msg_srv));
    if (sendto(sockfd, send_buffer, sizeof(msg_srv), 0,
            (struct sockaddr *)&adrcli, sizeof(adrcli)) < 0){
        perror("sendTo() => dernier_n_billets ");
        exit(EXIT_FAILURE);
    }
}

void envoyer_billets(int num_fil, int nb, struct sockaddr_in6 adrcli){
    msg_dernier_billets *msg;
    fil *fil;
    if (num_fil>0) fil = get_fil(mes_fils, num_fil);
    else fil = mes_fils;
    billet *billets;
    if (fil) billets = fil->billets;

    while (nb && billets && fil){

        int taille_billet = strlen(billets->message);

        msg = compose_msg_dernier_billet(
                num_fil 
                , get_name(my_users, fil->id_proprietaire)
                , get_name(my_users, billets->id_proprietaire)
                , taille_billet
                , billets->message);

        char send_buffer[sizeof(msg_dernier_billets)];
        memcpy(send_buffer, msg, sizeof(msg_dernier_billets));

        printf("%d,%d\n",fil->id_proprietaire,billets->id_proprietaire);

        if (sendto(sockfd, send_buffer, sizeof(msg_dernier_billets), 0,
                    (struct sockaddr *)&adrcli, sizeof(adrcli)) < 0){
            perror("sendTo() => envoyer_billets ");
            exit(EXIT_FAILURE);
        }

        billets = billets->suivant;
        if (!billets && num_fil == 0){
            fil = fil->suivant;
            if (!fil) return;
            billets = fil->billets;
        }
        free(msg);
        nb--;   
    }
}

int dernier_n_billets(const char *buffer, uint16_t id,
                                     struct sockaddr_in6 adrcli){
    msg_fil *mf = malloc(sizeof(msg_fil));
    memcpy(mf, buffer, sizeof(msg_fil));
    if (!est_inscrit(my_users, id)){
        envoyer_erreur(adrcli);
        return -1;
    }
    msg_srv *reponse_srv = malloc(sizeof(msg_srv));
    reponse_srv->entete = mf->entete;
    
    if (mf->numfil > 0){
        int nb_msgs = nb_msgs_fil(mes_fils, mf->numfil);
        if (nb_msgs < 0){
            envoyer_erreur(adrcli);
            return -1;
        }
        reponse_srv->numfil = mf->numfil;
        reponse_srv->nb = (mf->nb > nb_msgs || mf->nb == 0) 
                                    ? nb_msgs : mf->nb ;
    }
    else{
        reponse_srv->numfil = nb_fils(mes_fils);
        reponse_srv->nb = nb_msgs_total_fil(mes_fils);
    }

    char send_buffer[sizeof(msg_srv)];
    memcpy(send_buffer, reponse_srv, sizeof(send_buffer));

    if (sendto(sockfd, send_buffer, sizeof(msg_srv), 0,
            (struct sockaddr *)&adrcli, sizeof(adrcli)) < 0){
        perror("sendTo() => dernier_n_billets ");
        exit(EXIT_FAILURE);
    }

    envoyer_billets(mf->numfil, reponse_srv->nb, adrcli);

    return 0;   
}

void ajout_fichier_aux(int id, int port
    , struct sockaddr_in6 adrcli, int f, char *nom){

    int sock_fichier = socket(PF_INET6, SOCK_DGRAM, 0);

    if(sock_fichier < 0) {
        perror("socket() => ajout_fichier_aux ");
        exit(EXIT_FAILURE);
    }    

    struct sockaddr_in6 servadrfichier;

    memset(&servadrfichier, 0, sizeof(struct sockaddr_in6));
    servadrfichier.sin6_family = AF_INET6;
    servadrfichier.sin6_port = port;
    servadrfichier.sin6_addr = in6addr_any;

    if (bind(sock_fichier, (struct sockaddr*)&servadrfichier, sizeof(struct sockaddr_in6)) < 0) {
        perror("Erreur lors de la liaison de la socket");
        exit(EXIT_FAILURE);
    }

    fichier *fich = creer_fichier(mes_fils, f, idFichier, id, nom);
    idFichier++;

    while (1){

        char recv_buffer[sizeof(msg_fichier)];
        memset(recv_buffer, 0, sizeof(msg_fichier));

        if (recv(sock_fichier, recv_buffer, sizeof(msg_fichier), 0) < 0){
            perror("recv() => ajout_fichier_aux ");
            exit(EXIT_FAILURE);
        }

        msg_fichier *msg = malloc(sizeof(msg_fichier));
        memcpy(msg, recv_buffer, sizeof(msg_fichier));

        ajout_bloc_fichier(msg->numbloc, msg->data, fich);            

        if (strlen(msg->data) < 512) break;
    }

    close(sock_fichier);
}

void ajout_fichier(const char *buffer, uint16_t id,
                                     struct sockaddr_in6 adrcli){
    msg_fil *msg = malloc(sizeof(msg_fil));
    memcpy(msg, buffer, sizeof(msg_fil));
    msg->data = malloc(sizeof(msg->datalen));
    strncpy(msg->data, buffer+sizeof(msg_fil), msg->datalen);
    *(msg->data + msg->datalen) = '\0';

    printf("%s\n", msg->data);
    //envoie du port au client
    pthread_mutex_lock(&verrou);
    int port = htons(get_allocated_port(id));
    pthread_mutex_unlock(&verrou);

    msg_srv *resp = compose_msg_srv(compose_entete(5, id)
                        , ntohs(msg->numfil), ntohs(port));

    char send_buffer[sizeof(msg_srv)];
    memcpy(send_buffer, resp, sizeof(send_buffer));

    fil *fils;

    if(!(fils=get_fil(mes_fils, ntohs(msg->numfil))) 
        || !est_inscrit(my_users, id)
        || exsist_fichier(fils, msg->data)){
        envoyer_erreur(adrcli);
        return;
    }

    if (sendto(sockfd, send_buffer, sizeof(msg_srv), 0,
            (struct sockaddr *)&adrcli, sizeof(adrcli)) < 0){
        perror("sendTo() => dernier_n_billets ");
        exit(EXIT_FAILURE);
    }

    ajout_fichier_aux(id, port, adrcli, ntohs(msg->numfil), msg->data);

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
    return NULL;
}



int main(){

    my_users = malloc(sizeof(user));

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

    
    my_users = add_user(my_users, 1, "younes");
    mes_fils = add_new_fil(mes_fils, 1, "::1", 0);

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
            case 3:
                dernier_n_billets(recv_buffer , id, adrcli);
                break;
            default:
            case 5:
                ajout_fichier(recv_buffer, id, adrcli);
                break;
        }
    }

    close(sockfd);

    return 0;
}