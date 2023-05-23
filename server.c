#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <net/if.h>
#include "user.h"
#include "msgcli.h"
#include "msgsrv.h"
#include "fil.h"

#define MAX_CLIENTS 2047 //le plus grand id représentable sur 11 bits = 2^11 - 1

lusers my_users = NULL;
fil * mes_fils = NULL;
uint16_t id_u = 199;
int sockfd;

pthread_mutex_t verrou = PTHREAD_MUTEX_INITIALIZER;
int idFichier = 0;

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
    billet * bp;
    int fin;
    int debut = 0;
    while (1) {
        bp = mon_fil->billets;
        fin = bp->numero;
        sleep(10);
        for(int i = fin;  i > debut; i--){
            strcpy(notif->pseudo, bp->pseudo);
            strcpy(notif->data, bp->message);
            memcpy(buffer, notif, sizeof(notif_srv));
            sendto(sock, buffer, sizeof(notif_srv), 0, (struct sockaddr*)&grsock, sizeof(grsock));
            bp = bp->suivant;
        }
        debut = fin;
    }
}

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
                , billets->pseudo
                , taille_billet
                , billets->message);

        char send_buffer[sizeof(msg_dernier_billets)];
        memcpy(send_buffer, msg, sizeof(msg_dernier_billets));

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

void is_ready_to_receve(struct sockaddr_in6 adrcli){
    int ok = 1;
    if (sendto(sockfd, &ok, sizeof(ok), 0,
            (struct sockaddr *)&adrcli, sizeof(adrcli)) < 0){
        perror("sendTo() => dernier_n_billets ");
        exit(EXIT_FAILURE);
    }
}

void ajout_fichier_aux(int id, int port
    , struct sockaddr_in6 adrcli, int f, char *nom){

    int sock_fichier = socket(PF_INET6, SOCK_DGRAM, 0);

    if(sock_fichier < 0) {
        perror("socket() => ajout_fichier_aux ");
        exit(EXIT_FAILURE);
    }    

    int reuse = 1;
    setsockopt(sock_fichier, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    struct sockaddr_in6 servadrfichier;

    memset(&servadrfichier, 0, sizeof(struct sockaddr_in6));
    servadrfichier.sin6_family = AF_INET6;
    servadrfichier.sin6_port = port;
    servadrfichier.sin6_addr = in6addr_any;

    if (bind(sock_fichier, (struct sockaddr*)&servadrfichier, sizeof(struct sockaddr_in6)) < 0) {
        perror("Erreur lors de la liaison de la socket");
        exit(EXIT_FAILURE);
    }

    is_ready_to_receve(adrcli);

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

        printf("Reception du paquet numero %d, de taille %zu octets de l'utilasteur %d\n",
                            ntohs(msg->numbloc), strlen(msg->data), id);

        ajout_bloc_fichier(ntohs(msg->numbloc), msg->data, fich);            

        if (strlen(msg->data) < 512) break;
    }

    close(sock_fichier);
}

msg_fil *init_reponse_client(const char *buffer, uint16_t id,
                                     struct sockaddr_in6 adrcli, int mask){
    msg_fil *msg = malloc(sizeof(msg_fil));
    memcpy(msg, buffer, sizeof(msg_fil));
    memcpy(msg->data, buffer+sizeof(msg_fil), msg->datalen);
    *(msg->data + msg->datalen) = '\0';

    fil *fils;


    if(!(fils=get_fil(mes_fils, ntohs(msg->numfil))) 
        || !est_inscrit(my_users, id)
        || (mask ^ exsist_fichier(fils, msg->data))){
        envoyer_erreur(adrcli);

        return NULL;
    }
    return msg;
}

void init_reponse_serveur(uint16_t entete, int id, int arg1, int arg2
                                , struct sockaddr_in6 adrcli){
    msg_srv *resp = compose_msg_srv(entete
                        , arg1, arg2);

    char send_buffer[sizeof(msg_srv)];
    memcpy(send_buffer, resp, sizeof(send_buffer));

    if (sendto(sockfd, send_buffer, sizeof(msg_srv), 0,
            (struct sockaddr *)&adrcli, sizeof(adrcli)) < 0){
        perror("sendTo() => dernier_n_billets ");
        exit(EXIT_FAILURE);
    }
}

void ajout_fichier(const char *buffer, uint16_t id,
                                     struct sockaddr_in6 adrcli){
    msg_fil *msg = init_reponse_client(buffer, id, adrcli, 0);

    if (!msg) return;

    //envoie du port au client
    pthread_mutex_lock(&verrou);
    int port = htons(get_allocated_port(id));
    pthread_mutex_unlock(&verrou);

    init_reponse_serveur(compose_entete(5, id), id, ntohs(msg->numfil)
        , ntohs(port), adrcli);

    ajout_fichier_aux(id, port, adrcli, ntohs(msg->numfil), msg->data);

}

void wait_until_ready(uint16_t id){
    uint16_t *my_id = malloc(sizeof(uint16_t));
    if (recv(sockfd, my_id, sizeof(uint16_t), 0) < 0){
        perror("recv() ");
        exit(EXIT_FAILURE);
    }
    if (*my_id != id) wait_until_ready(id);
    else return;
}

void telecharger_fichier_aux(uint16_t entete, uint16_t id, int port, int f, 
    char *nom_fichier){

    //on recupere le fichier a envoyer
    fichier *fich = get_fichier(get_fil(mes_fils, f), nom_fichier);

    struct sockaddr_in6 adrcli_fichier;
    memset(&adrcli_fichier, 0, sizeof(adrcli_fichier));
    adrcli_fichier.sin6_family = AF_INET6;
    adrcli_fichier.sin6_port = port;
    inet_pton(PF_INET6, "::1", &(adrcli_fichier.sin6_addr));

    data_fichier *data = fich->data;

    int numBloc = 1;

    wait_until_ready(id);

    while (data){

            int len_data = strlen(data->data);

            msg_fichier *msg = compose_msg_fichier(entete, htons(numBloc), data->data);

            int len = sizeof(msg_fichier);
            char send_buffer[len];
            memcpy(send_buffer, msg, len);

            if (sendto(sockfd, send_buffer, len, 0,
                        (struct sockaddr*)&adrcli_fichier, sizeof(adrcli_fichier)) < 0){
                perror("sendto() => telecharger_fichier_aux ");
                exit(EXIT_FAILURE);
            }

            printf("Envoi du paquet numero %d, de taille %d a l'utilisateur %d\n", 
                                numBloc, len_data, id);

            data = data->suivant;

            if (!data && len_data >= 512)
                send_empty_buffer(adrcli_fichier, 6, id, sockfd);

            numBloc++;

    }

}

void telecharger_fichier(const char *buffer, uint16_t id,
                                     struct sockaddr_in6 adrcli){
    msg_fil *msg = init_reponse_client(buffer, id, adrcli, 1);

    if (!msg) return;

    int port = msg->nb;
    uint16_t entete = compose_entete(6, id);

    init_reponse_serveur(entete, id, ntohs(msg->numfil), ntohs(port),
                                adrcli);

    telecharger_fichier_aux(entete, id, port, ntohs(msg->numfil), msg->data);
}   

msg_srv * erreur(){
    uint16_t entete = compose_entete(31, 0);
    msg_srv * ms = compose_msg_srv(entete, 0, 0);
    return ms; 
}

void recup_pseudo(char* pseudo) {
    int length = strlen(pseudo);
    int hashIndex = 0;
    while (hashIndex < length && pseudo[hashIndex] != '#') {
        hashIndex++;
    }
    if(hashIndex < length){
        pseudo[hashIndex] = '\0';
    }
}

msg_srv * inscription(const char * buffer){
    msg_inscri * mi = malloc(sizeof(msg_inscri));
    memcpy(mi, buffer, sizeof(msg_inscri));
    recup_pseudo(mi->pseudo);
    if(strlen(mi->pseudo) > 1){
        register_user(mi->pseudo);
        uint16_t entete = compose_entete(1, id_u);
        id_u++;
        msg_srv * ms = compose_msg_srv(entete, 0, 0);
        free(mi);
        return ms;
    }
    return erreur();
}

msg_srv * poster_billet(uint16_t id, const char * buffer){
    msg_fil * mf = malloc(sizeof(msg_fil));
    memcpy(mf, buffer, sizeof(msg_fil));
    char data_buf[mf->datalen+1];
    printf("%s\n", mf->data);
    strcpy(data_buf, mf->data);
    char * pseudo = get_user_pseudo(my_users, id);
    if(pseudo != NULL){
        int n = add_new_billet(&mes_fils, ntohs(mf->numfil), pseudo, data_buf);
        if(n < 0){
            free(mf);
            return erreur();
        }
        else{
            if(ntohs(mf->numfil) == 0){
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

    sockfd = socket(PF_INET6, SOCK_DGRAM, 0);
    if(sockfd < 0) {
        perror("socket()");
        exit(-1);
    }

    int ifindex = if_nametoindex ("eth0");
    if(setsockopt(sockfd, IPPROTO_IPV6, IPV6_MULTICAST_IF, &ifindex, sizeof(ifindex))){
        perror("erreur initialisation de l’interface locale");
        exit(-1);
    }

    struct sockaddr_in6 srvadr;
    memset(&srvadr, 0, sizeof(srvadr));
    srvadr.sin6_family = AF_INET6;
    srvadr.sin6_addr = in6addr_any;
    srvadr.sin6_port = htons(7777);

    if (bind(sockfd, (struct sockaddr *)&srvadr, sizeof(srvadr)) < 0) return -1;

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
        //Lecture de l'entête
        uint16_t e = *((uint16_t*)recv_buffer);
        uint8_t codeReq = 0;
        uint16_t id = 0;
        extract_entete(e, &codeReq, &id);

        printf("id : %d codeReq : %d\n", id, codeReq);

        char send_buffer[sizeof(msg_srv)];
        msg_srv * ms = NULL;
        msg_srv_fil * ms_a = NULL;

        switch(codeReq){
            case 1:
                ms = inscription(recv_buffer);
                memcpy(send_buffer, ms, sizeof(msg_srv));
                sendto(sockfd, send_buffer, sizeof(msg_srv), 0, (struct sockaddr*)&adrcli, lencli);
                break;
            case 2:
                ms = poster_billet(id, recv_buffer);
                if(ms == NULL){
                    ms = erreur();
                }
                memcpy(send_buffer, ms, sizeof(msg_srv));
                sendto(sockfd, send_buffer, sizeof(msg_srv), 0, (struct sockaddr*)&adrcli, lencli);
                break;
            case 3:
                dernier_n_billets(recv_buffer, id, adrcli);
                break;
            case 4:
                ms_a = abonnement(id, recv_buffer);
                if(ms_a != NULL){
                    memcpy(send_buffer, ms_a, sizeof(msg_srv_fil));
                    sendto(sockfd, send_buffer, sizeof(msg_srv_fil), 0, (struct sockaddr*)&adrcli, lencli);
                }
                else{ 
                    memcpy(send_buffer, erreur(), sizeof(msg_srv));
                    sendto(sockfd, send_buffer, sizeof(msg_srv), 0, (struct sockaddr*)&adrcli, lencli);
                }
                break;
            case 5:
                ajout_fichier(recv_buffer, id, adrcli);
                break;
            case 6:
                telecharger_fichier(recv_buffer, id, adrcli);
                break;
            default:
                break;
        }
    }
    close(sockfd);

    return 0;
}