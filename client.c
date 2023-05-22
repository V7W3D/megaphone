#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <net/if.h>
#include <sys/time.h>
#include <errno.h>
#include "user.h"
#include "msgcli.h"
#include "msgsrv.h"
#include "fil.h"

#define TIMEOUT 2

#define EMPTY(f) *f = '\0'

#define SIZE_BLOC 512

int sock = 0;
uint16_t id;
struct sockaddr_in6 servadr;
mes_notification notifs = NULL;

void* recevoir_notification(void* arg){
    thread_arg a = *(thread_arg *) arg;

    int sockfd = socket(PF_INET6, SOCK_DGRAM, 0);

    struct sockaddr_in6 grsock;
    memset(&grsock, 0, sizeof(grsock));
    grsock.sin6_family = AF_INET6;
    grsock.sin6_addr = in6addr_any;
    grsock.sin6_port = htons(a.port);

    if(bind(sockfd, (struct sockaddr*)&grsock, sizeof(grsock))) {
        return NULL;
    }

    struct ipv6_mreq group;
    inet_pton (AF_INET6, a.adr, &group.ipv6mr_multiaddr.s6_addr);
    int ifindex = if_nametoindex ("eth0");
    group.ipv6mr_interface = ifindex;

    if(setsockopt(sockfd, IPPROTO_IPV6, IPV6_JOIN_GROUP, &group, sizeof(group))<0){
        return NULL;
    }

    notif_srv * notif = malloc(sizeof(notif_srv));
    char buffer[sizeof(notif_srv)];
    while(1){
        if (read(sockfd, buffer, sizeof(notif_srv)) < 0) return NULL;
        memcpy(notif, buffer, sizeof(notif_srv));
        notifs = add_notif(notifs, notif->pseudo, notif->data, notif->numfil);
    }
}

msg_inscri * inscription(const char * pseudo){
    uint16_t e = compose_entete(1,0);
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

void sabonner(const char * buffer){
    msg_srv_fil * mf = malloc(sizeof(msg_srv_fil));
    memcpy(mf, buffer, sizeof(msg_srv_fil));
    thread_arg * a = malloc(sizeof(thread_arg));
    strcpy(a->adr, mf->adr);
    a->port = ntohs(mf->nb);
    printf("%s\n", mf->adr);
    pthread_t thread;
    pthread_create(&thread, NULL, recevoir_notification, a);
}


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

    printf("Envoie de la requete au serveur...\n");
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

    printf("OK du serveur\n");

    uint16_t nb_real_billets = rep_srv->nb;

    printf("Nombre de billets : %d\n", nb_real_billets);

    recv_dernier_n_billets(nb_real_billets);
}

void wait_until_ready(){
    int *resp = malloc(sizeof(int));
    if (recv(sock, resp, sizeof(int), 0) < 0){
        perror("recv() ");
        exit(EXIT_FAILURE);
    }
    if (*resp == 1) return;
    else wait_until_ready();
}

void ajout_fichier_aux(int port, FILE *fichier){

    struct sockaddr_in6 servadrfichier;

    memset(&servadrfichier, 0, sizeof(servadrfichier));
    servadrfichier.sin6_family = AF_INET6;
    servadrfichier.sin6_port = port;
    inet_pton(AF_INET6, "::1", &servadrfichier.sin6_addr);

    //lecture du fichier
    char buffer[SIZE_BLOC];
    int numBloc = 1;
    long nb_read_total = 0;
    long nb_read;

    wait_until_ready();

    while (!feof(fichier) && nb_read_total <= LEN_FILE){
        nb_read = fread(buffer, sizeof(char), SIZE_BLOC, fichier);
        SET_END_AT(buffer, nb_read);
        msg_fichier *msg = compose_msg_fichier(compose_entete(5, id), htons(numBloc), buffer);
        int len = sizeof(msg_fichier);

        char send_buffer[len];
        memcpy(send_buffer, msg, len);

        printf("Envoie du paquet numero %d, de taille %zu octets au serveur\n",
                                numBloc, strlen(buffer));

        //envoyer la requete au serveur
        if (sendto(sock, send_buffer, len, 0,
                    (struct sockaddr*)&servadrfichier, sizeof(servadrfichier)) < 0){
            perror("sendto()");
            exit(EXIT_FAILURE);
        }

        if (feof(fichier) && strlen(buffer)>=SIZE_BLOC) 
            send_empty_buffer(servadrfichier, 5, id, sock);

        numBloc++;
        nb_read_total+=nb_read;
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

    printf("Envoi de la requete au serveur...\n");

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

    printf("OK du serveur\n");

    ajout_fichier_aux(rep_srv->nb, fichier);
    
    fclose(fichier);
}

void is_ready_to_receve(){
    if (sendto(sock, &id, sizeof(id), 0
        ,(struct sockaddr*)&servadr, sizeof(servadr)) < 0){
        perror("sendto() ");
        exit(EXIT_FAILURE);
    }
}

void telecharger_fichier_aux(int port, const char *nom, const char *path){
    int soket_fichier = socket(PF_INET6, SOCK_DGRAM, 0);

    struct sockaddr_in6 client_adr;

    memset(&client_adr, 0, sizeof(struct sockaddr_in6));
    client_adr.sin6_family = AF_INET6;
    client_adr.sin6_port = port;
    client_adr.sin6_addr = in6addr_any;

    int reuse = 1;
    if (setsockopt(soket_fichier, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("Erreur lors de la configuration de l'option SO_REUSEADDR");
        exit(EXIT_FAILURE);
    }

    if (bind(soket_fichier, (struct sockaddr *)&client_adr, sizeof(client_adr)) < 0){
        perror("bind() ");
        exit(EXIT_FAILURE);
    }

    is_ready_to_receve();

    char *full_path = malloc(sizeof(nom) + sizeof(path) + 1);
    EMPTY(full_path);
    strcat(full_path, path);
    strcat(full_path, "/");
    strcat(full_path, nom);

    FILE *fichier = fopen(full_path, "a");

    if (!fichier){
        perror("Erreur ouverture fichier ");
        return;
    }

    int numpaquet = 1;

    while (1){

        char recv_buffer[sizeof(msg_fichier)];
        memset(recv_buffer, 0, sizeof(msg_fichier));

        //is_ready(sock_fichier, port);

        if (recv(soket_fichier, recv_buffer, sizeof(msg_fichier), 0) < 0){
            perror("recv() => ajout_fichier_aux ");
            exit(EXIT_FAILURE);
        }

        msg_fichier *msg = malloc(sizeof(msg_fichier));
        memcpy(msg, recv_buffer, sizeof(msg_fichier));

        printf("Reception du paquet numero %d, de taille %zu octets du serveur\n"
                        , numpaquet, strlen(msg->data));

        printf("Ecriture du paquet dans le fichier...\n");
        fwrite(msg->data, sizeof(char)
                        , strlen(msg->data), fichier);

        numpaquet++;

        if (strlen(msg->data) < 512) break;

    }

    printf("Fichier telecharger avec succes a l'adresse : %s\n", full_path);

    free(full_path);
    fclose(fichier);
    close(soket_fichier);
}

//nom du fichier, et path est le chemin ou enregistrer le fichier
void telecharger_fichier(uint16_t f, const char *nom, const char *path){

    int port = get_allocated_port(id);

    msg_fil * msg = compose_msg_fil(nom, 6, id, f, port);

    int len = sizeof(msg_fil) + strlen(nom);

    char send_buffer[len];
    memcpy(send_buffer, msg, len);
    memcpy(send_buffer+sizeof(msg_fil), nom, strlen(nom));

    printf("Envoi de la requete au serveur...\n");
    //envoyer la requete au serveur
    if (sendto(sock, send_buffer, len, 0,
                (struct sockaddr*)&servadr, sizeof(servadr)) < 0){
        perror("sendto()");
        exit(EXIT_FAILURE);
    }  

    char recv_buffer[sizeof(msg_srv)];
    memset(recv_buffer, 0, sizeof(msg_srv));

    if (recv(sock, recv_buffer, sizeof(msg_srv), 0) < 0){
        perror("recv() => telecharger_fichier ");
        exit(EXIT_FAILURE);
    }

    msg_srv *rep_srv = malloc(sizeof(msg_srv));
    memcpy(rep_srv, recv_buffer, sizeof(msg_srv));

    if (erreur(rep_srv->entete)){
        return;
    }

    printf("OK du serveur\n");

    free(rep_srv);

    telecharger_fichier_aux(htons(port), nom, path);

}

int main(){
    
    struct sockaddr_in server_addr;
    struct timeval timeout; 

    sock = socket(PF_INET6, SOCK_DGRAM, 0);
    if(sock < 0) {
        perror("socket()");
        return -1;
    }

    timeout.tv_sec = TIMEOUT; 
    timeout.tv_usec = 0; 

    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        perror("Failed to set socket receive timeout");
        exit(EXIT_FAILURE);
    }

    memset(&servadr, 0, sizeof(servadr));
    servadr.sin6_family = AF_INET6;
    servadr.sin6_port = htons(7777);
    inet_pton(AF_INET6, "::1", &servadr.sin6_addr);

    int ok = 1;
    if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &ok, sizeof(ok)) < 0) {
        perror("echec de SO_REUSEADDR");
        close(sock);
        return 1;
    }

    
    char inscri_buffer[sizeof(msg_inscri)];
    char send_buffer[sizeof(msg_fil)];
    char recv_buffer[sizeof(msg_srv_fil)];
    msg_srv * msf = malloc(sizeof(msg_srv_fil));
    msg_inscri * mi;
    msg_fil * mf;
    int choix = 0;
    uint8_t codeReq = 0;
    uint16_t id = 0;
    uint16_t e = 0;
    uint16_t numfil = 0;
    char pseudo[11];
    char message[256];

menu:
    while(1){
        printf("[1] Inscription\n");
        printf("[2] Poster un billet\n");
        printf("[4] Abonnement\n");
        printf("[7] Notifications\n");
        printf("--------------------------\n");
        printf("codeReq : ");
        scanf("%d", &choix);

        switch (choix) {
            case 1:
                printf("Entrez un pseudo: ");
                fscanf(stdin, "%10s", pseudo);
                mi = inscription(pseudo);
                memcpy(inscri_buffer, mi, sizeof(msg_inscri));
                if (sendto(sock, inscri_buffer, sizeof(msg_inscri), 0, (struct sockaddr*)&servadr, sizeof(servadr)) < 0) goto error;
                if(read(sock, recv_buffer, sizeof(msg_srv)) < 0) exit(EXIT_FAILURE);
                memcpy(msf, inscri_buffer, sizeof(msg_srv));
                e = *((uint16_t*)recv_buffer);
                extract_entete(e, &codeReq, &id);
                if(codeReq != 31) printf("Votre id est : %d\n", id);
                else goto error;
                break;
            case 2:
                printf("Entrez votre id : ");
                scanf("%hd", &id);
                printf("Entrez le numéro du fil sur lequel vous voulez poster (0 pour créer un nouveau fil): ");
                scanf("%hd", &numfil);
                printf("Votre message :");
                fscanf(stdin, "%255s", message);
                mf = poster_billet(id, numfil, message);
                memcpy(send_buffer, mf, sizeof(msg_fil));
                if (sendto(sock, send_buffer, sizeof(msg_fil), 0, (struct sockaddr*)&servadr, sizeof(servadr)) < 0) goto error;
                if(read(sock, recv_buffer, sizeof(msg_srv)) < 0) exit(EXIT_FAILURE);
                memcpy(msf, recv_buffer, sizeof(msg_srv));
                e = *((uint16_t*)recv_buffer);
                extract_entete(e, &codeReq, &id);
                if(codeReq != 31) printf("Votre message a été posté\n");
                else goto error;
                break;
            case 3:
                break;
            case 4:
                printf("Entrez votre id : ");
                scanf("%hd", &id);
                printf("Entrez le numéro du fil auqel vous voulez vous abonner: ");
                scanf("%hd", &numfil);
                mf = demande_abonnement(id, numfil);
                memcpy(send_buffer, mf, sizeof(msg_fil));
                if (sendto(sock, send_buffer, sizeof(msg_fil), 0, (struct sockaddr*)&servadr, sizeof(servadr)) < 0) goto error;
                if(read(sock, recv_buffer, sizeof(msg_srv_fil)) < 0) exit(EXIT_FAILURE);
                memcpy(msf, recv_buffer, sizeof(msg_srv_fil));
                e = *((uint16_t*)recv_buffer);
                extract_entete(e, &codeReq, &id);
                if(codeReq == 4){
                    sabonner(recv_buffer);
                    printf("Vous êtes désormais abonné au fil %d\n", ntohs(msf->numfil));
                }
                else goto error;
                break;
            case 5:
                break;
            case 6:
                break;
            case 7:
                affich_notif(notifs);
                free_notif(notifs);
                break;
            default:
                printf("codeReq erroné\n");
                break;
        }
    }

error:
    printf("Une erreur est survenue lors de votre derière requête. Veuillez réessayer");
    goto menu;
}
