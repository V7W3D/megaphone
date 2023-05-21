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

    struct sockaddr_in6 servadr;
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