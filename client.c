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
#include <net/if.h>

#define BUF_SIZE 1024

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
        printf("file : %d pseudo : %s message : %s\n", notif->numfil, notif->pseudo, notif->data);
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

int main(){
    int sock = socket(PF_INET6, SOCK_DGRAM, 0);
    if(sock < 0) {
        perror("socket()");
        return -1;
    }

    struct sockaddr_in6 servadr;
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
    


    msg_inscri * m = inscription("ahmed");

    char inscri_buffer[sizeof(msg_inscri)];
    memcpy(inscri_buffer, m, sizeof(msg_inscri));

    if (sendto(sock, inscri_buffer, sizeof(msg_inscri), 0, (struct sockaddr*)&servadr, sizeof(servadr)) < 0) {
        perror("Erreur lors de l'envoi du message");
        exit(EXIT_FAILURE);
    }

    char recv_buffer[sizeof(msg_srv_fil)];
    memset(&servadr, 0, sizeof(servadr));
    socklen_t recv_len = sizeof(servadr);

    if (recvfrom(sock, recv_buffer, sizeof(msg_srv_fil), 0, (struct sockaddr*)&servadr, &recv_len) < 0) {
        perror("Erreur lors de l'envoi du message");
        exit(EXIT_FAILURE);
    }


    msg_srv * mf = malloc(sizeof(msg_srv_fil));
    memcpy(mf, recv_buffer, sizeof(msg_srv_fil));
    uint16_t e = *((uint16_t*)recv_buffer);
    uint8_t codeReq = 0;
    uint16_t id = 0;
    extract_entete(e, &codeReq, &id);
    printf("codeReq : %d id : %d f : %d\n", codeReq, id, ntohs(mf->numfil));

    
    msg_fil * mm = poster_billet(199, 0, "hello my name is ahmed");

    char send_buffer[sizeof(msg_fil)];
    memcpy(send_buffer, mm, sizeof(msg_fil));

    if (sendto(sock, send_buffer, sizeof(msg_fil), 0, (struct sockaddr*)&servadr, sizeof(servadr)) < 0) {
        perror("Erreur lors de l'envoi du message");
        exit(EXIT_FAILURE);
    }

    memset(&servadr, 0, sizeof(servadr));

    if (recvfrom(sock, recv_buffer, sizeof(msg_srv_fil), 0, (struct sockaddr*)&servadr, &recv_len) < 0) {
        perror("Erreur lors de l'envoi du message");
        exit(EXIT_FAILURE);
    }

    memcpy(mf, recv_buffer, sizeof(msg_srv_fil));
    e = *((uint16_t*)recv_buffer);
    extract_entete(e, &codeReq, &id);
    printf("codeReq : %d id : %d f : %d\n", codeReq, id, ntohs(mf->numfil));
    
    
    mm = demande_abonnement(199, 1);

    memcpy(send_buffer, mm, sizeof(msg_fil));

    if (sendto(sock, send_buffer, sizeof(msg_fil), 0, (struct sockaddr*)&servadr, sizeof(servadr)) < 0) {
        perror("Erreur lors de l'envoi du message");
        exit(EXIT_FAILURE);
    }

    memset(&servadr, 0, sizeof(servadr));

    if (recvfrom(sock, recv_buffer, sizeof(msg_srv_fil), 0, (struct sockaddr*)&servadr, &recv_len) < 0) {
        perror("Erreur lors de l'envoi du message");
        exit(EXIT_FAILURE);
    }

    memcpy(mf, recv_buffer, sizeof(msg_srv_fil));
    e = *((uint16_t*)recv_buffer);
    extract_entete(e, &codeReq, &id);
    printf("codeReq : %d id : %d f : %d\n", codeReq, id, ntohs(mf->numfil));
    
    if(codeReq == 4){ 
        sabonner(recv_buffer);
    }
    sleep(30);
}